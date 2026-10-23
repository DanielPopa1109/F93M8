#include "CanTp.h"
#include "../PduR/PduR.h"
#include <string.h>

#define CANTP_TIMEOUT_TICKS 200U
#define CANTP_RX_BLOCK_SIZE 8U

typedef enum { TX_IDLE, TX_WAIT_FC, TX_SEND_CF, TX_WAIT_CONFIRM } CanTp_TxState;
typedef struct {
    uint8_t active;
    uint8_t nextSn;
    uint8_t blockCount;
    uint16_t len;
    uint16_t offset;
    uint16_t timer;
    uint8_t data[CANTP_MAX_PAYLOAD];
} CanTp_RxContext;
typedef struct {
    CanTp_TxState state;
    uint8_t nextSn;
    uint8_t blockSize;
    uint8_t blockCount;
    uint8_t stTicks;
    uint8_t stWait;
    uint8_t waitCount;
    uint16_t len;
    uint16_t offset;
    uint16_t timer;
    uint8_t data[CANTP_MAX_PAYLOAD];
} CanTp_TxContext;

static CanTp_RxContext rx;
static CanTp_TxContext tx;
static uint8_t initialized,afterConfirmation,fcPending;
static uint8_t earlyFc[8],earlyFcDlc;

static Can_StatusType CanTp_Send(uint8_t address, const uint8_t *payload, uint8_t len)
{
    uint8_t frame[8] = {0};
    if (len > 7U || payload == 0) return CAN_NOT_OK;
    frame[0] = address;
    memcpy(&frame[1], payload, len);
    return CanIf_Transmit(CANIF_TX_DIAG, frame, 8U);
}

static void CanTp_RxAbort(void)
{
    rx.active = 0U;
    rx.offset = rx.len = rx.timer = 0U;
    PduR_CanTpRxError();
}

static void CanTp_TxFinish(uint8_t success)
{
    tx.state = TX_IDLE;
    earlyFcDlc=0U;
    tx.timer = tx.len = tx.offset = 0U;
    PduR_CanTpTxConfirmation(success);
}

static Can_StatusType CanTp_SendFc(uint8_t status)
{
    uint8_t payload[3] = {(uint8_t)(0x30U | status), CANTP_RX_BLOCK_SIZE, 0U};
    Can_StatusType result=CanTp_Send(CANTP_RESPONSE_ADDRESS, payload, 3U);
    if(result==CAN_OK) fcPending=1U;
    return result;
}

void CanTp_Init(void)
{
    memset(&rx, 0, sizeof(rx));
    memset(&tx, 0, sizeof(tx));
    initialized = 1U; fcPending=earlyFcDlc=0U;
}

Can_StatusType CanTp_Transmit(const uint8_t *data, uint16_t len)
{
    uint8_t frame[7] = {0};
    Can_StatusType result;
    if (!initialized || data == 0 || len == 0U || len > CANTP_MAX_PAYLOAD ||
        tx.state != TX_IDLE) return CAN_NOT_OK;
    memcpy(tx.data, data, len);
    tx.len = len;
    tx.offset = 0U;
    tx.nextSn = 1U;
    tx.blockCount = 0U;
    tx.waitCount = 0U;
    if (len <= 6U)
    {
        frame[0] = (uint8_t)len;
        memcpy(&frame[1], tx.data, len);
        result = CanTp_Send(CANTP_RESPONSE_ADDRESS, frame, (uint8_t)(len + 1U));
        if(result==CAN_OK) { tx.state=TX_WAIT_CONFIRM; afterConfirmation=TX_IDLE; tx.timer=CANTP_TIMEOUT_TICKS; }
        else CanTp_TxFinish(0U);
        return result;
    }
    frame[0] = (uint8_t)(0x10U | ((len >> 8) & 0x0FU));
    frame[1] = (uint8_t)len;
    memcpy(&frame[2], tx.data, 5U);
    result = CanTp_Send(CANTP_RESPONSE_ADDRESS, frame, 7U);
    if (result != CAN_OK) { CanTp_TxFinish(0U); return result; }
    tx.offset = 5U;
    tx.state = TX_WAIT_CONFIRM; afterConfirmation=TX_WAIT_FC;
    tx.timer = CANTP_TIMEOUT_TICKS;
    return CAN_OK;
}

static void CanTp_HandleFc(const uint8_t *frame, uint8_t dlc)
{
    uint8_t fs, st;
    if(frame[0]==CANTP_GATEWAY_RX_ADDRESS && tx.state==TX_WAIT_CONFIRM && afterConfirmation==TX_WAIT_FC) {
        memcpy(earlyFc,frame,dlc); earlyFcDlc=dlc; return;
    }
    if (frame[0] != CANTP_GATEWAY_RX_ADDRESS || tx.state != TX_WAIT_FC)
        return;
    if (dlc < 4U) { CanTp_TxFinish(0U); return; }
    fs = frame[1] & 0x0FU;
    st = frame[3];
    if (fs == 2U) { CanTp_TxFinish(0U); return; }
    if (fs == 1U)
    {
        if (++tx.waitCount > 8U) CanTp_TxFinish(0U);
        else tx.timer = CANTP_TIMEOUT_TICKS;
        return;
    }
    if (fs != 0U || (st >= 0x80U && st <= 0xF0U) || st > 0xF9U)
    { CanTp_TxFinish(0U); return; }
    tx.blockSize = frame[2];
    tx.blockCount = 0U;
    tx.waitCount = 0U;
    tx.stTicks = st <= 0x7FU ? (uint8_t)((st + 4U) / 5U) : 1U;
    tx.stWait = 0U;
    tx.state = TX_SEND_CF;
    tx.timer = CANTP_TIMEOUT_TICKS;
}

