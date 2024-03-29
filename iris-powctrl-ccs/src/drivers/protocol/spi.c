/*
 * spi.c
 *
 *  Created on: Jul. 9, 2021
 *      Author: asgaria/mckoyj
 */
#include "drivers/protocol/spi.h"

// Data buffers for ISRs
#define SPIA0_BYTE_LENGTH         4
#define SPIA1_BYTE_LENGTH         4
#define SPIB0_BYTE_LENGTH         2
#define SPIB1_BYTE_LENGTH         2
unsigned int SPIA0_index = 0;
uint8_t SPIA0_TXData[SPIA0_BYTE_LENGTH] = {0}, SPIA0_RXData[SPIA0_BYTE_LENGTH] = {0};
unsigned int SPIA1_index = 0;
uint8_t SPIA1_TXData[SPIA1_BYTE_LENGTH] = {0}, SPIA1_RXData[SPIA1_BYTE_LENGTH] = {0};
unsigned int SPIB0_index = 0;
uint8_t SPIB0_TXData[SPIB0_BYTE_LENGTH] = {0}, SPIB0_RXData[SPIB0_BYTE_LENGTH] = {0};
unsigned int SPIB1_index = 0;
uint8_t SPIB1_TXData[SPIB1_BYTE_LENGTH] = {0}, SPIB1_RXData[SPIB1_BYTE_LENGTH] = {0};

void Init_SPI_Ports()
{
    /*********************************************************
     *                  SPI A0 Interface Pins
     *********************************************************/
    //P1.4(SPI CLK on UCB0CLK) --> P5.0
    //GPIO_setAsOutputPin(GPIO_PORT_SPIA0_CLK_MISO_MOSI, GPIO_PIN_SPIA0_CLK);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_SPIA0_CLK_MISO_MOSI, GPIO_PIN_SPIA0_CLK, GPIO_PRIMARY_MODULE_FUNCTION);
    //P1.6(MOSI on UCB0SIMO) --> P5.2
    //GPIO_setAsOutputPin(GPIO_PORT_SPIA0_CLK_MISO_MOSI, GPIO_PIN_SPIA0_MOSI);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_SPIA0_CLK_MISO_MOSI, GPIO_PIN_SPIA0_MOSI, GPIO_PRIMARY_MODULE_FUNCTION );
    //P1.7(MISO on UCB0SOMI) --> P5.1
    //GPIO_setAsInputPin(GPIO_PORT_SPIA0_CLK_MISO_MOSI, GPIO_PIN_SPIA0_MISO);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_SPIA0_CLK_MISO_MOSI, GPIO_PIN_SPIA0_MISO, GPIO_PRIMARY_MODULE_FUNCTION);
    //set P2.5 as SPI CS, already set to output above --> P4.7
    //GPIO_setAsOutputPin(GPIO_PORT_SPIA0_CS, GPIO_PIN_SPIA0_CS);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_SPIA0_CS, GPIO_PIN_SPIA0_CS, GPIO_PRIMARY_MODULE_FUNCTION);

    //GPIO_setOutputLowOnPin(GPIO_PORT_SPIA0_CS, GPIO_PIN_SPIA0_CS);
    //GPIO_setOutputHighOnPin(GPIO_PORT_SPIA0_CS, GPIO_PIN_SPIA0_CS);
    GPIO_setOutputHighOnPin(GPIO_PORT_SPIA0_CLK_MISO_MOSI, GPIO_PIN_SPIA0_CLK);
    GPIO_setOutputHighOnPin(GPIO_PORT_SPIA0_CLK_MISO_MOSI, GPIO_PIN_SPIA0_MOSI);
    GPIO_setOutputHighOnPin(GPIO_PORT_SPIA0_CS, GPIO_PIN_SPIA0_CS);
}

void Init_SPI()
{
    //Init_SPI_Ports();
    Init_SPIA0();
    //Init_SPIA1();
    //Init_SPIB0();
    //Init_SPIB1();

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();
}

/*
 * SPI A0 communicates with the CAN transceiver.
 * Initialization not working with driverlib functions.
 */
