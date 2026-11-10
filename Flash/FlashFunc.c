//----------------------------------------------------------------------------------------------------
// ID Code      : FlashFunc.c
// Update Note  : Decompiled and reverse-engineered from vendor FlashFunc.obj
//                Clean open-source implementation for RTD2660 SPI Flash wear-leveling storage
//----------------------------------------------------------------------------------------------------

#define __FLASH_KX_FUNC__

#include "Core\Header\Include.h"

#if(_MEMORY_LOCATION == _FLASH)

//----------------------------------------------------------------------------------------------------
// Low-level SPI Flash SFR Operations
//----------------------------------------------------------------------------------------------------

void FlSetOpCode(void)
{
    MCU_WREN_OP_CODE_FF62 = 0x06;
    MCU_EWSR_OP_CODE_FF63 = 0x50;
    MCU_READ_OP_CODE_FF6A = 0x03;
    MCU_FAST_READ_OP_CODE_FF6B = 0x0B;
}

void FlSetStatusReg(BYTE ucStatus)
{
    BYTE ucCmd = 0x60 | 0x08; // 0x68
    MCU_COMMON_INST_EN_FF60 = ucCmd;
    MCU_COMMON_OP_CODE_FF61 = 0x01; // WRSR (Write Status Register)
    MCU_FLASH_PROG_ISP0_FF64 = ucStatus;
    MCU_COMMON_INST_EN_FF60 = ucCmd | 0x01; // 0x69: Execute
}

void FlashProtectEnable(void)
{
    FlSetStatusReg(0x1C);
    CTimerPollingFlagProcForMcu(40, (BYTE *)&MCU_COMMON_INST_EN_FF60, 0x01, _FALSE);
    MCU_COMMON_OP_CODE_FF61 = 0xFF;
}

void FlashProtectDisable(void)
{
    FlSetStatusReg(0x00);
    CTimerPollingFlagProcForMcu(40, (BYTE *)&MCU_COMMON_INST_EN_FF60, 0x01, _FALSE);
    MCU_COMMON_OP_CODE_FF61 = 0xFF;
}

bit CFlashWREN(BYTE ucPage)
{
    ucPage = ucPage;
    MCU_PROG_OP_CODE_FF6D = 0x02; // Page Program opcode
    FlashProtectDisable();
    return _TRUE;
}

bit CFlashWRDis(void)
{
    MCU_PROG_OP_CODE_FF6D = 0xFF;
    FlashProtectEnable();
    return _TRUE;
}

void FlashEraseSector(BYTE ucPage)
{
    MCU_FLASH_PROG_ISP0_FF64 = 0x05;
    MCU_FLASH_PROG_ISP1_FF65 = ucPage << 4;
    MCU_FLASH_PROG_ISP2_FF66 = 0x00;
    MCU_COMMON_INST_EN_FF60 = 0xB8;
    MCU_COMMON_OP_CODE_FF61 = 0x20; // Sector Erase 4KB
    MCU_COMMON_INST_EN_FF60 = 0xB9; // 0xB8 | 0x01: Execute
    CTimerPollingFlagProcForMcu(120, (BYTE *)&MCU_COMMON_INST_EN_FF60, 0x01, _FALSE);
    MCU_COMMON_OP_CODE_FF61 = 0xFF;
}

bit CFlashErase(BYTE ucPage)
{
    if ((BYTE)(ucPage << 4) >= 0xF0)
    {
        return _FALSE;
    }
    CFlashWREN(ucPage);
    FlashEraseSector(ucPage);
    CFlashWRDis();
    return _TRUE;
}

//----------------------------------------------------------------------------------------------------
// Utility Functions
//----------------------------------------------------------------------------------------------------

void FillFFToMemory(BYTE *Array, BYTE ucCnt)
{
    while (ucCnt--)
    {
        *Array++ = 0xFF;
    }
}

bit CTimerPollingFlagProcForMcu(WORD usTimeout, BYTE *ucAddress, BYTE ucBit, bit bSuccess)
{
    while (1)
    {
        CTimerDelayXms(1);
        if (bSuccess)
        {
            if ((*ucAddress & ucBit) != 0)
            {
                return _TRUE;
            }
        }
        else
        {
            if ((*ucAddress & ucBit) == 0)
            {
                return _TRUE;
            }
        }
        if (usTimeout == 0)
        {
            return _FALSE;
        }
        usTimeout--;
    }
}

bit FLCompSaveData(BYTE xdata *pFlash, BYTE *Array)
{
    BYTE i;

    pFlash++;
    for (i = 0; i < 14; i++)
    {
        if (*pFlash++ != *Array++)
        {
            return _FALSE;
        }
    }
    return _TRUE;
}

