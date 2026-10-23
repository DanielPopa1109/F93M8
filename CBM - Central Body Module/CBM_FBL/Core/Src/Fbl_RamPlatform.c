/* Independent STM32F411 SPI1/MCP2515 polling. Pins/config match resident source:
 * PA5/6/7 AF5, PC4 CS, CNF 00/90/82. No HAL, DMA or IRQ dependency. */
#include "Fbl_Ram.h"
#include "stm32f4xx.h"
static RAM_DATA uint32_t lastCycles,milliseconds,remainder;
static RAM_DATA uint8_t spiError;
RAM_CODE uint32_t Ram_Millis(void) {
 uint32_t now=DWT->CYCCNT,delta=now-lastCycles;lastCycles=now;
 remainder+=delta;milliseconds+=remainder/100000U;remainder%=100000U;return milliseconds;
}
static RAM_CODE void cs(uint8_t high){
 /* RXNE may precede the final trailing SCK edge. Complete the SPI transaction
  * before releasing MCP2515 CS, including an independent bounded BSY wait. */
 if(high){uint32_t budget=100000;while(SPI1->SR&SPI_SR_BSY)if(!--budget){spiError=1;break;}}
 GPIOC->BSRR=high?(1U<<4):(1U<<20);
}
static RAM_CODE uint8_t spi(uint8_t v) {
 uint32_t budget=100000U;
 while(!(SPI1->SR&SPI_SR_TXE))if(!--budget){spiError=1;return 0;}
 *(volatile uint8_t *)&SPI1->DR=v;budget=100000U;
 while(!(SPI1->SR&SPI_SR_RXNE))if(!--budget){spiError=1;return 0;}
 return *(volatile uint8_t *)&SPI1->DR;
}
static RAM_CODE uint8_t readReg(uint8_t a){uint8_t v;cs(0);spi(3);spi(a);v=spi(0);cs(1);return v;}
static RAM_CODE void writeReg(uint8_t a,uint8_t v){cs(0);spi(2);spi(a);spi(v);cs(1);}
static RAM_CODE void modify(uint8_t a,uint8_t mask,uint8_t v){cs(0);spi(5);spi(a);spi(mask);spi(v);cs(1);}
RAM_CODE int Ram_CanInit(void) {
 CoreDebug->DEMCR|=CoreDebug_DEMCR_TRCENA_Msk;DWT->CTRL|=1;
 lastCycles=DWT->CYCCNT;milliseconds=remainder=0;spiError=0;
 RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN|RCC_AHB1ENR_GPIOCEN;RCC->APB2ENR|=RCC_APB2ENR_SPI1EN;
 (void)RCC->APB2ENR;
 GPIOC->BSRR=1U<<4;GPIOC->MODER=(GPIOC->MODER&~(3U<<8))|(1U<<8);
 GPIOA->MODER=(GPIOA->MODER&~(0x3FU<<10))|(0x2AU<<10);
 GPIOA->AFR[0]=(GPIOA->AFR[0]&~0xFFF00000U)|0x55500000U;
 SPI1->CR1=0;SPI1->CR2=0;SPI1->CR1=SPI_CR1_MSTR|SPI_CR1_SSM|SPI_CR1_SSI|(3U<<3)|SPI_CR1_SPE;
 writeReg(0x0F,0x80);
 uint32_t begin=Ram_Millis();while((readReg(0x0E)&0xE0)!=0x80)if(spiError || Ram_Millis()-begin>100)return 1;
 writeReg(0x2A,0);writeReg(0x29,0x90);writeReg(0x28,0x82);
 writeReg(0x2B,0);writeReg(0x2C,0);writeReg(0x30,0);writeReg(0x40,0);writeReg(0x50,0);
 /* Receive standard traffic, filter ID/RTR in SRAM software. */
 writeReg(0x60,0x60);writeReg(0x70,0x60);writeReg(0x0F,0);
 begin=Ram_Millis();while(readReg(0x0E)&0xE0)if(spiError || Ram_Millis()-begin>100)return 1;
 return spiError?1:0;
}
RAM_CODE int Ram_CanReceive(uint16_t *id,uint8_t *dlc,uint8_t *p) {
 uint8_t status=readReg(0x2C),base,raw[13];
 if(spiError)return -1;if(!(status&3))return 0;base=(status&1)?0x61:0x71;
 cs(0);spi(3);spi(base);for(uint8_t i=0;i<13;i++)raw[i]=spi(0);cs(1);
 modify(0x2C,(status&1)?1:2,0);
 if(spiError || (raw[1]&8) || (raw[4]&0x40) || (raw[4]&15)>8)return 0;
 *id=((uint16_t)raw[0]<<3)|(raw[1]>>5);*dlc=raw[4]&15;
 for(uint8_t i=0;i<*dlc;i++)p[i]=raw[5+i];return 1;
}
RAM_CODE int Ram_CanSend(const uint8_t *p) {
 if(spiError || (readReg(0x2D)&0x20) || (readReg(0x30)&8))return 1;
 modify(0x2C,4,0);cs(0);spi(2);spi(0x31);spi(0xE2);spi(0);spi(0);spi(0);spi(8);
 for(uint8_t i=0;i<8;i++)spi(p[i]);cs(1);cs(0);spi(0x81);cs(1);
 uint32_t begin=Ram_Millis();
 for(;;){uint8_t ctrl=readReg(0x30),flags=readReg(0x2C);
  if(spiError || (ctrl&0x70) || (readReg(0x2D)&0x20) || Ram_Millis()-begin>1000){modify(0x30,8,0);return 1;}
  if(!(ctrl&8) && (flags&4)){modify(0x2C,4,0);return 0;}}
}
RAM_CODE void Ram_Reset(void) {
 /* FCD may immediately continue with APP programming. Enter the replacement
  * resident FBL once in programming session, even when the old APP is valid. */
 volatile uint32_t *box=(volatile uint32_t *)FBL_BOOT_MAILBOX;
 box[1]=2;box[2]=~2U;box[0]=0x46424C31U;__asm volatile("dsb":::"memory");
 SCB->AIRCR=0x05FA0004U;__asm volatile("dsb":::"memory");for(;;){}
}
#ifndef RAM_HOST_TEST
static RAM_CODE void fault(void){for(;;){}}
__attribute__((section(".ram_blob_vectors"),used,aligned(512)))
const uint32_t ram_vectors[112]={RAM_STACK_TOP,(uint32_t)Ram_Start,[2 ... 111]=(uint32_t)fault};
#endif
