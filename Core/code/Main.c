//----------------------------------------------------------------------------------------------------
// Copyright of Realtek SemiConductor Corp.
// Version : V1.00c (09.15.2009)
//
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
// ID Code      : Main.c No.0002
// Update Note  :
//
//----------------------------------------------------------------------------------------------------

#define __MAIN__

#include "Core\Header\Include.h"


/**
 * CMainUserInitial
 * Setting some flags for user when initial
 * @param <none>
 * @return {none}
 *
 */
void CMainUserInitial(void)
{
    // User can make some initial settings at firmware startup
    CLR_PANELPOWERSTATUS();
    CLR_LIGHTPOWERSTATUS();
    CLR_FRAMESYNCSTATUS();
    CLR_POWERSWITCH();
    CLR_FIRST_ADCCLOCK();
    CLR_OSD_READYFORDISPLAY();
#if(_HDMI_SUPPORT == _ON)
       CLR_HDMIINPUT();
#endif
    SET_FIRST_LOADFONT();
#if defined(CONFIG_VBI_ENABLE)
    CLR_OSDVBI_INITIAL();
#endif
    //SET_SOURCE_AUTOCHANGE();
    CLR_SOURCE_AUTOCHANGE();
    SET_PRE_VGA_CONNECT(bVGACONNECT);
#if(_TMDS_SUPPORT == _ON)
#if (_HDMI_SUPPORT == _ON)
    SET_PRE_HDMI_CONNECT(bHDMICONNECT);
#else
    SET_PRE_DVI_CONNECT(bDVICONNECT);
#endif
#endif
    CLR_USE_TRANSITION_RESULT();
    CheckLanguage();
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------

/**
 * CMainSystemInitial
 * Initial the system for once when power up
 * @param <none>
 * @return {none}
 *
 */
void CMainSystemInitial(void)
{
    // Initial MCU
    CMcuInitial();

    // 1. Initial timer events
    CTimerInitialTimerEvent();

    // 2. Configure PinShare FIRST: switches Pins 52..55 (SDA,SCL,CS,RST) from default Input to Push-Pull Output!
    CMiscSetPinShare();

    // 3. Keep HPD LOW during boot so handheld does not send premature HDMI data
    bHot_Plug = _HOT_PLUG_LOW;
    bHot_Plug2 = _HOT_PLUG_LOW;

    // 4. Power on Panel logic & VDD (exact factory live dump @ 0x0C8DC)
    MCU_PORT80_PIN_REG_FFD6 = 1; // bPANELPOWER = 1
    P1 &= ~0x02;                 // Pin 65 (P1.1) latch = 0 (LOW / 0V)
    bLIGHTPOWER = 1;             // Pin 64 = 1
    MCU_PORT75_PIN_REG_FFD4 = 0; // Pin 104 = 0
    _SET_INPUT_SOURCE(_SOURCE_HDMI);
    SET_POWERSTATUS();
    SET_PANELPOWERSTATUS();
    SET_LIGHTPOWERSTATUS();

    // 5. Initialize ST7701S panel over SPI with active push-pull output pins (exact factory @ 0x0C8F1)
    ST7701S_Init();

    // 6. Set Free-Run DPLL clock and enable display output (exact factory @ 0x0C8F4 & 0x0C906)
    CModeSetFreeRun();
    CScalerSetBit(0x01, 0xFE, 0x01); // Enable display output
    CTimerDelayXms(20);

    // 7. Configure Scaler registers & EDID
    CScalerInitial();
    CDDCCIInitial();
    #if((_DEBUG_TOOL == _ISP_FOR_DDCCI) && _SUPPORTDDCCI)
    EX1 = 1;   // Enable INT1 (External Interrupt 1) for DDC/CI handler
    #endif
    CEepromStartupCheck();
    CPowerLightPowerOn();

    CLR_POWERSWITCH();
    ucCurrState = _INITIAL_STATE;

    if(GET_POWERSTATUS())
       CPowerLedRed();
    else
       CPowerLedOff();

    // Initial key scan status
    CKeyInitial();

    // Initial user settings
    CMainUserInitial();

#if(_RS232_EN)
    CUartInit();
#endif

    CIrdaInitial();
    GetExtendEnable();
    SetPanelLR();
    SetPanelUD();
    SetFM();
    CAdjustBacklight();

    // 8. Panel and scaler are completely initialized and active!
    // Assert HPD HIGH to notify HDMI source (handheld) to read EDID and send video
    bHot_Plug = _HOT_PLUG_HI;
    bHot_Plug2 = _HOT_PLUG_HI;
}

/**
 * main
 * main process
 * first system initial, then main loop
 * @param <none>
 * @return {none}
 *
*/
void main(void)
{
    CMainSystemInitial();
    
    do
    {
        CMiscIspDebugProc();                                         
        CTimerHandler();
        CKeyHandler();
        CModeHandler();
        COsdHandler(); 
    }while(_TRUE);
}

/*
void DbgLed(unsigned char ucCount)
{
	unsigned char i;	     
	for(i=0; i<ucCount; i++)
	{
		CPowerLedOrange();
		CTimerDelayXms(100);
		CPowerLedRed();
		CTimerDelayXms(100);
	}
	CPowerLedGreen();
}
  */
