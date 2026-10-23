#ifndef F93_DCM_CORE_H
#define F93_DCM_CORE_H

#include <stdint.h>

/* Shared UDS service front end for the two STM32 applications. The board DCM
 * owns CAN transmission and ECU-specific service handlers. Called every 5 ms. */
typedef struct
{
    uint16_t s3Ticks;
} DcmCore_State;

typedef struct
{
    uint8_t handled;
    uint8_t responseLength;
    uint8_t response[8];
} DcmCore_Result;

static inline void DcmCore_MainFunction(DcmCore_State *state, uint8_t *expired)
{
    *expired = 0U;
    if (state->s3Ticks < 1000U) ++state->s3Ticks;
    if (state->s3Ticks == 1000U) *expired = 1U; /* 5 seconds at 5 ms */
}

static inline uint8_t DcmCore_SessionByte(uint8_t internalSession)
{
    if (internalSession == 1U) return 0x03U; /* extended */
    if (internalSession == 2U) return 0x02U; /* programming */
    if (internalSession == 3U) return 0x04U; /* CBM coding */
    if (internalSession == 4U) return 0x05U; /* CBM calibration */
    return 0x01U;
}

static inline DcmCore_Result DcmCore_Process(const uint8_t frame[8],
                                              uint8_t dlc,
                                              uint8_t internalSession)
{
    DcmCore_Result result = {0};
    uint8_t sid;
    if (dlc < 2U || dlc > 8U || frame == 0) return result;
    sid = frame[1];
    if (sid != 0x3EU && sid != 0x22U) return result;
    if (sid == 0x22U && (dlc < 4U || frame[2] != 0xF1U || frame[3] != 0x86U))
        return result; /* Leave target-specific DIDs with the ECU DCM. */
    result.handled = 1U;
    if ((frame[0] & 0xF0U) != 0U || frame[0] > dlc - 1U ||
        (sid == 0x3EU && frame[0] != 2U) ||
        (sid == 0x22U && frame[0] != 3U))
    {
        result.responseLength = 4U;
        result.response[0] = 3U;
        result.response[1] = 0x7FU;
        result.response[2] = sid;
        result.response[3] = 0x13U;
        return result;
    }
    if (sid == 0x3EU)
    {
        if (frame[2] == 0x80U) return result; /* Suppress positive response. */
        if (frame[2] != 0U)
        {
            result.responseLength = 4U;
            result.response[0] = 3U;
            result.response[1] = 0x7FU;
            result.response[2] = 0x3EU;
            result.response[3] = 0x12U;
            return result;
        }
        result.responseLength = 3U;
        result.response[0] = 2U;
        result.response[1] = 0x7EU;
        result.response[2] = 0U;
        return result;
    }
    result.responseLength = 5U;
    result.response[0] = 4U;
    result.response[1] = 0x62U;
    result.response[2] = 0xF1U;
    result.response[3] = 0x86U;
    result.response[4] = DcmCore_SessionByte(internalSession);
    return result;
}

#endif
