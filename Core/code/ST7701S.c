#include "Core\Header\Include.h"

#if(_PANEL_TYPE == _PNL_TM043_ST7701 || _PANEL_TYPE == _PNL_TM043_ST7701S)

#if(_PANEL_TYPE == _PNL_TM043_ST7701S)
// Hardware SPI Pins on Port 6 (SDA and CS SWAPPED for TM043YYSP21):
#define SPI_CS      MCU_PORT62_PIN_REG_FFC9  // Pin 52 (CS - swapped with Pin 54)
#define SPI_SCL     MCU_PORT63_PIN_REG_FFCA  // Pin 53 (SCL)
#define SPI_SDA     MCU_PORT64_PIN_REG_FFCB  // Pin 54 (SDA - swapped with Pin 52)
#define SPI_RST     MCU_PORT65_PIN_REG_FFCC  // Pin 55 (RST)
#else
// Hardware SPI Pins on Port 6 (Standard layout for EHD-5008):
#define SPI_SDA     MCU_PORT62_PIN_REG_FFC9  // Pin 52
#define SPI_SCL     MCU_PORT63_PIN_REG_FFCA  // Pin 53
#define SPI_CS      MCU_PORT64_PIN_REG_FFCB  // Pin 54
#define SPI_RST     MCU_PORT65_PIN_REG_FFCC  // Pin 55
#endif

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
    SPI_CS = 1;   // End previous packet
    ST_DelayCS(); // Gap between packets
    SPI_CS = 0;   // Assert CS for new command packet
    SPI_SDA = 0;  // 9th bit = 0 (Command)
    ST_ClockPulse();
    for (i = 0; i < 8; i++) {
        SPI_SDA = (cmd & 0x80) ? 1 : 0;
        ST_ClockPulse();
        cmd <<= 1;
    }
}

static void ST7701S_Write_Data(BYTE dat)
{
    BYTE i;
    SPI_SDA = 1;  // 9th bit = 1 (Data parameter)
    ST_ClockPulse();
    for (i = 0; i < 8; i++) {
        SPI_SDA = (dat & 0x80) ? 1 : 0;
        ST_ClockPulse();
        dat <<= 1;
    }
}

static void ST7701S_EndPacket(void)
{
    _nop_();
    SPI_CS = 1;
    _nop_();
}

