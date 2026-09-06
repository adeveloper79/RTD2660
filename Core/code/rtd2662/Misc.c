//----------------------------------------------------------------------------------------------------
// ID Code      : Misc.c No.0002
// Update Note  :
//
//----------------------------------------------------------------------------------------------------

#define __MISC__

#include "Core\Header\Include.h"

//----------------------------------------------------------------------------------------------------
// Polling Events
//----------------------------------------------------------------------------------------------------



#if(_HSYNC_TYPE_DETECTION == _AUTO_RUN)
/**
 * CMiscHsyncTypeAutoRunFlagPollingEvent
 * Hsync Type Detection Auto Run flag polling event
 * @param <none>
 * @return {_TRUE if event occurs,_FALSE if not}
 *
*/
//--------------------------------------------------
// Description  : Hsync Type Detection Auto Run flag polling event
// Input Value  : None
// Output Value : Return _TRUE if event occurs
//--------------------------------------------------
bit CMiscHsyncTypeAutoRunFlagPollingEvent(void)
{
    if((bit)CScalerGetBit(_VSYNC_COUNTER_LEVEL_MSB_4C, _BIT7))
        return _TRUE;
    else
        return _FALSE;
}
#endif  // End of #if(_HSYNC_TYPE_DETECTION == _AUTO_RUN)

/**
 * CMiscModeMeasurePollingEvent
 * Measure start/end polling event
 * @param <none>
 * @return {none}
 *
*/
//--------------------------------------------------
// Description  : Measure start/end polling event
// Input Value  : None
// Output Value : Return _TRUE if measure finished
//--------------------------------------------------
bit CMiscModeMeasurePollingEvent(void)
{
#if (0)
	BYTE count=20;

	do{
		if(!(bit)CScalerGetBit(_MEAS_HS_PERIOD_H_52, _BIT5))
			return _TRUE;
		CTimerDelayXms(5);
		count--;
	}while(count!=0);
	return _FALSE;
#else
    if(!(bit)CScalerGetBit(_MEAS_HS_PERIOD_H_52, _BIT5))
        return _TRUE;
    else
        return _FALSE;
#endif
}

bit CMiscMeasureResultPOPPollingEvent(void)
{
    if(!(bit)CScalerGetBit(_MEAS_HS_PERIOD_H_52, _BIT6))
        return _TRUE;
    else
        return _FALSE;
}


#if((_TMDS_SUPPORT == _ON) || (_HDMI_SUPPORT == _ON))
//--------------------------------------------------
// Description  : Auto measure start/end polling event
// Input Value  : None
// Output Value : Return _TRUE if auto measure finished
//--------------------------------------------------
bit CMiscAutoMeasurePollingEvent(void)
{
    if(!(bit)CScalerGetBit(_AUTO_ADJ_CTRL1_7D, _BIT0))
        return _TRUE;
    else
        return _FALSE;
}
#endif

//--------------------------------------------------
// Description  : Apply double buffer polling event
// Input Value  : None
// Output Value : Return _TRUE if apply double buffer finished
//--------------------------------------------------
bit CMiscApplyDoubleBufferPollingEvent(void)
{
    if(!(bit)CScalerGetBit(_VGIP_CTRL_10, _BIT5))
        return _TRUE;
    else
        return _FALSE;
}

#if(((_TMDS_SUPPORT == _ON) || (_HDMI_SUPPORT == _ON)) && (_DVI_LONG_CABLE_SUPPORT == _ON))
//--------------------------------------------------
// Description  : TMDS transition measure polling event
// Input Value  : None
// Output Value : Return _TRUE if measure finished
//--------------------------------------------------
bit CMiscTMDSMeasureEvent(void)
{
    CScalerPageSelect(_PAGE2);
    if(!(bit)CScalerGetBit(_P2_TMDS_MEAS_RESULT0_A2, _BIT7))
        return _TRUE;
    else
        return _FALSE;
}

//--------------------------------------------------
// Description  : TMDS CRC measure polling event
// Input Value  : None
// Output Value : Return _TRUE if CRC measure finished
//--------------------------------------------------
bit CMiscTMDSCRCEvent(void)
{
    CScalerPageSelect(_PAGE2);
    if(!(bit)CScalerGetBit(_P2_TMDS_CTRL_A4, _BIT0))
    {
        return _TRUE;
    }
    else
    {
        return _FALSE;
    }
}
#endif  // End of #if(((_TMDS_SUPPORT == _ON) || (_HDMI_SUPPORT == _ON)) && (_DVI_LONG_CABLE_SUPPORT == _ON))


