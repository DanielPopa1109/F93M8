#include <assert.h>
#include <string.h>
#include "../BSW/Com/Can/Can.h"
#include "../BSW/Com/CanIf/CanIf.h"
#include "../BSW/Com/CanTp/CanTp.h"
#include "../BSW/Com/PduR/PduR.h"

static Can_FrameType sent[64];
static unsigned sentCount, requestCount;
static uint8_t lastRequest[256];
static uint16_t lastLength;

Can_StatusType Can_TestTransmit(uint16_t id, const uint8_t *data, uint8_t dlc)
{
    assert(sentCount < 64U);
    sent[sentCount].id = id;
    sent[sentCount].dlc = dlc;
    memcpy(sent[sentCount].data, data, dlc);
    ++sentCount;
    return CAN_OK;
}

void Dcm_TpRxIndication(const uint8_t *data, uint16_t len)
{
    static const uint8_t longResponse[10] = {0x62, 0xF1, 0x86, 1, 2, 3, 4, 5, 6, 7};
    static const uint8_t shortResponse[2] = {0x7E, 0x00};
    assert(len <= sizeof(lastRequest));
    memcpy(lastRequest, data, len);
    lastLength = len;
    ++requestCount;
    assert(PduR_DcmTransmit(len > 6U ? longResponse : shortResponse,
                            len > 6U ? 10U : 2U) == CAN_OK);
}

static void reset(void)
{
    sentCount = requestCount = 0U;
    lastLength = 0U;
    Can_Init(); CanIf_Init(); CanTp_Init(); PduR_Init();
}

static void receive(uint16_t id, const uint8_t *bytes, uint8_t dlc)
{
    Can_FrameType frame = {0};
    frame.id = id; frame.dlc = dlc;
    memcpy(frame.data, bytes, dlc);
    Can_RxIndication(&frame);
}

int main(void)
{
    static const uint8_t sf[8] = {0x54, 0x02, 0x3E, 0x00};
    static const uint8_t ff[8] = {0x54, 0x10, 0x09, 1, 2, 3, 4, 5};
    static const uint8_t cf[8] = {0x54, 0x21, 6, 7, 8, 9};
    static const uint8_t badCf[8] = {0x54, 0x22, 6, 7, 8, 9};
    static const uint8_t fc[8] = {0x41, 0x30, 0, 0};
    static const uint8_t tooLarge[8] = {0x54, 0x11, 0x01, 1, 2, 3, 4, 5};
    unsigned i;

    reset();
    receive(0x710, sf, 8);
    assert(requestCount == 1 && lastLength == 2 && lastRequest[0] == 0x3E);
    assert(sentCount == 1 && sent[0].id == 0x710 &&
           sent[0].data[0] == 0xF0 && sent[0].data[1] == 2 && sent[0].data[2] == 0x7E);

    reset();
    receive(0x710, ff, 8);
    assert(sentCount == 1 && sent[0].data[1] == 0x30);
    receive(0x710, cf, 8);
    assert(requestCount == 1 && lastLength == 9 && lastRequest[8] == 9);
    assert(sentCount == 2 && sent[1].data[1] == 0x10 && sent[1].data[2] == 10);
    receive(0x710, fc, 8);
    CanTp_MainFunction();
    assert(sentCount == 3 && sent[2].data[1] == 0x21 && sent[2].data[2] == 3);

    reset();
    receive(0x710, ff, 8);
    receive(0x710, badCf, 8);
    assert(requestCount == 0 && PduR_GetRxErrorCount() == 1);

    reset();
    receive(0x710, ff, 8);
    for (i = 0; i < 201; ++i) CanTp_MainFunction();
    receive(0x710, cf, 8);
    assert(requestCount == 0 && PduR_GetRxErrorCount() == 1);

    reset();
    {
        uint8_t first[8] = {0x54, 0x11, 0x00, 1, 2, 3, 4, 5};
        uint8_t consecutive[8] = {0x54};
        receive(0x710, first, 8);
        for (i = 0; i < 42U; ++i)
        {
            consecutive[1] = (uint8_t)(0x20U | ((i + 1U) & 0x0FU));
            memset(&consecutive[2], (int)i, 6U);
            receive(0x710, consecutive, 8);
        }
        assert(requestCount == 1 && lastLength == 256U);
    }

    /* BS is byte 2 and STmin byte 3 in an extended-address FC. */
    reset();
    {
        uint8_t response[20] = {0x62};
        uint8_t limitedFc[8] = {0x41, 0x30, 1, 10};
        uint8_t overflowFc[8] = {0x41, 0x32, 0, 0};
        assert(CanTp_Transmit(response, sizeof(response)) == CAN_OK);
        receive(0x710, limitedFc, 4);
        CanTp_MainFunction();
        assert(sentCount == 2 && sent[1].data[1] == 0x21);
        CanTp_MainFunction();
        assert(sentCount == 2); /* Block size one requires another FC. */
        receive(0x710, overflowFc, 4);
        assert(PduR_GetTxErrorCount() == 1);
    }

    reset();
    {
        uint8_t response[20] = {0x62};
        uint8_t waitFc[8] = {0x41, 0x31, 0, 0};
        uint8_t continueFc[8] = {0x41, 0x30, 0, 10};
        assert(CanTp_Transmit(response, sizeof(response)) == CAN_OK);
        receive(0x710, waitFc, 4);
        CanTp_MainFunction();
        assert(sentCount == 1);
        receive(0x710, continueFc, 4);
        CanTp_MainFunction();
        assert(sentCount == 2);
        CanTp_MainFunction();
        assert(sentCount == 2);
        CanTp_MainFunction();
        assert(sentCount == 3);
    }

    reset();
    receive(0x711, sf, 8);
    { uint8_t wrong[8] = {0x55, 0x02, 0x3E, 0x00}; receive(0x710, wrong, 8); }
    assert(requestCount == 0 && sentCount == 0);

    reset();
    receive(0x710, tooLarge, 8);
    assert(sentCount == 1 && sent[0].data[1] == 0x32 && requestCount == 0);

    reset();
    receive(0x710, ff, 8);
    receive(0x710, cf, 8);
    { uint8_t wrongFc[8] = {0x54, 0x30, 0, 0}; receive(0x710, wrongFc, 8); }
    assert(sentCount == 2);
    receive(0x710, fc, 8);
    CanTp_MainFunction();
    assert(sentCount == 3);

    reset();
    receive(0x710, ff, 8);
    receive(0x710, cf, 8);
    for (i = 0; i < 201; ++i) CanTp_MainFunction();
    assert(PduR_GetTxErrorCount() == 1 && sentCount == 2);
    return 0;
}