void ST7701S_Init(void)
{
#if(_PANEL_TYPE == _PNL_TM043_ST7701S)
    // Tianma 4.3" 480x800 ST7701S (TM043YYSP21) Init Sequence
    // Power-up and Reset Sequence
    SPI_CS = 1;
    SPI_SCL = 1;
    SPI_SDA = 1;
    SPI_RST = 0;
    CTimerDelayXms(30);          // 30ms VDD stabilization

    SPI_RST = 1;
    CTimerDelayXms(10);

    SPI_RST = 0;
    CTimerDelayXms(20);          // Reset pulse LOW 20ms

    SPI_RST = 1;
    CTimerDelayXms(140);         // 140ms stabilization

    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x13);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xEF);
    ST7701S_Write_Data(0x08);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x10);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xC0);
    ST7701S_Write_Data(0x63);
    ST7701S_Write_Data(0x00);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xC1);
    ST7701S_Write_Data(0x13);
    ST7701S_Write_Data(0x02);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xC2);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x07);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xCC);
    ST7701S_Write_Data(0x10);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB0);
    ST7701S_Write_Data(0xC0);
    ST7701S_Write_Data(0x11);
    ST7701S_Write_Data(0x58);
    ST7701S_Write_Data(0x0D);
    ST7701S_Write_Data(0x13);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x0F);
    ST7701S_Write_Data(0x0B);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x2C);
    ST7701S_Write_Data(0x07);
    ST7701S_Write_Data(0x15);
    ST7701S_Write_Data(0x13);
    ST7701S_Write_Data(0xAE);
    ST7701S_Write_Data(0xB1);
    ST7701S_Write_Data(0x56);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB1);
    ST7701S_Write_Data(0xC0);
    ST7701S_Write_Data(0x0E);
    ST7701S_Write_Data(0x58);
    ST7701S_Write_Data(0x0E);
    ST7701S_Write_Data(0x0F);
    ST7701S_Write_Data(0x05);
    ST7701S_Write_Data(0x0B);
    ST7701S_Write_Data(0x06);
    ST7701S_Write_Data(0x07);
    ST7701S_Write_Data(0x28);
    ST7701S_Write_Data(0x05);
    ST7701S_Write_Data(0x14);
    ST7701S_Write_Data(0x12);
    ST7701S_Write_Data(0x2B);
    ST7701S_Write_Data(0xAE);
    ST7701S_Write_Data(0xD8);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x11);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB0);
    ST7701S_Write_Data(0x4D);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB1);
    ST7701S_Write_Data(0x5D);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB2);
    ST7701S_Write_Data(0x89);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB3);
    ST7701S_Write_Data(0x80);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB5);
    ST7701S_Write_Data(0x4D);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB7);
    ST7701S_Write_Data(0x85);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB8);
    ST7701S_Write_Data(0x22);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xB9);
    ST7701S_Write_Data(0x10);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xC1);
    ST7701S_Write_Data(0x78);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xC2);
    ST7701S_Write_Data(0x78);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xD0);
    ST7701S_Write_Data(0x88);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE0);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x02);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE1);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0A);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x09);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0B);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Data(0x44);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE2);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x40);
    ST7701S_Write_Data(0x40);
    ST7701S_Write_Data(0x03);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x04);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE3);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x33);
    ST7701S_Write_Data(0x33);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE4);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Data(0x44);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE5);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Data(0x42);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x12);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0C);
    ST7701S_Write_Data(0x3E);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0E);
    ST7701S_Write_Data(0x40);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE6);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x33);
    ST7701S_Write_Data(0x33);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE7);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Data(0x44);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xE8);
    ST7701S_Write_Data(0x0F);
    ST7701S_Write_Data(0x41);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x11);
    ST7701S_Write_Data(0x43);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0B);
    ST7701S_Write_Data(0x3D);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0x0D);
    ST7701S_Write_Data(0x3F);
    ST7701S_Write_Data(0xA0);
    ST7701S_Write_Data(0xA0);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xEB);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0xE4);
    ST7701S_Write_Data(0xE4);
    ST7701S_Write_Data(0x44);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xED);
    ST7701S_Write_Data(0xAB);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x76);
    ST7701S_Write_Data(0x54);
    ST7701S_Write_Data(0x2F);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xF2);
    ST7701S_Write_Data(0x45);
    ST7701S_Write_Data(0x67);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Data(0xFF);
    ST7701S_Write_Data(0xBA);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xEF);
    ST7701S_Write_Data(0x10);
    ST7701S_Write_Data(0x0D);
    ST7701S_Write_Data(0x04);
    ST7701S_Write_Data(0x08);
    ST7701S_Write_Data(0x3F);
    ST7701S_Write_Data(0x1F);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0xFF);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Data(0x01);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_Write_Data(0x00);
    ST7701S_EndPacket();
    ST7701S_Write_Cmd(0x11);
    ST7701S_EndPacket();
    CTimerDelayXms(120);
    ST7701S_Write_Cmd(0x29);
    CTimerDelayXms(20);
    ST7701S_Write_Cmd(0x3A);
    ST7701S_Write_Data(0x77);
    ST7701S_Write_Cmd(0x36);
    ST7701S_Write_Data(0x00);
    ST7701S_EndPacket();
#else
    // EHD-5008 5.0" 480x854 ST7701 Init Sequence
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
#endif
}

#endif // #if(_PANEL_TYPE == _PNL_TM043_ST7701 || _PANEL_TYPE == _PNL_TM043_ST7701S)
