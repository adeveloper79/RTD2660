//----------------------------------------------------------------------------------------------------
// ID Code      : PNL_TM043_ST7701S.h
// Description  : Tianma 4.3" 480x800 ST7701S IPS Panel (TM043YYSP21, 24-bit TTL RGB Interface)
//----------------------------------------------------------------------------------------------------

#if(_PANEL_TYPE == _PNL_TM043_ST7701S)

#define _PANEL_LR_UD_FUNC				_OFF
//--------------------------------------------------
//_PANEL_TTL / _PANEL_HZ / _PANEL_LVDS / _PANEL_RSDS
#define _OUTPUT_BUS                     _PANEL_TTL

//--------------------------------------------------
// _DISP_24_BIT / _DISP_18_BIT
#define _DISP_BIT                       _DISP_24_BIT

//--------------------------------------------------
#define _DISP_EO_SWAP                   0
#define _DISP_RB_SWAP                   0
#define _DISP_ML_SWAP                   0

#define _RSDS_GC_SWAP                   0
#define _RSDS_HL_SWAP                   0
#define _RSDS_PN_SWAP                   0

//--------------------------------------------------
// _DISP_SINGLE_PORT / _DISP_DOUBLE_PORT
#define _DISPLAY_PORT                   _DISP_SINGLE_PORT

//--------------------------------------------------
#define _DISP_WID                       480
#define _DISP_LEN                       800

#define _DH_ACT_STA_POS                 48
#define _DH_ACT_END_POS                 (_DISP_WID + _DH_ACT_STA_POS)

#define _DV_ACT_STA_POS                 19
#define _DV_ACT_END_POS                 (_DISP_LEN + _DV_ACT_STA_POS)

//--------------------------------------------------
// _LVDS_MAP1 / _LVDS_MAP2
#define _LVDS_MAP                       _LVDS_MAP1

//--------------------------------------------------
#define _STD_DH_TOTAL                   710
#define _STD_DV_TOTAL                   845

#define _STD_PIXCEL_CLOCK               36

#define _HSYNC_WIDTH                    16
#define _VSYNC_HEIGHT                   3

//--------------------------------------------------
// Definitions for Display VIDEO/TV Signal
//--------------------------------------------------
#define _VIDEO_50_DH_TOTAL              _STD_DH_TOTAL
#define _REGSISTER40_50HZ               0x00
#define _REGSISTER41_50HZ               0x36
#define _REGSISTER42_50HZ               0x1B
#define _REGSISTER43_50HZ               0x02
#define _P1_REGSISTERC4_50HZ            0x18
#define _P1_REGSISTERC5_50HZ            0x9B

#define _VIDEO_60_DH_TOTAL              _STD_DH_TOTAL
#define _REGSISTER40_60HZ               0x00
#define _REGSISTER41_60HZ               0x22
#define _REGSISTER42_60HZ               0x08
#define _REGSISTER43_60HZ               0x02
#define _P1_REGSISTERC4_60HZ            0x18
#define _P1_REGSISTERC5_60HZ            0x83

//--------------------------------------------------
// Definitions of DPLL N Code
//--------------------------------------------------
#define _DPLL_N_CODE                    16

//--------------------------------------------------
// Set Last Line Finetune Method
//--------------------------------------------------
#define _LAST_LINE_FINETUNE             _LAST_LINE_METHOD_NONE
#define _LAST_LINE_TARGET               400

//--------------------------------------------------
#define _AUTOCALC_PIXCLK                (_DISABLE << 2)     

//--------------------------------------------------
// Definitions for First DHS Masking
//--------------------------------------------------
#define _DHS_MASK                       (1 << 7)

//--------------------------------------------------
// Definitions for Display Signal (Active-Low Syncs for ST7701S)
//--------------------------------------------------
#define _DVS_INVERT                     (_ENABLE << 2)
#define _DHS_INVERT                     (_ENABLE << 1)
#define _DEN_INVERT                     (_DISABLE)