void CanTp_RxIndication(const uint8_t *frame, uint8_t dlc)
{
    uint8_t type, count;
    uint16_t total;
    if (!initialized || frame == 0 || dlc < 2U || dlc > 8U) return;
    type = frame[1] & 0xF0U;
    if (type == 0x30U) { CanTp_HandleFc(frame, dlc); return; }
    if (frame[0] != CANTP_TARGET_ADDRESS) return;
    if (type == 0x00U)
    {
        count = frame[1] & 0x0FU;
        if (count == 0U || count > 6U || count > dlc - 2U) return;
        if (rx.active) CanTp_RxAbort();
        PduR_CanTpRxIndication(&frame[2], count);
        return;
    }
    if (type == 0x10U)
    {
        if (dlc < 8U) return;
        total = (uint16_t)(((uint16_t)(frame[1] & 0x0FU) << 8) | frame[2]);
        if (rx.active) CanTp_RxAbort();
        if (total <= 6U || total > CANTP_MAX_PAYLOAD)
        { (void)CanTp_SendFc(2U); return; }
        memcpy(rx.data, &frame[3], 5U);
        rx.active = 1U;
        rx.len = total;
        rx.offset = 5U;
        rx.nextSn = 1U;
        rx.blockCount = 0U;
        rx.timer = CANTP_TIMEOUT_TICKS;
        if (CanTp_SendFc(0U) != CAN_OK) CanTp_RxAbort();
        return;
    }
    if (type == 0x20U && rx.active)
    {
        if (dlc < 3U || (frame[1] & 0x0FU) != rx.nextSn)
        { CanTp_RxAbort(); return; }
        count = ((uint16_t)(rx.len - rx.offset) > 6U) ? 6U : (uint8_t)(rx.len - rx.offset);
        if (count > dlc - 2U) { CanTp_RxAbort(); return; }
        memcpy(&rx.data[rx.offset], &frame[2], count);
        rx.offset = (uint16_t)(rx.offset + count);
        rx.nextSn = (uint8_t)((rx.nextSn + 1U) & 0x0FU);
        rx.timer = CANTP_TIMEOUT_TICKS;
        if (rx.offset == rx.len)
        {
            rx.active = 0U;
            PduR_CanTpRxIndication(rx.data, rx.len);
            return;
        }
        if (++rx.blockCount == CANTP_RX_BLOCK_SIZE)
        {
            rx.blockCount = 0U;
            if (CanTp_SendFc(0U) != CAN_OK) CanTp_RxAbort();
        }
    }
}

void CanTp_MainFunction(void)
{
    uint8_t frame[7] = {0};
    uint8_t count;
    if (!initialized) return;
    if (rx.active && rx.timer > 0U && --rx.timer == 0U) CanTp_RxAbort();
    if (tx.state == TX_IDLE) return;
    if (tx.timer > 0U && --tx.timer == 0U) { CanTp_TxFinish(0U); return; }
    if (tx.state != TX_SEND_CF) return;
    if (tx.stWait > 0U) { --tx.stWait; return; }
    count = ((uint16_t)(tx.len - tx.offset) > 6U) ? 6U : (uint8_t)(tx.len - tx.offset);
    frame[0] = (uint8_t)(0x20U | tx.nextSn);
    memcpy(&frame[1], &tx.data[tx.offset], count);
    if (CanTp_Send(CANTP_RESPONSE_ADDRESS, frame, (uint8_t)(count + 1U)) != CAN_OK)
        return; /* Retry on next tick until N_As expires. */
    tx.offset = (uint16_t)(tx.offset + count);
    tx.nextSn = (uint8_t)((tx.nextSn + 1U) & 0x0FU);
    tx.timer = CANTP_TIMEOUT_TICKS;
    afterConfirmation=tx.offset==tx.len?TX_IDLE:TX_SEND_CF;
    tx.stWait = tx.stTicks > 0U ? (uint8_t)(tx.stTicks - 1U) : 0U;
    if (tx.blockSize != 0U && ++tx.blockCount == tx.blockSize)
    { afterConfirmation = tx.offset==tx.len?TX_IDLE:TX_WAIT_FC; tx.blockCount = 0U; }
    tx.state=TX_WAIT_CONFIRM;
}

void CanTp_TxConfirmation(uint8_t success) {
    if(tx.state!=TX_WAIT_CONFIRM) return;
    if(!success) { CanTp_TxFinish(0U); return; }
    if(afterConfirmation==TX_IDLE) { CanTp_TxFinish(1U); return; }
    tx.state=(CanTp_TxState)afterConfirmation; tx.timer=CANTP_TIMEOUT_TICKS;
    if(earlyFcDlc) { uint8_t n=earlyFcDlc; earlyFcDlc=0; CanTp_HandleFc(earlyFc,n); }
}
void CanTp_FcConfirmation(uint8_t success) {
    if(fcPending && !success && rx.active) CanTp_RxAbort();
    fcPending=0U;
}
uint8_t CanTp_IsBusy(void) { return tx.state!=TX_IDLE; }
