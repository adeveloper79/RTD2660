#include "Core\Header\Include.h"

#if(_PANEL_TYPE == _PNL_TM043_ST7701 || _PANEL_TYPE == _PNL_TM043_ST7701S)

#define SPI_SDA     MCU_PORT62_PIN_REG_FFC9  // Pin 52
#define SPI_SCL     MCU_PORT63_PIN_REG_FFCA  // Pin 53
#define SPI_CS      MCU_PORT64_PIN_REG_FFCB  // Pin 54
#define SPI_RST     MCU_PORT65_PIN_REG_FFCC  // Pin 55

static void ST_ClockPulse(void)
{
    _nop_();
    SPI_SCL = 0;
    _nop_();
    SPI_SCL = 1;
    _nop_();
}

static void ST_DelayCS(void)
{
    BYTE k;
    for (k = 0; k < 16; k++) { _nop_(); }
}

static void ST7701S_Write_Cmd(BYTE cmd)
{
    BYTE i;
    SPI_CS = 1;   // Latch / end previous packet
    ST_DelayCS(); // 8-10us gap (4-5 samples at 500kHz matching original.csv)
    SPI_CS = 0;   // Assert CS for new command packet
    SPI_SDA = 0;  // 9th bit = 0 (Command)
    ST_ClockPulse();
    for (i = 0; i < 8; i++) {
        SPI_SDA = (cmd & 0x80) ? 1 : 0;
        ST_ClockPulse();
        cmd <<= 1;
    }
    // CS stays 0 for subsequent parameters (exact factory protocol)
}

static void ST7701S_Write_Data(BYTE dat)
{
    BYTE i;
    // CS stays 0 during all parameters of the command
    SPI_SDA = 1;  // 9th bit = 1 (Data parameter)
    ST_ClockPulse();
    for (i = 0; i < 8; i++) {
        SPI_SDA = (dat & 0x80) ? 1 : 0;
        ST_ClockPulse();
        dat <<= 1;
    }
    // CS stays 0 during all parameters of the command
}

static void ST7701S_EndPacket(void)
{
    _nop_();
    SPI_CS = 1;
    _nop_();
}

