#include "Core\Header\include.h"


#if(_KEY_TYPE == _KT_PCB2660_003)

#define AD_KEY_STEP    3

//--------------------------------------------------
// Description  : Get key status
// Input Value  : None
// Output Value : Return Key status
//--------------------------------------------------
#if(_KEY_SCAN_TYPE == _KEY_SCAN_NORMAL)

BYTE CKeyScanIO(void)
{
	BYTE ucKeyState = _NONE_KEY_MASK;    
    
    if(!bKey0)   	ucKeyState    = ucKeyState | _AT_KEY0;
    if(!bKey1)   	ucKeyState    = ucKeyState | _AT_KEY1;
    if(!bKey2)   	ucKeyState    = ucKeyState | _AT_KEY2;
    if(!bKey3)   	ucKeyState    = ucKeyState | _AT_KEY3;
    if(!bKey4)   	ucKeyState    = ucKeyState | _AT_KEY4;
    if(!bKey5)   	ucKeyState    = ucKeyState | _AT_KEY5;
    if(!bKey6)   	ucKeyState    = ucKeyState | _AT_KEY6;
    if(!bKey7)   	ucKeyState    = ucKeyState | _AT_KEY7;
    
	return ucKeyState;
}
//--------------------------------------------------
BYTE CIOMaskToKeyMaskPreset(BYTE ucIOMask)
{
	BYTE code *p = tIOKeyMask;
	BYTE ucKeyState = _NONE_KEY_MASK;
	
	while(1)
	{
		if(p[0] == 0xFF && p[1] == 0xFF)
			break;

		if(ucIOMask & p[1])
			ucKeyState |= p[0];	

		p += 2;
	}

	return ucKeyState;
}
//--------------------------------------------------
BYTE CIOMaskToKeyMask(BYTE ucIOMask)
{
	if(g_bUsePresetKey)
	{
		return CIOMaskToKeyMaskPreset(ucIOMask);
	}
	else
	{
		return CIOMaskToKeyMaskExt(ucIOMask);
	}
}

#else

BYTE CKeyScanIO(void)
{
	return 0;
}

#endif
//--------------------------------------------------
BYTE CKeyScan(void)
{
    BYTE ucKeyState = _NONE_KEY_MASK;    

#if(_KEY_SCAN_TYPE == _KEY_SCAN_NORMAL)
    ucKeyState = CKeyScanIO();
    ucKeyState = CIOMaskToKeyMask(ucKeyState);
#else
    BYTE ad0 = CGetADCAValue(MCU_ADC0); // Pin 50 (ADCA0)
    BYTE ad1 = CGetADCAValue(MCU_ADC1); // Pin 51 (ADCA1)

    // Pin 50: Power Button (active LOW / GND pull -> ad0 <= 25)
    if(ad0 <= 25)
    {
        ucKeyState = _POWER_KEY_MASK;
    }
    // Pin 51: Brightness Cycle Button (active LOW / GND pull -> ad1 <= 25)
    else if(ad1 <= 25)
    {
        ucKeyState = _LEFT_KEY_MASK;
    }
    else
    {
        ucKeyState = _NONE_KEY_MASK;
    }
#endif

    if(ucKeyState != _NONE_KEY_MASK)
       CKeyInitial();

    return ucKeyState;
}
//--------------------------------------------------
/*
void CScanAutoKey(void)
{
     if(GET_POWERSTATUS())
     {
         // check 3 sec do change source ???
         #if(_TMDS_SUPPORT)
         {
             if(CheckKeyUp(_AUTO_KEY_MASK,50))
                ucKeyMessage = _AUTO_KEY_MESSAGE;
             else ucKeyMessage = _SOURCE_KEY_MESSAGE;
         }
         #else
         {
             CKeyMessageConvert(_AUTO_KEY_MASK, _AUTO_KEY_MESSAGE);
         }
         #endif
     }
     else
     {
         if(CheckKeyDown(_AUTO_KEY_MASK | _POWER_KEY_MASK,20))
         {
             SET_POWERSWITCH();
             ucKeyMessage = _AUTO_POWER_KEY_MESSAGE;
         }
     }
}
*/
//--------------------------------------------------

void CheckMenuKey(void)
{
	if(CheckKeyUp(_MENU_KEY_MASK,250) == 0)
	{
       if(stSystemData.InputSource == _SOURCE_VGA)
	    {		
	    	ucKeyMessage = _FAC_KEY_MESSAGE;
		}
		else
		{
		    CKeyMessageConvert(_MENU_KEY_MASK, _MENU_KEY_MESSAGE);
		}

	}
	else
	{
		CKeyMessageConvert(_MENU_KEY_MASK, _MENU_KEY_MESSAGE);
	}
		
}
//--------------------------------------------------

void CKeyMessageProc(void)
{          
    switch(ucKeyStateCurr)
    {        
        case _MENU_KEY_MASK:	 //Menu
            CheckMenuKey();     
            break;
/*        case _UP_KEY_MASK:	     
            CKeyMessageConvert(_UP_KEY_MASK, _UP_KEY_MESSAGE);
            break;
        
           case _DOWN_KEY_MASK:	 
            CKeyMessageConvert(_DOWN_KEY_MASK, _DOWN_KEY_MESSAGE);
            break;
*/

        case _SLEEP_KEY_MASK:	     
            CKeyMessageConvert(_SLEEP_KEY_MASK, _SLEEP_KEY_MESSAGE);
            break;
        
        case _DISPLAYMODE_KEY_MASK:	 
            CKeyMessageConvert(_DISPLAYMODE_KEY_MASK, _DISPLAY_RATIO_MESSAGE);
            break;

        case _TCON_KEY_MASK:	 
            CKeyMessageConvert(_TCON_KEY_MASK, _TCON_MESSAGE);
            break;

        case _LEFT_KEY_MASK:     //Left
            CKeyMessageConvert(_LEFT_KEY_MASK, _LEFT_KEY_MESSAGE);
            break;

        case _RIGHT_KEY_MASK:    //Right
            CKeyMessageConvert(_RIGHT_KEY_MASK, _RIGHT_KEY_MESSAGE);
            break; 
               
        case _SOURCE_KEY_MASK:
            CKeyMessageConvert(_SOURCE_KEY_MASK, _SOURCE_KEY_MESSAGE);
            break;
        case _Reset_KEY_MASK:
	     //CKeyMessageConvert(_Reset_KEY_MASK, _Reset_KEY_MESSAGE);	
	    break;	 
        default:
            CLR_KEYREPEATSTART();
            CTimerCancelTimerEvent(CKeyRepeatEnableTimerEvent);
            break;
    }
} 
//--------------------------------------------------
              
#endif      //#if(_KEY_TYPE == _KT_PCB2660_003)
