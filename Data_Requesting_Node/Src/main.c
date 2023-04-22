///**
// ******************************************************************************
// * @file           : main.c
// * @author         : Nitin Flavier & Johny John
// * @brief          : Main program body
// ******************************************************************************
// * @attention      : Bit Rate = 125 kbits/sec
// *
// * Copyright (c) 2023 STMicroelectronics.
// * All rights reserved.
// *
/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Nitin Flavier & Johny John
 * @brief          : Main program body
 ******************************************************************************
 * @attention      : Bit Rate = 125 kbits/sec
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */



#include <stdint.h>
#include <stm32f4xx.h>

uint8_t count = 1;

void Set_Pin(void)
{
    RCC->AHB1ENR |= (1 << 1)|(1 << 0);               // GPIOA, GPIOB enabled
    GPIOA->MODER &= ~(3<<0);              // Input Mode
    GPIOB->MODER |= (0xA << 16);          // Alternate functionality mode

    // Setting AFR_H for port B pin 8,9 we are making it as CAN_Rx(8) & CAN_Tx(9)
    GPIOB->AFR[1] |= (0x9 | (0x9 << 4));
}

void CAN_Init(void)
{
    RCC->APB1ENR |= (1<<25);     // Enable CAN 1 clock
 /*
    The software requests bxCAN to enter initialization or Sleep mode
    by setting the INRQ or SLEEP bits in the CAN_MCR register. Once the mode has been
    entered, bxCAN confirms it by setting the INAK or SLAK bits in the CAN_MSR register
 */


    CAN1->MCR |= (1<<0);       // CAN INRQ requesting it to enter into initialization
    while(!(CAN1->MSR & 0x1));   // Mode has been entered it is acknowledged

    //CAN1->BTR |= (1<<30);      // CAN LoopBack Mode here

    CAN1->BTR &= ~(3<<24);     // SWJ 1 Time Quantum

    CAN1->BTR &= ~(0x7F << 16);  // clearing

    CAN1->BTR |= (12 << 16);   // Time Segment 1
    CAN1->BTR |= (1 << 20);     // Time Segment 2

    CAN1->BTR |= (7 << 0);      // Baud-Rate Prescaler

    /*
       Exit Initialization mode
       Wait until INAK bit is cleared by hardware
    */
    CAN1->MCR &= ~(1<<0);
    while(CAN1->MSR & 0x1);

    /*
       Exit Sleep mode
       Wait until INAK bit is
       cleared by hardware
    */
    CAN1->MCR &= ~(1 << 1);
    while(CAN1->MSR & 0x2);

    /* Setting Up Transmission */

    CAN1->sTxMailBox[0].TIR = 0;

    CAN1->sTxMailBox[0].TIR |= (138 << 21);    // STD ID : 138
    CAN1->sTxMailBox[0].TIR |= (1 << 1);       // RTR bit

    CAN1->sTxMailBox[0].TDHR = 0;     // Data byte 4, 5, 6, 7
    CAN1->sTxMailBox[0].TDTR = 0;    // Sending only 1 byte

}


void CAN_Send_Request(void)
{
	CAN1->sTxMailBox[0].TDTR |= (8 << 0);  // requesting 8 bytes of data;

	/* Request for transmission */
	CAN1->sTxMailBox[0].TIR |= 1;
}

void delay(uint32_t l)
{
    for(uint32_t i=0;i<1000*l;i++);
}

int main(void)
{
    Set_Pin();             // Configure GPIOB 8 as CAN-Tx & GPIOB 9 as CAN-Rx
    CAN_Init();

    while(1)
    {
    	if(GPIOA->IDR & (1<<0))
    	{
    		CAN_Send_Request();
    	}

        delay(300);
        count++;
        if(count > 100)
        	count = 1;
    }
}
