# STM32 platform

## Identified target

`ELC_APP/ELC_APP.ioc` and both CubeIDE projects identify `STM32F103C8T6` / `STM32F103C8Tx`, STM32F1 family, Cortex-M3. The project defines `STM32F103xB`, so compile-time device support may represent the 128-KiB density class while the part number C8 conventionally denotes 64 KiB. Resolve actual silicon density and linker fit from the fitted MCU and production build; source files alone do not establish this.

Application and FBL clock setup use HSE, PLL x9, AHB /1, APB1 /2, APB2 /1, flash latency 2 in their respective `SystemClock_Config` functions. CAN timing must be computed from actual HSE and APB1 clock, not inferred from prescaler alone. Both generated CAN modules set prescaler 6, SJW 1, but APP uses BS1 8 / BS2 3 while FBL uses BS1 5 / BS2 6.

APP uses CAN1 PA11 RX / PA12 TX; FBL generated MSP config uses the same pins. Exact board transceiver routing should be checked against `Tool/Documentation/HW/Rev A/` before hardware claims.

APP initializes GPIO, ADC, DMA, timers, CAN, CRC and FreeRTOS. It configures falling-edge PVD level 0. FBL configures PVD similarly and enables several NVIC interrupts. No independent watchdog initialization was found in the reviewed application/FBL startup; do not assume watchdog supervision during flash operations. See [memory/build](MEMORY_AND_BUILD.md).