//----------------------------------------------------------------------------------------------------
// Misc Functions
//----------------------------------------------------------------------------------------------------

//--------------------------------------------------
// Description  : Enable double buffer
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CMiscEnableDoubleBuffer(void)
{
    CScalerSetBit(_VGIP_CTRL_10, ~_BIT4, _BIT4);
}
//--------------------------------------------------
// Description  : Disable double buffer
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CMiscDisableDoubleBuffer(void)
{
    CScalerSetBit(_VGIP_CTRL_10, ~_BIT4, 0x00);
}

//--------------------------------------------------
// Description  : Apply double buffer
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CMiscApplyDoubleBuffer(void)
{
    BYTE ucTimeout=12;
    if((bit)CScalerGetBit(_VGIP_CTRL_10, _BIT4))
    {
        CScalerSetBit(_VGIP_CTRL_10, ~_BIT5, _BIT5);
        do
            {
                CTimerDelayXms(5);
                if(CMiscApplyDoubleBufferPollingEvent())
                {
                    break;
                }
            }
            while(--ucTimeout); 
    }
}

//--------------------------------------------------
// Description  : Clear status Reg[02] and Reg[03]
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CMiscClearStatusRegister(void)
{
    CScalerSetByte(_STATUS0_02, 0x00);
    CScalerSetByte(_STATUS1_03, 0x00);
}

//--------------------------------------------------
// Description  : Set pin share
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CMiscSetPinShare(void)
{
    // Exact factory PinShare registers decoded from live_dump @ 0x0CF44 (bits 2:0 = Pin 50 mode)
    MCU_PIN_SHARE_CTRL00_FF96 = (0x28 & 0xF8) | ((BYTE)_PIN_50);
    MCU_PIN_SHARE_CTRL01_FF97 = (((BYTE)_PIN_51<<6) | (_PIN_64<<3) | (_PIN_65));
    MCU_PIN_SHARE_CTRL02_FF98 = 0xA3;
    MCU_PIN_SHARE_CTRL03_FF99 = 0x89;
    MCU_PIN_SHARE_CTRL04_FF9A = 0x46;
    MCU_PIN_SHARE_CTRL05_FF9B = 0x37;
    MCU_PIN_SHARE_CTRL06_FF9C = 0xA4;
    MCU_PIN_SHARE_CTRL07_FF9D = 0x13;
    MCU_PIN_SHARE_CTRL08_FF9E = 0x33;
    MCU_PIN_SHARE_CTRL09_FF9F = 0x74;
    MCU_PIN_SHARE_CTRL0A_FFA0 = 0x24;
    MCU_PIN_SHARE_CTRL0B_FFA1 = 0x55;
    MCU_PIN_SHARE_CTRL0C_FFA2 = 0x52;
    MCU_PIN_SHARE_CTRL0D_FFA3 = 0x22;
    MCU_PIN_SHARE_CTRL0E_FFA4 = 0x0F;
}


//----------------------------------------------------------------------------------------------------
// Debug Functions
//----------------------------------------------------------------------------------------------------

//--------------------------------------------------
// Description  : ISP process
// Input Value  : None
// Output Value : None
//--------------------------------------------------
#if(_DEBUG_TOOL == _ISP_FOR_RTD3580D_EMCU)
void CMiscIspack(void)
{
    BYTE halt = 0;

    do
    {
        if(bRunCommand)
        {
            switch(ucDdcciData[0])
            {

                case 0x80:
                    halt = ucDdcciData[1];
                    break;

                // andy extand 
                case 0x10:
                     ucVirtualKey = ucDdcciData[1];
                     break;                    

                case 0x41:
                    MCU_I2C_IRQ_CTRL2_FF2A  |= 0x20;
                    CScalerRead(ucDdcciData[1], 1, pData, _NON_AUTOINC);
                    TxBUF= pData[0];
                    MCU_I2C_DATA_OUT_FF26 = pData[0];
                    break;

                case 0x44:
                    MCU_I2C_IRQ_CTRL2_FF2A  |= 0x20;
                    CI2cRead(ucDdcciData[2], ucDdcciData[1], 1, pData);
                    TxBUF = pData[0];
                    MCU_I2C_DATA_OUT_FF26 = TxBUF;
                    
                    break;

                // for RTD & I2c Device
                case 0x22:
                    ucDdcciData[2] = ucDdcciData[1];
                    break;

                case 0x28:
                    ucDdcciData[3] = ucDdcciData[1];
                    break;

                case 0x24:
                    CI2cWrite(ucDdcciData[2], ucDdcciData[1], 1, &ucDdcciData[3]);
                    
                    break;
                    
                case 0x20:
                    CScalerSetByte(ucDdcciData[2], ucDdcciData[1]);
                    break;

                default:

                    break;
            }

            bRunCommand=0;
            ucDdcciCommandNumber = 0;
        }
    }
    while(halt != 0);
}