//----------------------------------------------------------------------------------------------------
// High-Level Wear-Leveling Storage (16-byte blocks)
//----------------------------------------------------------------------------------------------------

bit FLSaveData(BYTE ucPage, BYTE ID, BYTE FreeGroup)
{
    BYTE ucCnt = _PAGE_SAVE_DATA_CNT; // 0xFF
    BYTE ucCheckSum = 0;
    BYTE xdata *pFlash = (BYTE xdata *)(ucPage << 12);
    BYTE j;

    while (ucCnt--)
    {
        if (ucCnt == 0)
        {
            return _FALSE;
        }
        if (*pFlash == ID)
        {
            if (FLCompSaveData(pFlash, g_szFlashBuf))
            {
                return _TRUE;
            }
            *pFlash = 0x00; // Mark old record invalidated
        }
        else if (*pFlash == 0xFF)
        {
            if (ucCnt < FreeGroup)
            {
                return _FALSE;
            }
            *pFlash = ID;
            for (j = 0; j < 14; j++)
            {
                pFlash[1 + j] = g_szFlashBuf[j];
                ucCheckSum += g_szFlashBuf[j];
            }
            pFlash[15] = ucCheckSum;
            return _TRUE;
        }
        pFlash += 16;
    }
    return _FALSE;
}

bit FLLoadData(BYTE ucPage, BYTE ID)
{
    BYTE i;
    BYTE ucCnt = _PAGE_SAVE_DATA_CNT; // 0xFF
    BYTE ucCheckSum = 0;
    BYTE xdata *pFlash = (BYTE xdata *)(ucPage << 12);

    while (ucCnt--)
    {
        if (*pFlash == ID)
        {
            for (i = 0; i < 14; i++)
            {
                g_szFlashBuf[i] = pFlash[1 + i];
                ucCheckSum += g_szFlashBuf[i];
            }
            if (pFlash[15] == ucCheckSum)
            {
                return _TRUE;
            }
            return _FALSE;
        }
        else if (*pFlash == 0xFF)
        {
            return _FALSE;
        }
        pFlash += 16;
    }
    return _FALSE;
}

void CFlashMovePage(BYTE ucObjPage, BYTE ucSrcPage)
{
    BYTE ucCnt = _PAGE_SAVE_DATA_CNT; // 0xFF
    BYTE xdata *pSrc = (BYTE xdata *)(ucSrcPage << 12);
    BYTE xdata *pObj = (BYTE xdata *)(ucObjPage << 12);
    BYTE i;

    CFlashWREN(ucObjPage);
    while (ucCnt--)
    {
        if (*pSrc != 0xFF && *pSrc != 0x00)
        {
            for (i = 0; i < 16; i++)
            {
                pObj[i] = pSrc[i];
            }
            pObj += 16;
        }
        pSrc += 16;
    }
    CFlashWRDis();
}

//----------------------------------------------------------------------------------------------------
// Random Page Management
//----------------------------------------------------------------------------------------------------

BYTE FoundEmptyRandomPage(void)
{
    BYTE ucPage;
    BYTE xdata *pFlash = (BYTE xdata *)0x6000;

    for (ucPage = _RANDOM_SAVE_START_PAGE; ucPage < _RANDOM_SAVE_END_PAGE; ucPage++)
    {
        if (*pFlash == 0xFF)
        {
            return ucPage;
        }
        pFlash = (BYTE xdata *)((WORD)pFlash + 0x1000);
    }
    return 0xFF;
}

void InitRandomPage(BYTE ucPage, BYTE ucRandomPageIndex)
{
    BYTE xdata *pFlash = (BYTE xdata *)(ucPage << 12);

    CFlashWREN(ucPage);
    pFlash[0] = 0xFE;
    pFlash[1] = ucRandomPageIndex;
    CFlashWRDis();
}

BYTE FoundRandomPage(BYTE ucRandomPageIndex)
{
    BYTE ucPage;
    BYTE xdata *pFlash = (BYTE xdata *)0x6000;

    for (ucPage = _RANDOM_SAVE_START_PAGE; ucPage < _RANDOM_SAVE_END_PAGE; ucPage++)
    {
        if (pFlash[0] == 0xFE && pFlash[1] == ucRandomPageIndex)
        {
            return ucPage;
        }
        pFlash = (BYTE xdata *)((WORD)pFlash + 0x1000);
    }
    return 0xFF;
}

void InitFlash(void)
{
    BYTE i;

    FlSetOpCode();
    for (i = 0; i < 7; i++)
    {
        g_szRandomSavePage[i] = FoundRandomPage(i);
    }
}

#endif // #if(_MEMORY_LOCATION == _FLASH)