void Init_SPIA0()
{

    SYSCFG3 |= USCIA0RMP;             // SETS remap source
    //-- setup SPIA0
    UCA0CTLW0 |= UCSWRST;           // put into SW reset
    UCA0CTLW0 |= UCSSEL__SMCLK;     // choose SMCLK
    UCA0BRW = 10;                   // set prscaler to 10 to get 100KHz
    //UCA0BRW = 8;                   // set prscaler to 10 to get 100KHz
    //UCA0BRW = 6;                   // set prscaler to 10 to get 100KHz
    UCA0CTLW0 |= UCSYNC | UCMST | UCMSB | UCMODE_2 | UCSTEM;
    UCA0CTLW0 |= UCSYNC;            // put into SPI
    UCA0CTLW0 |= UCMST;             // set as master
    UCA0CTLW0 |= UCMSB;
    UCA0CTLW0 |= UCMODE_2;
    UCA0CTLW0 |= UCSTEM;
    //UCA0CTLW0 |= UCCKPL;            // Clock inactive HIGH
    //configure the ports
    P5SEL1 &= ~(BIT2 | BIT1 | BIT0);
    P5SEL0 |= (BIT2 | BIT1 | BIT0);
    /*
    P5SEL1 &= ~BIT1;               // P5.1 use SOMI
    P5SEL0 |= BIT1;
    P5SEL1 &= ~BIT0;                // P5.0 use SCLK
    P5SEL0 |= BIT0;
    P5SEL1 &= ~BIT2;               // P5.2 use SIMO
    P5SEL0 |= BIT2;
    */
    //P4SEL1 &= ~BIT7;               // P4.7 use STE
    //P4SEL0 |= BIT7;
    //PM5CTL0 &= ~LOCKLPM5;
    UCA0CTLW0 &= ~UCSWRST;        // take out of SW reset
    // -- ENABLE IRQs
    //P4IE |= BIT1;
    //P4F
    UCA0IE |= UCTXIE;
    UCA0IFG &= ~UCTXIFG;         //clear flag

    // Send dummy message to pull CLK low
    uint8_t dummy_data[4] = {0};
    GPIO_setOutputLowOnPin(GPIO_PORT_SPIA0_CS, GPIO_PIN_SPIA0_CS);
    read_write_SPIA0(&dummy_data);
    GPIO_setOutputHighOnPin(GPIO_PORT_SPIA0_CS, GPIO_PIN_SPIA0_CS);

}
/*
 * SPI A1 communicates with the ADCS subsystem.
 */
void Init_SPIA1()
{
    // for ADCS communication
    // PxSEL0 and PxSEL1 define port function as follows

    // PxSEL1   PxSEL0      FUNCTION
    //_________________________________
    // 0        0           GPIO
    // 0        1           PRIMARY*
    // 1        0           SECONDARY*
    // 1        1           TERTIARY*
    //_________________________________
    // * refer to datasheet to see the functions of each pin

    //P2SEL0 &= 0xFF ^(BIT5 | BIT6);             // set 4-SPI pin as second function
    P2SEL0 |= (BIT4 | BIT5 | BIT6);
    //P3SEL0 |= BIT1;

    UCA1CTLW0 |= UCSWRST;                     // **Put state machine in reset**
                                              // 4-pin, 8-bit SPI master
    // UCMST    sets 1 stop bit
    // UCSYNS   synchronous mode
    // UCCKPL   parity select = even
    // UCMSB    MSB first mode
    // UCMODE_1 idle-line multiprocessor mode
    // set to UCMODE_2 if you need CS=0 when active
    // UCSTEM   next frame transmitted is a break or a break/synch
    UCA1CTLW0 |= UCMST|UCSYNC|UCCKPL|UCMSB|UCMODE_2|UCSTEM;
    // UCSSEL__ACLK sets clock source to device specific
    //UCA1CTLW0 |= UCSSEL__ACLK;                // Select ACLK
    UCA1CTLW0 |= UCSSEL__SMCLK;
    // UCAxBRW Baud rate word
    // baud rate control clock
    // BR = ACLK/2
    UCA1BR0 = 0x02;                           // BRCLK = ACLK/2
    UCA1BR1 = 0;                              //
    // UCAxMCTLW modulation control word
    UCA1MCTLW = 0;                            // No modulation
    // over sampling mode disabled
    UCA1CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
}
/*
 * SPI B0 communicates with ADC bank A
 */
void Init_SPIB0()
{
    //P2SEL0 &= 0xFF ^(BIT5 | BIT6);             // set 4-SPI pin as second function
    P1SEL0 |= (BIT1 | BIT2 | BIT3);
    //P1SEL0 |= BIT0;   //STE pin will be controlled manually
    // Control word 0
    UCB0CTLW0 |= UCSWRST;                     // **Put state machine in reset**
                                              // 4-pin, 8-bit SPI master

    UCB0CTLW0 |= UCMST|UCSYNC|UCCKPL|UCMSB|UCMODE_2|UCSTEM;
    // Clock source
    // UCSSEL__ACLK sets clock source to device specific
    UCB0CTLW0 |= UCSSEL__ACLK;                // Select ACLK
//    UCB0CTLW0 |= UCSSEL__SMCLK;
    // Clock divide
    UCB0BR0 = 0x02;                           // BRCLK = ACLK/2
    UCB0BR1 = 0;                              //
    // UCBxMCTLW modulation control word
    //UCB0MCTLW = 0;                            // No modulation
    // over sampling mode disabled
    UCB0CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**

    UCB0IE |= UCRXIE;                         // Enable USCI_B0 RX interrupt

    //SYSCFG3|=USCIB1RMP;  //Set the remapping source
        // It works without it and I don't know why!

}
/*
 * SPI B1 communicates with ADC Bank B
 */