//--------------------------------------------------
void GetVirtualKey(void)
{
    if(ucKeyMessage == _NONE_KEY_MESSAGE)
    {
        ucKeyMessage = ucVirtualKey;
    }
    ucVirtualKey = _NONE_KEY_MESSAGE;
}
//--------------------------------------------------

// Description  : ISP for RTD3580D Embeded MCU Initial
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CInitIspack(void) 
{
    MCU_I2C_SET_SLAVE_FF23 = 0x6a;
    MCU_I2C_IRQ_CTRL_FF28  = 0x3f;
    MCU_IRQ_PRIORITY_FF01  = 0x01;      //assign int1 (IRQ2)

    EX1  = 1;
}

#endif
//--------------------------------------------------

#if(_DEBUG_TOOL == _ISP_FOR_DDCCI && _SUPPORTDDCCI)
void CDDCCISendReply(BYTE length)
{
    BYTE chk;
    BYTE i;

    ucDDCCI_TxBuf[0] = length | 0x80;

    chk = 0x50 ^ 0x6E;
    for (i = 0; i <= length; i++)
    {
        chk ^= ucDDCCI_TxBuf[i];
    }
    ucDDCCI_TxBuf[length + 1] = chk;

    EA = 0;
    // Reset buffer and set DATA_BUF_WEN = 1 so MCU can write TX buffer
    MCU_I2C_IRQ_CTRL2_FF2A |= 0x60;

    // Pre-load header byte 0x6E
    MCU_I2C_DATA_OUT_FF26 = 0x6E;

    // Remaining bytes to send via DDCCI_TxInt on subsequent DOUTI interrupts:
    // ucDDCCI_TxBuf[0] (length byte) through ucDDCCI_TxBuf[length + 1] (checksum byte)
    // Total count = length + 2 bytes!
    txBufferPtr = &ucDDCCI_TxBuf[0];
    ucDDCCI_TxCount = length + 2;

    MCU_I2C_STATUS2_FF29 = 0x00;
    EA = 1;
}