#define _DISPLAY_INVERT                 (_DVS_INVERT | _DHS_INVERT | _DEN_INVERT)

//--------------------------------------------------
// Definitions for Display Control
//--------------------------------------------------
#define _DISPLAY_CTRL                   (_DHS_MASK | _DISPLAY_INVERT)

//--------------------------------------------------
// TTL panel setting
//--------------------------------------------------
#define _TTL_DCLK_DELAY                 0
#define _TTL_DCLK_OUTPUT                _ENABLE
#define _TTL_DCLK_INVERT                _ENABLE

//--------------------------------------------------
// Support Timing
//--------------------------------------------------
#define _H_FREQ_MAX                     930             // Unit in 0.1 kHZ
#define _H_FREQ_MIN                     200             // Unit in 0.1 kHZ
#define _V_FREQ_MAX                     780             // Unit in 0.1 HZ
#define _V_FREQ_MIN                     490             // Unit in 0.1 HZ

#define LVDS_CTRL_LEVEL                 4

//--------------------------------------------------
#define _TVNUMBER_POSITION_OFFSET       64
#define _TVSIGNAL_POSITION_OFFSET       64

#ifdef __RTDACCESS__

//--------------------------------------------------
code PanelType Panel =
{
	// Panel Style
    _OUTPUT_BUS      |
    _AUTOCALC_PIXCLK |
    _LVDS_MAP        |	
    _DISP_BIT        | 
    _RSDS_GC_SWAP    |
    _RSDS_HL_SWAP    |
    _RSDS_PN_SWAP,

	// Panel Configuration
	_DISP_EO_SWAP |
	_DISP_RB_SWAP |
	_DISP_ML_SWAP |
	_DISPLAY_PORT |
	_DISPLAY_CTRL,

    _DH_ACT_STA_POS,  // Display Horizontal Start Position
    _DISP_WID,        // Display Horizontal Width

    _STD_DH_TOTAL,    // Display Horizontal Total Clock Number in One Display Line

    _DV_ACT_STA_POS,  // Display Vertical Start Position
    _DISP_LEN,        // Display Vertical Height

    _STD_DV_TOTAL,    // Display Vertical Total Line Number in One Frame

    _HSYNC_WIDTH,     // Display H Sync Width
    _VSYNC_HEIGHT,    // Display V Sync Height

    _STD_PIXCEL_CLOCK,// Typical Pixel Clock in MHz

    _H_FREQ_MAX,      // H Sync Max Freq Unit in 0.1 kHZ
    _H_FREQ_MIN,      // H Sync Min Freq Unit in 0.1 kHZ
    _V_FREQ_MAX,      // V Sync Max Freq Unit in 0.1 HZ
    _V_FREQ_MIN,      // V Sync Min Freq Unit in 0.1 HZ

    // TV PAL
    _VIDEO_50_DH_TOTAL,
#if(_VIDEO_TV_SUPPORT)
    _REGSISTER40_50HZ,
    _REGSISTER41_50HZ,
    _REGSISTER42_50HZ,
    _REGSISTER43_50HZ,
    _P1_REGSISTERC4_50HZ,
    _P1_REGSISTERC5_50HZ,
#endif

    // TV NTSC
    _VIDEO_60_DH_TOTAL,
#if(_VIDEO_TV_SUPPORT)
    _REGSISTER40_60HZ,
    _REGSISTER41_60HZ,
    _REGSISTER42_60HZ,
    _REGSISTER43_60HZ,
    _P1_REGSISTERC4_60HZ,
    _P1_REGSISTERC5_60HZ,
#endif

    // TTL setting   
    (_TTL_DCLK_DELAY << 4)  |
    (_TTL_DCLK_OUTPUT << 1) |
    _TTL_DCLK_INVERT,

};