void ST7701S_Init(void)
{
    // Hold Reset LOW immediately while panel power rails and VDD stabilize on cold boot
    SPI_CS = 1;
    SPI_SCL = 1;
    SPI_SDA = 1;
    SPI_RST = 0;
    CTimerDelayXms(30);          // 30ms VDD stabilization with RESET held LOW

    // Idle HIGH before reset pulse
    SPI_RST = 1;
    CTimerDelayXms(10);          // 10ms idle HIGH

    // Hardware Reset Pulse LOW (exact 20ms = 9,830 samples matching factory @ 0x6B31)
    SPI_RST = 0;
    CTimerDelayXms(20);          // 20ms LOW

    // Release Reset HIGH
    SPI_RST = 1;
    CTimerDelayXms(20);          // 20ms
    CTimerDelayXms(120);         // 120ms (Total 140ms before SPI commands)

    // Sample 83,748: 100% Exact Factory ST7701S SPI Commands decoded from live dump 0x6B31
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x13);
    ST7701S_Write_Cmd(0xEF);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Cmd(0xC0);
    ST7701S_Write_Data(0xEB);
    ST7701S_Write_Data(0x03);
    ST7701S_Write_Cmd(0xC1);
    ST7701S_Write_Data(0x11);
    ST7701S_Write_Data(0x02);
    ST7701S_Write_Cmd(0xC2);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Cmd(0xCC);
    ST7701S_Write_Data(0x18);
    ST7701S_Write_Cmd(0xB0);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x0D);
    ST7701S_Write_Data(0x14);
    ST7701S_Write_Data(0x0D);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Data(0x05);
    ST7701S_Write_Data(0x02);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x1E);
    ST7701S_Write_Data(0x05);
    ST7701S_Write_Data(0x13);
    ST7701S_Write_Data(0x11);
    ST7701S_Write_Data(0xA3);
    ST7701S_Write_Data(0x29);
    ST7701S_Write_Data(0x18);
    ST7701S_Write_Cmd(0xB1);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x0C);
    ST7701S_Write_Data(0x14);
    ST7701S_Write_Data(0x0C);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Data(0x05);
    ST7701S_Write_Data(0x03);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x07);
    ST7701S_Write_Data(0x20);
    ST7701S_Write_Data(0x05);
    ST7701S_Write_Data(0x13);
    ST7701S_Write_Data(0x11);
    ST7701S_Write_Data(0xA4);
    ST7701S_Write_Data(0x29);
    ST7701S_Write_Data(0x18);
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x11);
    ST7701S_Write_Cmd(0xB0);
    ST7701S_Write_Data(0x6C);
    ST7701S_Write_Cmd(0xB1);
    ST7701S_Write_Data(0x43);
    ST7701S_Write_Cmd(0xB2);
    ST7701S_Write_Data(0x87);
    ST7701S_Write_Cmd(0xB3);
    ST7701S_Write_Data(0x80);
    ST7701S_Write_Cmd(0xB5);
    ST7701S_Write_Data(0x47);
    ST7701S_Write_Cmd(0xB7);
    ST7701S_Write_Data(0x85);
    ST7701S_Write_Cmd(0xB8);
    ST7701S_Write_Data(0x20);
    ST7701S_Write_Cmd(0xB9);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Cmd(0xC1);
    ST7701S_Write_Data(0x78);
    ST7701S_Write_Cmd(0xC2);
    ST7701S_Write_Data(0x78);
    ST7701S_Write_Cmd(0xD0);
    ST7701S_Write_Data(0x88);
    ST7701S_EndPacket();
    CTimerDelayXms(100);
    ST7701S_Write_Cmd(0xE0);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x02);
    ST7701S_Write_Cmd(0xE1);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x0A);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x07);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x09);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x33);
    ST7701S_Write_Data(0x33);
    ST7701S_Write_Cmd(0xE2);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Cmd(0xE3);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x33);
    ST7701S_Write_Data(0x33);
    ST7701S_Write_Cmd(0xE4);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Cmd(0xE5);
    ST7701S_Write_Data(0x0E);
    ST7701S_Write_Data(0x60);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Data(0x60);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0A);
    ST7701S_Write_Data(0x60);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0C);
    ST7701S_Write_Data(0x60);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Cmd(0xE6);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x33);
    ST7701S_Write_Data(0x33);
    ST7701S_Write_Cmd(0xE7);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Cmd(0xE8);
    ST7701S_Write_Data(0x0D);
    ST7701S_Write_Data(0x60);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0F);
    ST7701S_Write_Data(0x60);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x09);
    ST7701S_Write_Data(0x60);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0B);
    ST7701S_Write_Data(0x60);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Cmd(0xEB);
    ST7701S_Write_Data(0x02);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0xE4);
    ST7701S_Write_Data(0xE4);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x40);
    ST7701S_Write_Cmd(0xEC);
    ST7701S_Write_Data(0x02);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Cmd(0xED);
    ST7701S_Write_Data(0xAB);
    ST7701S_Write_Data(0x89);
    ST7701S_Write_Data(0x76);
    ST7701S_Write_Data(0x54);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Data(0x45);
    ST7701S_Write_Data(0x67);
    ST7701S_Write_Data(0x98);
    ST7701S_Write_Data(0xBA);
    ST7701S_Write_Cmd(0xEF);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x45);
    ST7701S_Write_Data(0x3F);
    ST7701S_Write_Data(0x54);
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x13);
    ST7701S_Write_Cmd(0xE8);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x0E);
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Cmd(0x11);
    ST7701S_EndPacket();
    CTimerDelayXms(120);
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x13);
    ST7701S_Write_Cmd(0xE8);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x0C);
    ST7701S_EndPacket();
    CTimerDelayXms(10);
    ST7701S_Write_Cmd(0xE8);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Cmd(0x29);
    ST7701S_Write_Cmd(0x3A);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Cmd(0x36);
    ST7701S_Write_Data(0x00);
    ST7701S_EndPacket();
    CTimerDelayXms(20);
    ST7701S_EndPacket();

}

#endif