void CDDCCIVesaHandler(void)
{
    BYTE cmd;
    BYTE opcode;
    BYTE val;
    BYTE i;
#if(_HDMI_AUDIO_SUPPORT == _ON)
    static code BYTE tCapStr[] = "(prot(monitor)type(LCD)model(EHD)cmds(01 02 03 07 0C E3 F3)vcp(04 10 12 14 16 18 1A 60 62 8A 8D D6 DF)mccs_ver(2.1))";
#else
    static code BYTE tCapStr[] = "(prot(monitor)type(LCD)model(EHD)cmds(01 02 03 07 0C E3 F3)vcp(04 10 12 14 16 18 1A 60 8A D6 DF)mccs_ver(2.1))";
#endif
    WORD usOffset;
    BYTE ucCapLen;
    BYTE ucChunkLen;

    if (rxStatus != DDC2B_COMPLETED)
        return;

    cmd = ucDDCCI_RxBuf[1];

    if (cmd == DDC2B_CMD_CapabilitiesRequest) // 0xF3
    {
        usOffset = (((WORD)ucDDCCI_RxBuf[2]) << 8) | ucDDCCI_RxBuf[3];
        ucCapLen = sizeof(tCapStr) - 1;
        ucChunkLen = 0;

        if (usOffset < ucCapLen)
        {
            ucChunkLen = ucCapLen - usOffset;
            if (ucChunkLen > 24)
                ucChunkLen = 24;
        }

        ucDDCCI_TxBuf[1] = DDC2B_CMD_CapabilitiesRequest_Reply; // 0xE3
        ucDDCCI_TxBuf[2] = ucDDCCI_RxBuf[2]; // Offset High
        ucDDCCI_TxBuf[3] = ucDDCCI_RxBuf[3]; // Offset Low
        for (i = 0; i < ucChunkLen; i++)
        {
            ucDDCCI_TxBuf[4 + i] = tCapStr[usOffset + i];
        }
        CDDCCISendReply(ucChunkLen + 3);
        DDCCI_InitRx();
    }
    else if (cmd == DDC2B_CMD_GetVCPFeature) // 0x01
    {
        opcode = ucDDCCI_RxBuf[2];
        ucDDCCI_TxBuf[1] = DDC2B_CMD_GetVCPFeature_Reply; // 0x02
        ucDDCCI_TxBuf[2] = 0x00; // Result Code: 0x00 = No Error
        ucDDCCI_TxBuf[3] = opcode;
        ucDDCCI_TxBuf[4] = 0x00; // Type: 0 = Set parameter

        if (opcode == DDC2B_CMD_VCP_Brightness) // 0x10 (Brightness)
        {
            ucDDCCI_TxBuf[5] = 0x00; // Max High
            ucDDCCI_TxBuf[6] = 100;  // Max Low = 100
            ucDDCCI_TxBuf[7] = 0x00; // Cur High
            ucDDCCI_TxBuf[8] = stSystemData.BackLight; // Cur Low
        }
        else if (opcode == DDC2B_CMD_VCP_Contrast) // 0x12 (Contrast)
        {
            ucDDCCI_TxBuf[5] = 0x00; // Max High
            ucDDCCI_TxBuf[6] = 100;  // Max Low = 100
            ucDDCCI_TxBuf[7] = 0x00; // Cur High
            ucDDCCI_TxBuf[8] = GET_CONTRAST(); // Cur Low
        }
#if(_HDMI_AUDIO_SUPPORT == _ON)
        else if (opcode == 0x62) // Audio Volume (0..100)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 100;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = GET_VOLUME();
        }
#endif
        else if (opcode == 0x8A) // TV/Color Saturation (0..100)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 100;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = GET_SATURATION();
        }
#if(_HDMI_AUDIO_SUPPORT == _ON)
        else if (opcode == 0x8D) // Audio Mute (1 = Mute, 2 = Unmute)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 0x02;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = GET_AUDIO_MUTE() ? 0x01 : 0x02;
        }