#if(_VGA_EDID)
BYTE code tVGA_EDID_DATA[] =
{
    0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x4a,0x8b,0x00,0x00,0x01,0x01,0x01,0x01,
    0x1e,0x0c,0x01,0x01,0x0e,0x24,0x1b,0x78,0xe8,0x8a,0x01,0x9a,0x58,0x52,0x8b,0x28,
    0x1e,0x50,0x54,0xff,0xff,0x80,0x61,0x40,0x61,0x4f,0x61,0x59,0x71,0x4f,0x81,0x40,
    0x81,0x59,0x81,0x99,0xa9,0x40,0x00,0x00,0x00,0xfc,0x00,0x31,0x37,0x27,0x27,0x20,
    0x4c,0x43,0x44,0x0a,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xfc,0x00,0x4d,0x6f,0x6e,
    0x69,0x74,0x6f,0x72,0x0a,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xfd,0x00,0x2b,
    0x55,0x14,0x5c,0x0e,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xff,
    0x00,0x30,0x30,0x30,0x30,0x30,0x31,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0xbd,
};
#endif

#if(_DVI_EDID)
BYTE code tDVI_EDID_DATA[] =
{
    // Exact 480x800 @ 60Hz EDID
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x15, 0x04, 0x08, 0x50, 0x01, 0x00, 0x00, 0x00,
    0x1C, 0x21, 0x01, 0x03, 0x81, 0x0C, 0x08, 0x78, 0x2E, 0x35, 0x85, 0xA6, 0x56, 0x48, 0x9A, 0x24,
    0x12, 0x50, 0x54, 0xAF, 0xEF, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x10, 0x0E, 0xE0, 0xE6, 0x10, 0x20, 0x2D, 0x30, 0x10, 0x50,
    0x13, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x10, 0x00, 0x38, 0x2D, 0x40,
    0x10, 0x2C, 0x45, 0x80, 0xB9, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x10, 0x00, 0x38,
    0x2D, 0x40, 0x10, 0x2C, 0x45, 0x80, 0xB9, 0x88, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xFC,
    0x00, 0x45, 0x48, 0x44, 0x2D, 0x35, 0x30, 0x30, 0x38, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x01, 0x40,
};
#endif

//--------------------------------------------------
WORD code tVIDEO_TABLE_INPUT_INFO[][5] =
{
	{ 864,    167,    658,    31,    274 },
	{ 858,    161,    660,    26,    232 },
};

#define _TCON(x)							(x * 8 + 0x08)

BYTE code tTTL_INITIAL[] =
{
#if(_OUTPUT_BUS == _PANEL_TTL)    
    5,  _AUTOINC,       _TCON_ADDR_PORT_8B,     0x00, 0x80,     // TCON Global Enable
    5,  _AUTOINC,       _TCON_ADDR_PORT_8B,     0x01, 0x3E,     // Output Port Enable
    5,  _AUTOINC,       _TCON_ADDR_PORT_8B,     0x78, 0xF0,     // LVDS/TTL Pad Power Up
    4,  _NON_AUTOINC,   _DISP_ACCESS_PORT_2A,   0x23,
    4,  _NON_AUTOINC,   _DISP_DATA_PORT_2B,     0x40,           // DE phase calibration
    4,  _NON_AUTOINC,   _DISP_ACCESS_PORT_2A,   0x22,
    4,  _NON_AUTOINC,   _DISP_DATA_PORT_2B,     0xA0,           // Sync width calibration
    4,  _NON_AUTOINC,   _DISP_ACCESS_PORT_2A,   0x20,
    4,  _NON_AUTOINC,   _DISP_DATA_PORT_2B,     0x02,           // DCLK output enable
#endif
    _END
};

BYTE code tLVDS_INITIAL[] =
{
    _END
};

#else

extern code PanelType Panel;
extern WORD code tVIDEO_TABLE_INPUT_INFO[][5];
extern BYTE code tTTL_INITIAL[];
extern BYTE code tLVDS_INITIAL[];
extern WORD code tModeLimitTable[];
extern BYTE code tVGA_EDID_DATA[];
extern BYTE code tDVI_EDID_DATA[];

#endif // __RTDACCESS__

#endif // #if(_PANEL_TYPE == _PNL_TM043_ST7701S)
