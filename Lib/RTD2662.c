//----------------------------------------------------------------------------------------------------
// ID Code      : RTD2662.c
// Description  : Clean C reimplementation of proprietary RTD2662.obj (Video Decoder routines)
//                Decompiled and verified via Ghidra from RTD2660 firmware.
//----------------------------------------------------------------------------------------------------

#include "Core\Header\Include.h"

// VDC Initialization Register Sequence (37 bytes, originally at 0x3441 in Bank 0)
BYTE code tVDC_INITIAL[] =
{
    4, _NON_AUTOINC, 0x9F, 0x08,
    4, _NON_AUTOINC, 0xA8, 0x80,
    4, _NON_AUTOINC, 0xA9, 0x80,
    4, _NON_AUTOINC, 0xD2, 0x80,
    4, _NON_AUTOINC, 0xD6, 0x00,
    4, _NON_AUTOINC, 0x9F, 0x09,
    4, _NON_AUTOINC, 0xE7, 0x8A,
    4, _NON_AUTOINC, 0xAB, 0x03,
    4, _NON_AUTOINC, 0xB6, 0x6B,
    0x00
};

//--------------------------------------------------
// Description  : Disables VDC analog output
//--------------------------------------------------
void kx_CVDCOutputDisable(void)
{
    CScalerPageSelect(0);
    CScalerSetBit(0xAF, ~0x06, 0x00);
}

//--------------------------------------------------
// Description  : Checks if Video Decoder PLL is locked
//--------------------------------------------------
bit kx_CVideoModeLocked(void)
{
    BYTE ucVal;
    CScalerPageSelect(8);
    CScalerRead(0xBC, 1, &ucVal, _NON_AUTOINC);
    return ((ucVal & 0x07) == 0x06);
}

//--------------------------------------------------
// Description  : Initializes Video Decoder registers
//--------------------------------------------------
void kx_CInitialVDC(void)
{
    CScalerPageSelect(8);
    CScalerSetByte(0xCF, 0x01);
    CTimerDelayXms(10);
    CScalerSetByte(0xCF, 0x00);
    CScalerSetBit(0xA7, ~0x30, 0x00);
    CScalerCodeW(tVDC_INITIAL);
}

//--------------------------------------------------
// Description  : Adjusts Video Decoder Hue (0 - 100)
//--------------------------------------------------
void kx_CAdjustVDCHue(const BYTE ucVDCHue)
{
    WORD usHue = ((WORD)ucVDCHue * 255) / 100;
    CScalerPageSelect(8);
    CScalerSetByte(0xAB, (BYTE)(usHue + 0x80));
}

//--------------------------------------------------
// Description  : Detects active video standard (NTSC, PAL, SECAM)
//--------------------------------------------------
BYTE kx_CSearchVideoMode(bit bMode)
{
    BYTE ucStatus;

    CScalerPageSelect(8);
    CScalerRead(0xD2, 1, &ucStatus, _NON_AUTOINC);
    if (ucStatus & 0x80)
    {
        CScalerRead(0xD3, 1, &ucStatus, _NON_AUTOINC);
    }
    ucStatus &= 0x07;

    if (!bMode)
    {
        switch (ucStatus)
        {
            case 0:
            case 4:
                return 0; // Reserved / Unknown
            case 1:
            case 3:
                return 1; // NTSC
            default:
                return 2; // PAL
        }
    }
    else
    {
        switch (ucStatus)
        {
            case 0: return 1; // NTSC
            case 1: return 2; // NTSC4.43
            case 2: return 3; // PAL-M
            case 3: return 4; // PAL 60
            case 4: return 5; // PAL N
            case 5: return 6; // PAL I
            case 6: return 7; // NTSC 50
            case 7: return 8; // SECAM
            default: return 0;
        }
    }
}

//--------------------------------------------------
// Description  : Configures video input routing channels
//--------------------------------------------------
void kx_CSelectVideoChannel(BYTE ucYCrChannel, BYTE ucAVOut)
{
    BYTE ucUpper = ucYCrChannel >> 4;
    BYTE ucLower = ucYCrChannel & 0x0F;
    BYTE ucRegA1, ucRegAE, ucRegAF, ucRegA2, ucRegA0;

    if (ucUpper == 2)
    {
        ucRegA1 = 0x0F;
        ucRegAE = 2;
        ucRegAF = 7;
        ucRegA2 = 0x32;
        ucRegA0 = 4;
    }
    else if (ucUpper == 3)
    {
        ucRegA1 = 0x1F;
        ucRegAE = 4;
        ucRegAF = 3;
        ucRegA2 = 0xC1;
        ucRegA0 = 8;
    }
    else if (ucUpper == 4)
    {
        ucRegA1 = 0x2F;
        ucRegAE = 8;
        ucRegAF = 7;
        ucRegA2 = 0x32;
        ucRegA0 = 0x0C;
    }
    else
    {
        ucRegA1 = 0x0F;
        ucRegAE = 1;
        ucRegAF = 3;
        ucRegA2 = 0xC1;
        ucRegA0 = 0;
    }

    if (ucLower == 2)
    {
        ucRegAE |= 2;
        ucRegAF = 7;
        ucRegA2 |= 0x31;
    }
    else if (ucLower == 3)
    {
        ucRegA1 |= 0x10;
        ucRegAE |= 4;
        ucRegAF = 7;
        ucRegA2 |= 0xC0;
    }
    else if (ucLower == 4)
    {
        ucRegA1 |= 0x20;
        ucRegAE |= 8;
        ucRegAF = 7;
        ucRegA2 |= 0x31;
    }
    else
    {
        if (ucLower == 1)
        {
            ucRegAE |= 1;
        }
        ucRegAF = 7;
        ucRegA2 |= 0xC0;
    }

    CScalerPageSelect(0);
    CScalerSetByte(0xA1, ucRegA1);
    CScalerSetByte(0xAE, ucRegAE);
    CScalerSetByte(0xAF, ucRegAF | 1);

    if (ucAVOut != 0)
    {
        CScalerSetBit(0xA0, 0xF3, ucRegA0);
        CScalerSetBit(0xAF, 0xDF, 0x20);
    }

    CScalerPageSelect(8);
    CScalerSetByte(0xA2, ucRegA2);

    if (ucLower == 0)
    {
        CScalerSetBit(0xA1, 0x9F, 0x00);
        CScalerSetByte(0xE0, 0x03);
    }
    else
    {
        CScalerSetBit(0xA1, 0x9F, 0x20);
        CScalerSetByte(0xE0, 0x00);
    }
}