#endif
        else if (opcode == 0x16) // Red Gain (0..255)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 255;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = stColorTempData.ColorTemp[_RED];
        }
        else if (opcode == 0x18) // Green Gain (0..255)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 255;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = stColorTempData.ColorTemp[_GREEN];
        }
        else if (opcode == 0x1A) // Blue Gain (0..255)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 255;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = stColorTempData.ColorTemp[_BLUE];
        }
        else if (opcode == 0x14) // Select Color Preset (9300K, 6500K, User)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 0x0B;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = (GET_COLOR_TEMP_TYPE() == _CT_9300) ? 0x05 : ((GET_COLOR_TEMP_TYPE() == _CT_6500) ? 0x08 : 0x0B);
        }
        else if (opcode == 0x60) // Input Source (0x01 = VGA, 0x11 = HDMI-1)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 0x11;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = (_GET_INPUT_SOURCE() == _SOURCE_HDMI) ? 0x11 : 0x01;
        }
        else if (opcode == 0x04) // Factory Defaults
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 0x01;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = 0x00;
        }
        else if (opcode == DDC2B_CMD_VCP_Version) // 0xDF (VCP Version)
        {
            ucDDCCI_TxBuf[5] = 0x00; // Max High
            ucDDCCI_TxBuf[6] = 0xFF; // Max Low
            ucDDCCI_TxBuf[7] = 0x02; // MCCS 2.1 (High = 2)
            ucDDCCI_TxBuf[8] = 0x01; // MCCS 2.1 (Low = 1)
        }
        else if (opcode == DDC2B_CMD_VCP_PowerStatus) // 0xD6 (Power Status)
        {
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 0x04;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = GET_POWERSTATUS() ? 0x01 : 0x04; // 1 = DPM On, 4 = Off
        }
        else
        {
            ucDDCCI_TxBuf[2] = 0x01; // Result Code: 0x01 = Unsupported VCP code
            ucDDCCI_TxBuf[5] = 0x00;
            ucDDCCI_TxBuf[6] = 0x00;
            ucDDCCI_TxBuf[7] = 0x00;
            ucDDCCI_TxBuf[8] = 0x00;
        }
        CDDCCISendReply(8);
        DDCCI_InitRx();
    }
    else if (cmd == DDC2B_CMD_SetVCPFeature) // 0x03
    {
        opcode = ucDDCCI_RxBuf[2];
        val = ucDDCCI_RxBuf[4]; // value low byte

        if (opcode == DDC2B_CMD_VCP_Brightness) // 0x10
        {
            if (val > 100) val = 100;
            stSystemData.BackLight = val;
            CAdjustBacklight();
            CEepromSaveSystemData();
        }
        else if (opcode == DDC2B_CMD_VCP_Contrast) // 0x12
        {
            if (val > 100) val = 100;
            SET_CONTRAST(val);
            CAdjustContrast();
            CEepromSaveBriConData();
        }
#if(_HDMI_AUDIO_SUPPORT == _ON)
        else if (opcode == 0x62) // Audio Volume (0..100)
        {
            if (val > 100) val = 100;
            SET_VOLUME(val);
            CAdjustAudio();
            CSetVolume();
            CEepromSaveAudioData();
        }
#endif
        else if (opcode == 0x8A) // Saturation (0..100)
        {
            if (val > 100) val = 100;
            SET_SATURATION(val);
            CAdjustYpbprSaturation(GET_SATURATION());
            CEepromSaveHueSatData();
        }
#if(_HDMI_AUDIO_SUPPORT == _ON)
        else if (opcode == 0x8D) // Audio Mute (1 = Mute, 2 = Unmute)
        {
            if (val == 0x01)
                SET_AUDIO_MUTE();
            else if (val == 0x02)
                CLR_AUDIO_MUTE();
            CSetVolume();
            CEepromSaveAudioData();
        }
#endif
        else if (opcode == 0x16) // Red Gain
        {
            stColorTempData.ColorTemp[_RED] = val;
            CAdjustContrast();
            CEepromSaveColorTempData();
        }
        else if (opcode == 0x18) // Green Gain
        {
            stColorTempData.ColorTemp[_GREEN] = val;
            CAdjustContrast();
            CEepromSaveColorTempData();
        }
        else if (opcode == 0x1A) // Blue Gain
        {
            stColorTempData.ColorTemp[_BLUE] = val;
            CAdjustContrast();
            CEepromSaveColorTempData();
        }
        else if (opcode == 0x14) // Select Color Preset
        {
            if (val == 0x05)
                SET_COLOR_TEMP_TYPE(_CT_9300);
            else if (val == 0x08)
                SET_COLOR_TEMP_TYPE(_CT_6500);
            else
                SET_COLOR_TEMP_TYPE(_CT_USER);
            CEepromLoadColorTempData();
            CAdjustContrast();
            CEepromSaveSystemData();
        }
        else if (opcode == 0x04) // Factory Reset
        {
            if (val == 0x01)
            {
                CDoReset();
            }
        }
        DDC2Bi_InitTx();
        DDCCI_InitRx();
    }
    else if (cmd == DDC2B_CMD_SaveCurrentSettings) // 0x0C
    {
        CEepromSaveSystemData();
        CEepromSaveBriConData();
        CEepromSaveAudioData();
        CEepromSaveHueSatData();
        CEepromSaveColorTempData();
        DDC2Bi_InitTx();
        DDCCI_InitRx();
    }
    else
    {
        DDC2Bi_InitTx();
        DDCCI_InitRx();
    }
}
#endif

/**
 * CMiscIspDebugProc
 * Debug tool process
 * @param <none>
 * @return {none}
 *
 */
void CMiscIspDebugProc(void)
{
    #if(_DEBUG_TOOL == _ISP_FOR_RTD3580D_EMCU)
    CMiscIspack();
    #endif

    #if(_DEBUG_TOOL == _ISP_FOR_DDCCI && _SUPPORTDDCCI)
    CDDCCIVesaHandler();
    // Do NOT call CDDCCICommand() from DDCCI.obj here - it unconditionally resets the TX buffer to NULL!
    #endif

    #if(_RS232_EN)
    CUartHandler();
    #endif
}