void Init_SPIB1()
{
    P4SEL0 |= (BIT3 | BIT4);
    P5SEL0 |= (BIT3);
    //P5SEL0 |= BIT4;   //STE pin will be controlled manually
    // Control word 0
    UCB1CTLW0 |= UCSWRST;                       // **Put state machine in reset**
                                                // 4-pin, 8-bit SPI master
    UCB1CTLW0 |= UCMST|UCSYNC|UCCKPL|UCMSB|UCMODE_2|UCSTEM;
    // UCSSEL__ACLK sets clock source to device specific
    UCB1CTLW0 |= UCSSEL__ACLK;                // Select ACLK
    //UCB1CTLW0 |= UCSSEL__SMCLK;
    // Clock divider
    UCB1BR0 = 0x02;                           // BRCLK = ACLK/2
    UCB1BR1 = 0;                              //
    // UCBxMCTLW modulation control word
    //UCB0MCTLW = 0;                            // No modulation
    // over sampling mode disabled
    UCB1CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**

    UCB1IE |= UCRXIE;                         // Enable USCI_B0 RX interrupt
}

uint32_t read_write_SPIA0(uint8_t data[4])
{
    //memcpy(&SPIA0_TXData, &data, SPIA0_BYTE_LENGTH);
    SPIA0_TXData[0] = data[0];
    SPIA0_TXData[1] = data[1];
    SPIA0_TXData[2] = data[2];
    SPIA0_TXData[3] = data[3];
    UCA0IFG |= UCTXIFG;
    return (((uint32_t)SPIA0_RXData[0]) << 24) | (((uint32_t)SPIA0_RXData[1] << 16)) | (((uint32_t)SPIA0_RXData[2]) << 8) | SPIA0_RXData[3];
}
uint32_t read_write_SPIA1(uint8_t data[])
{
    memcpy(SPIA1_TXData, data, SPIA1_BYTE_LENGTH);
    UCA1IFG |= UCTXIFG;
    return (((uint32_t)SPIA1_RXData[0]) << 24) | (((uint32_t)SPIA1_RXData[1] << 16)) | (((uint32_t)SPIA1_RXData[2]) << 8) | SPIA1_RXData[3];
}
uint16_t read_write_SPIB0(uint8_t data[])
{
    memcpy(SPIB0_TXData, data, SPIB0_BYTE_LENGTH);
    UCB0IFG |= UCTXIFG;
    return (((uint16_t)SPIB0_RXData[0]) << 8) | SPIB0_RXData[1];
}
uint16_t read_write_SPIB1(uint8_t data[])
{
    memcpy(SPIB1_TXData, data, SPIB1_BYTE_LENGTH);
    UCB1IFG |= UCTXIFG;
    return (((uint16_t)SPIB1_RXData[0]) << 8) | SPIB1_RXData[1];
}
// -------------------- ISRs --------------------
#pragma vector = EUSCI_A0_VECTOR
__interrupt void ISR_EUSCI_A0(void)
{
    if (SPIA0_index < SPIA0_BYTE_LENGTH)
    {
        UCA0TXBUF = SPIA0_TXData[SPIA0_index];
        SPIA0_RXData[SPIA0_index++] = UCA0RXBUF;
    }
    else
    {
        SPIA0_index = 0;
        UCA0IFG &= ~UCTXIFG;
    }

    /*switch(__even_in_range(UCA0IV,USCI_SPI_UCTXIFG))
    {
        case USCI_NONE: break;                    // Vector 0 - no interrupt
        case USCI_SPI_UCRXIFG:
            if (SPIA0_index < SPIA0_BYTE_LENGTH)
            {
                SPIA0_RXData[SPIA0_index] = UCA0RXBUF;
                UCA0IFG |= UCTXIFG;
            }
            else
            {
                //SPIA0_TXData = 0;
                UCA0IFG &= ~UCRXIFG;
            }
            break;
        case USCI_SPI_UCTXIFG:
            if (SPIA0_index < SPIA0_BYTE_LENGTH)
            {
                UCA0TXBUF = SPIA0_TXData[SPIA0_index++];
                UCA0IFG |= UCRXIFG;
                UCA0IFG &= ~UCTXIFG;
            }
            else
            {
                SPIA0_index = 0;
                UCA0IFG &= ~UCTXIFG;
            }
            break;
        default: break;
    }*/

}
/*
#pragma vector = EUSCI_A1_VECTOR
__interrupt void ISR_EUSCI_A1(void)
{

}
#pragma vector = EUSCI_B0_VECTOR
__interrupt void ISR_EUSCI_B0(void)
{

}
#pragma vector = EUSCI_B1_VECTOR
__interrupt void ISR_EUSCI_B1(void)
{

}
*/
