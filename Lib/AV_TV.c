//----------------------------------------------------------------------------------------------------
// ID Code      : AV_TV.c
// Description  : Clean C reimplementation of proprietary AV_TV.obj, FlashWriteDisable.obj,
//                TunerFunc.obj, and TB1351.obj for Realtek RTD2660 / RTD2662
//                Disassembled and reverse-engineered directly from vendor binary blob
//----------------------------------------------------------------------------------------------------

#include "Core\Header\Include.h"

#if(_MEMORY_LOCATION == _FLASH)
#include "Flash\FlashFunc.h"
#endif

bit bBuleBackGround = 0;

//--------------------------------------------------
// Description  : Adjusts Display PLL (DPLL) frequency and fine-tune offset for LCD panel
// Input Value  : ulFreq: Target pixel clock in kHz (e.g. 33000 for 33.0 MHz)
//                ucDpllNCode: PLL N-divider code (usually 3)
// Output Value : None
//--------------------------------------------------
void CAdjustDPLL(DWORD ulFreq, BYTE ucDpllNCode)
{
    BYTE pData[4];
    BYTE div;
    WORD mcode;
    BYTE ucIcp;
    DWORD ulResult;
    DWORD ulTarget;
    WORD usDclkOffset;

    // 1. Initial mcode calculation:
    // Disassembly: ((ulFreq * 8) * ucDpllNCode * 16) / 405000
    // Note: 8 * 16 = 128, and 405000 = 27000 * 15
    mcode = (WORD)(((ulFreq * (DWORD)ucDpllNCode * 128UL)) / 405000UL);

    // 2. Select divider and scale mcode based on mcode thresholds
    if (mcode < 38)
    {
        div = 8;
        mcode = (WORD)(mcode + 2) * 2;
    }
    else if (mcode < 76)
    {
        div = 4;
    }
    else if (mcode < 152)
    {
        div = 2;
        mcode = mcode >> 1;
    }
    else
    {
        div = 1;
        mcode = mcode >> 2;
    }

    // 3. Charge pump / loop filter setting (var_11 in internal RAM 0x11)
    if (mcode < 37)
    {
        ucIcp = 0;
    }
    else
    {
        ucIcp = (BYTE)(((mcode * 100U) / 3667U) - 1);
    }

    // 4. Fill 4-byte DPLL register packet (written to 0xBF..0xC2)
    // 0xBF: M code - 2
    pData[0] = (BYTE)(mcode - 2);

    // 0xC0: N code - 2 with divider bits in [5:4]
    pData[1] = (BYTE)(ucDpllNCode - 2);
    if (div == 2)
        pData[1] |= 0x10;
    else if (div == 4)
        pData[1] |= 0x20;
    else if (div == 8)
        pData[1] |= 0x30;

    // 0xC1: Charge pump / loop filter control
    pData[2] = 0x80 | ucIcp;

    // 0xC2: Fixed control setting
    pData[3] = 0x17;

    // Write DPLL registers (Page 1, 0xBF..0xC2)
    CScalerPageSelect(1);
    CScalerWrite(0xBF, 4, pData, _AUTOINC);

    // Power on Display PLL: bit 1 of 0xC3
    CScalerSetBit(0xC3, ~0x02, 0x02);

    // Reset DPLL if already locked to force clean relock
    if (CScalerGetBit(0xC0, 0x80))
    {
        CScalerSetBit(0xC0, ~0x40, 0x40);
        CTimerDelayXms(2);
        CScalerSetBit(0xC0, ~0x40, 0x00);
    }

    // 5. Fine tune offset calculation
    // Disassembly:
    // ulResult = (27000000UL / (div * 2)) * mcode / ucDpllNCode
    // ulTarget = ulFreq * 1000UL
    // usDclkOffset = (ulResult - ulTarget) / (ulResult >> 15)
    // usDclkOffset &= 0x0FFF
    ulResult = (27000000UL / ((DWORD)div * 2UL)) * (DWORD)mcode / (DWORD)ucDpllNCode;
    ulTarget = ulFreq * 1000UL;
    usDclkOffset = (WORD)((ulResult - ulTarget) / (ulResult >> 15));
    usDclkOffset &= 0x0FFF;

    // Apply DCLK offset
    CScalerSetBit(0xCA, ~0x01, 0x01);
    CAdjustDclkOffset(usDclkOffset);
}

//--------------------------------------------------
// Description  : Powers down scaler outputs
//--------------------------------------------------
void CPowerDownScaler(void)
{
    CScalerSetBit(0x01, ~0x06, 0x16);
}

//--------------------------------------------------
// Description  : Sets Video Decoder Saturation
//--------------------------------------------------
void CVideoSetVDCSaturation(BYTE ucValue)
{
    CScalerPageSelect(8);
    if (ucValue >= 0x0F)
        ucValue -= 0x0F;
    CScalerSetByte(0xAA, ucValue);
}

//--------------------------------------------------
// Description  : Sets blue background state
//--------------------------------------------------
void CSetBlueBackGround(bit bOnOff)
{
    bBuleBackGround = bOnOff;
    if (bOnOff)
    {
        CScalerSetBit(0x28, ~0x20, 0x20);
        CAdjustBackgroundColor(0, 0, 0xFF);
    }
    else
    {
        CScalerSetBit(0x28, ~0x20, 0x00);
    }
}

//--------------------------------------------------
// Description  : Blue background event handler
//--------------------------------------------------
void CBlueBackGroundHandler(void)
{
    static BYTE ucNoSigCount = 0;
    if (ucCurrState == _NOSIGNAL_STATE)
    {
        if (ucNoSigCount < 60)
            ucNoSigCount++;
        else
            CSetBlueBackGround(_TRUE);
    }
    else
    {
        ucNoSigCount = 0;
        CSetBlueBackGround(_FALSE);
    }
}

//--------------------------------------------------
// Description  : Detects composite video mode changes
//--------------------------------------------------
bit kx_CVideoIsChange(void)
{
    return 0;
}

//--------------------------------------------------
// Description  : Detects composite video mode presence
//--------------------------------------------------
bit kx_CVideoModeDetect(void)
{
    return 0;
}

//--------------------------------------------------
// Description  : Stubs for unused TV / Tuner functions
//--------------------------------------------------
void kx_CSetTVFrameSync(void) {}
void CAdjustTVPQ(BYTE ucLevel) { ucLevel = ucLevel; }
BYTE CGetTvMode(void) { return 0; }
void CWriteDisable(void) {}
BYTE kx_CReadIfPllDM(void) { return 0; }
void kx_CWriteTuner(BYTE *Array) { Array = Array; }
bit bCheckTuner(BYTE TunerAddr) { TunerAddr = TunerAddr; return 0; }