//--------------------------------------------------
//--------------------------------------------------
void CDDCCIInitial()
{
    #if(_DEBUG_TOOL == _ISP_FOR_RTD3580D_EMCU)
    CInitIspack();
    #endif

    #if((_DEBUG_TOOL == _ISP_FOR_DDCCI) && _SUPPORTDDCCI)
    // CInitDDCCI blob sets:
    //   FF2A |= 0x80
    //   FF23 = 0x6E  (slave 0x37, bit 0 = 0 -> ADC DDC / VGA!)
    //   FF28 = 0x0C  (DOI_EN | DII_EN)
    //   FF01 = 0x01
    //   EX1 = 1
    CInitDDCCI();

    // Route DDC-CI to DDC2 (HDMI).
    // In RTD2660 register 0xFF23:
    //   Bits 7:1 = 7-bit slave address (0x37)
    //   Bit 0    = CH_SEL: 0 = ADC DDC (VGA), 1 = DDC2 (HDMI/DVI)
    // Therefore 0x6E routes to VGA, and 0x6F routes to HDMI/DDC2!
    MCU_I2C_SET_SLAVE_FF23    = 0x6F;
    MCU_I2C_CHANNEL_CTRL_FF2B = 0x00; // Bit 0 (HCH_SEL) = 0 -> controlled by FF23[0] (CH_SEL)
    MCU_I2C_IRQ_CTRL_FF28     = 0x0C; // DOI_EN (0x08) | DII_EN (0x04)
    MCU_I2C_STATUS2_FF29      = 0x00; 
    MCU_I2C_IRQ_CTRL2_FF2A    = 0x80; // AUTO_RST_BUF = 1, DATA_BUF_WEN = 0 (Host write enabled)

    // Pre-initialize TX buffer with valid DDC/CI NULL string (0x6E, 0x80, 0xBE, 0x6E)
    // so any read from host receives valid header 0x6E instead of uninitialized memory.
    DDC2Bi_InitTx();
    IP |= 0x04; // Set high priority for External Interrupt 1 (DDC/CI)
    EX1 = 1;
    #endif



    CInitEdid();
}
//--------------------------------------------------
void CInitEdid(void)
{
    MCU_HDMI_DDC_ENA_FF2C = 0x00;  // Disable HDMI DDC while loading RAM
    MCU_DVI_DDC_ENA_FF1E  = 0x00;  // Disable DVI DDC while loading RAM
    MCU_DDCRAM_PART_FF21  = 0x2B;  // Partition DDC RAM (from live_dump @ 0xE6B9)

    CLoadEdid();                   // Load EDID data into DDC SRAM FIRST!

    MCU_DVI_DDC_ENA_FF1E  = 0x03;  // Enable DVI/HDMI DDC (from live_dump @ 0xE6AF)
    MCU_HDMI_DDC_ENA_FF2C = 0x03;  // Enable HDMI DDC (from live_dump @ 0xE6B5)
    MCU_ADC_DDC_ENA_FF1B  = 0x03;  // Enable ADC DDC (from live_dump @ 0xE6BF)
}




//--------------------------------------------------
void CLoadEdid(void)
{
    WORD i;

    BYTE xdata *p;
    BYTE code *pEdid;
    
    #if(_HDMI_EDID == _ON) 
    p = MCU_DDCRAM_HDMI;
    pEdid = &tHDMI_EDID_DATA[0];
    for(i=0;i<256;i++)
    {
        *p++ = *pEdid++;
    }
    #endif

    #if(_DVI_EDID == _ON)
    p = MCU_DDCRAM_DVI;
    pEdid = &tDVI_EDID_DATA[0];
    for(i=0;i<128;i++)
    {
        *p++ = *pEdid++;
    }
    
    #endif


    #if(_VGA_EDID == _ON)
    p = MCU_DDCRAM_VGA;
    pEdid = &tVGA_EDID_DATA[0];
    for(i=0;i<128;i++)
    {
        *p++ = *pEdid++;
    }
    #endif  
}
//--------------------------------------------------

