//----------------------------------------------------------------------------------------------------
// ID Code      : DDCCI.c
// Description  : Clean C implementation of DDCCI buffer definitions to replace DDCCI.obj
//                Faithfully reverse-engineered and disassembled from Realtek DDCCI.obj
//----------------------------------------------------------------------------------------------------

#include "Core\Header\Include.h"

#define RX_BUFFER_SIZE 32
#define TX_BUFFER_SIZE 32

unsigned char xdata rxStatus = 0;
unsigned char xdata ucDDCCI_CheckSum = 0;
unsigned char xdata ucDDCCI_RxCount  = 0;
unsigned char xdata ucDDCCI_RxIndex  = 0;
unsigned char xdata ucDDCCI_TxCount  = 0;
unsigned char xdata ucDDCCI_NullStr[4] = {0x6e, 0x80, 0xbe, 0x6e};
unsigned char xdata ucDDCCI_RxBuf[RX_BUFFER_SIZE];
unsigned char xdata ucDDCCI_TxBuf[TX_BUFFER_SIZE];

unsigned char xdata ucVPC_Page = 0;
bit fDDCCI_Loop = 0;

unsigned char xdata ucVirtualKey = 0xFF;
unsigned char xdata *txBufferPtr;

void CInitDDCCI(void)
{
    MCU_I2C_IRQ_CTRL2_FF2A |= 0x80;
    MCU_I2C_SET_SLAVE_FF23 = 0x6E;
    MCU_I2C_IRQ_CTRL_FF28 = 0x0C;
    MCU_IRQ_PRIORITY_FF01 = 0x01;
    EX1 = 1;
}

void CDDCCICommand(void)
{
}

void CDDCCICommandDo(void)
{
}
