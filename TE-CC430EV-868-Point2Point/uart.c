/**************************************************************************************************
  Filename:       uart.c
  Revised:        $Date: 2009-08-17 10:50:58 -0700 (Mon, 17 Aug 2009) $
  Author:         $Author: jnoxon $

  Description:    This file supports the SimpliciTI-compatible UART driver.

  Copyright 2004-2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights granted under
  the terms of a software license agreement between the user who downloaded the software,
  his/her employer (which must be your employer) and Texas Instruments Incorporated (the
  "License"). You may not use this Software unless you agree to abide by the terms of the
  License. The License limits your use, and you acknowledge, that the Software may not be
  modified, copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio frequency
  transceiver, which is integrated into your product. Other than for the foregoing purpose,
  you may not use, reproduce, copy, prepare derivative works of, modify, distribute,
  perform, display or sell this Software and/or its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS”
  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY
  WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
  IN NO EVENT SHALL TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE
  THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY
  INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST
  DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY
  THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/******************************************************************************
 * INCLUDES
 */
#include "bsp.h"
#include "uart.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "app_remap_led.h"


/******************************************************************************
 * MACROS
 */

/******************************************************************************
 * CONSTANTS AND DEFINES
 */

/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */
uart_get_tx_data_type uart_tx_handler = NULL;
uart_put_rx_data_type uart_rx_handler = NULL;
bool uart_tx_suspend = false;
extern unsigned long PAUSE_COUNTER;

/******************************************************************************
 * LOCAL FUNCTIONS
 */
static void uart_tx_irq( void );
static void uart_rx_irq( void );


#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR( void )
{
	  if( ( UCA0IV & ( 1 << UCRXIE ) ) != 0 )
	    uart_rx_irq( ); // handle the receive request
	  else
	    uart_tx_irq( ); // handle the transmit request

}
/*#pragma vector=USCI_A0_VECTOR
__interrupt void uart_rx_tx_enter_irq( void )
{
  if( ( UCA0IV & ( 1 << UCRXIE ) ) != 0 )
    uart_rx_irq( ); // handle the receive request
  else
    uart_tx_irq( ); // handle the transmit request
}*/






// the isr's...
static void uart_tx_irq( void )
  {    
  unsigned char c;
    {
    uart_get_tx_data_type handler;
    BSP_CRITICAL_STATEMENT( handler = uart_tx_handler );

    // if not currently in suspend mode and a handler exists
    if( uart_tx_suspend == false && handler != NULL )
      {
      if( (*handler)( &c ) != false ) // if data available, reset the interrupt
        {
        if(c==0x00){c=0x20;}
    	  UART_SEND( UART_NUMBER, UART_LOCATION, c ); // send the byte
        }
      else // if no data suspend transmission
        {
        uart_tx_message_suspend( handler );
        UART_IRQ_FLAG_SET( UART_NUMBER, UART_LOCATION, TX );
        }
      }
    else
      {
      bspIState_t istate;
      BSP_ENTER_CRITICAL_SECTION( istate );
      // if we are in suspended mode or we just sent an xon or xoff character
      // while transmits were disabled or the message has been completely sent,
      // then simply disable irq's so we don't get stuck in an infinite loop
      UART_IRQ_DISABLE( UART_NUMBER, UART_LOCATION, TX );
      UART_IRQ_FLAG_SET( UART_NUMBER, UART_LOCATION, TX );
      BSP_EXIT_CRITICAL_SECTION( istate );
      }
    }

        P3OUT &= ~(1 << 6);
  return;
}

/******************************************************************************
 * @fn          uart_rx_irq
 *
 * @brief       RX interrupt service routine
 *
 * input parameters
 *
 * output parameters
 *
 * @return   
 */

unsigned char uart_rx_bufer[RX_TX_BUFFER_SIZE];
int uart_rx_bufer_pointer = 0;

void uart_rx_irq( void )
  {
	 PAUSE_COUNTER = 0;
	 if(uart_rx_bufer_pointer != RX_TX_BUFFER_SIZE)
	 {
		 uart_rx_bufer[uart_rx_bufer_pointer] = UART_RECEIVE( UART_NUMBER, UART_LOCATION );
		 uart_rx_bufer_pointer++;//
	 }

  }

/******************************************************************************
 * GLOBAL FUNCTIONS
 */ 

/******************************************************************************
 * @fn          uart_init
 *
 * @brief       Configures UART and sets up transmit and receive interrupts
 *
 * input parameters
 *
 * output parameters
 *
 * @return   
 */

/*
void uart_init( void )
	{
	  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	  PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
	  P1MAP5 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.5
	  P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.6
	  PMAPPWD = 0;                              // Lock port mapping registers

	  P1DIR |= BIT6;                            // Set P1.6 as TX output
	  P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function

	  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	  UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
	  UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
	  UCA0BR1 = 0x00;                           //
	  UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
	  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

//!	  __bis_SR_register(LPM3_bits + GIE);       // Enter LPM3, interrupts enabled
//!	  __no_operation();                         // For debugger
	}
*/


void uart_init( void )
  {
  //! ===================================================================
	/*
  volatile unsigned int i;

  // make sure the handler functions are cleared in case we are re-initialized
  uart_tx_handler = NULL;
  uart_rx_handler = NULL;

  // initialize the uart interface for operations
  UART_INIT( UART_NUMBER,
             UART_LOCATION,
             UART_FLOW_CONTROL,    // enable/disable flow control
             UART_PARITY_MODE,     // enable/disable parity
             UART_STOP_BITS,       // number of stop bits
             UART_BAUD_RATE );     // baud rate to use
   
  i = UART_BAUD_RATE >> 5; // delay approximately 1 bit time
  while( --i != 0 ) // give the uart some time to initialize
      ; // null statement

   // set the interrupt flag so that a transmit interrupt will be pending
   // that way when a message is sent and the irq is enabled, the interrupt
   // will happen immediately to start the transmission
  UART_IRQ_FLAG_SET( UART_NUMBER, UART_LOCATION, TX ); // set the interrupt
  
  // enable receive interrupts, they are always welcome.
  UART_IRQ_ENABLE( UART_NUMBER, UART_LOCATION, RX ); 

  return;
  */
	//! ===================================================================




  unsigned int i;
   bspIState_t istate;
   BSP_ENTER_CRITICAL_SECTION( istate );

   // disable the receive and transmit interrupts for the moment
   UART_IRQ_DISABLE( UART_NUMBER, UART_LOCATION, TX );
   UART_IRQ_DISABLE( UART_NUMBER, UART_LOCATION, RX );

   BSP_EXIT_CRITICAL_SECTION( istate );

   // make sure the handler functions are cleared in case we are re-initialized
   uart_tx_handler = NULL;
   uart_rx_handler = NULL;

   // clear transmit suspend semaphore
   uart_tx_suspend = false;

   //!=========================================================================================
   /*
   PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
   P1MAP5 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.5
   P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.6
   PMAPPWD = 0;                              // Lock port mapping registers

   P1DIR |= BIT6;                            // Set P1.6 as TX output
   P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function

   UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
   UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
   UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
   UCA0BR1 = 0x00;                           //
   UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
   UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
   UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
*/
   //!=========================================================================================


   // initialize the uart interface for operations
   UART_INIT_CC430( UART_NUMBER,
              UART_LOCATION,
              UART_FLOW_CONTROL,    // enable/disable flow control
              UART_PARITY_MODE,     // enable/disable parity
              UART_STOP_BITS,       // number of stop bits
              UART_BAUD_RATE );     // baud rate to use*/

   i = UART_BAUD_RATE >> 5; // delay approximately 1 bit time
   while( --i != 0 ) // give the uart some time to initialize
       ; // null statement

   // enable receive interrupts, they are always welcome.
   UART_IRQ_ENABLE( UART_NUMBER, UART_LOCATION, RX );

   return;

  }


/******************************************************************************
 * @fn          uart_tx_message
 *
 * @brief       Installs transmit handler if no message currently being sent
 *
 * input parameters
 * @param   handler - UART transmit handler
 *
 * @return   Status of the operation.
 *           true                 Transmit handler successfully installed
 *           false                Message being sent or handler is invalid
 *                                
 */
bool uart_tx_message( uart_get_tx_data_type handler )
  {
  bspIState_t  intState;
  bool status = false; /* assume failure initially */

  /* updates required, store interrupt state and disable interrupts */
  BSP_ENTER_CRITICAL_SECTION(intState);

  /* if no message is currently being sent and handler looks valid */
  if( uart_tx_handler == NULL && handler != NULL )
    {
    uart_tx_handler = handler; /* install the handler */

    /* once the handler has been setup, enable the interrupt.
     * this will cause the message to begin transmission */
    UART_IRQ_ENABLE( UART_NUMBER, UART_LOCATION, TX ); 

    status = true; /* indicate success */    
    }

  BSP_EXIT_CRITICAL_SECTION(intState); /* restore interrupt state */
    
  return status; /* indicate status */
  }
  
bool uart_tx_message_end( uart_get_tx_data_type handler )
  {
  bspIState_t intState;
  bool status = false; // assume failure initially

  BSP_ENTER_CRITICAL_SECTION( intState );

  if( uart_tx_handler == handler )
    {
    // no more data to send, reset the handler to flag not busy
    uart_tx_handler = NULL;
    uart_tx_suspend = false;
    status = true; // indicate success
    }
    
  BSP_EXIT_CRITICAL_SECTION( intState );
  
  return status; // indicate status
  }

bool uart_tx_message_suspend( uart_get_tx_data_type handler )
  {
  bool status = false; // assume failure initially
  bspIState_t intState;
  BSP_ENTER_CRITICAL_SECTION( intState );
  
  if( uart_tx_handler == handler )
    {
    uart_tx_suspend = true; // indicate we are in suspended status
    status = true; // indicate success
    }
  
  BSP_EXIT_CRITICAL_SECTION( intState );
  return status;
  }
bool uart_tx_message_resume( uart_get_tx_data_type handler )
  {
  bool status = false; // assume failure initially
  bspIState_t intState;
  BSP_ENTER_CRITICAL_SECTION( intState );
  
  if( uart_tx_handler == handler )
    {
    uart_tx_suspend = false; // indicate we are no longer suspended
    UART_IRQ_ENABLE( UART_NUMBER, UART_LOCATION, TX ); // enable interrupt
    status = true; // indicate success
    }
  
  BSP_EXIT_CRITICAL_SECTION( intState );
  return status;
  }

/******************************************************************************
 * @fn          uart_rx_message
 *
 * @brief       Installs receive handler if no message currently being received
 *
 * input parameters
 * @param   handler - UART receive handler
 *
 * @return   Status of the operation.
 *           true                 Receive handler successfully installed
 *           false                Message being received or handler is invalid
 *                                
 */
bool uart_rx_message( uart_put_rx_data_type handler )
  {
  bspIState_t intState;
  bool status = false;  /* assume failure initially */
  
  /* updates required, store interrupt state and disable interrupts */
  BSP_ENTER_CRITICAL_SECTION(intState);

  /* if no message is being received and the handler looks valid */
  if( uart_rx_handler == NULL && handler != NULL )
    {
    uart_rx_handler = handler; /* install the handler */

    status = true; /* indicate success */
    }
  
  BSP_EXIT_CRITICAL_SECTION(intState); /* restore interrupt state */
    
  return status; /* indicate status */
  }

bool uart_rx_message_end( uart_put_rx_data_type handler )
  {
  bspIState_t intState;
  bool status = false; // assume failure initially
    
  BSP_ENTER_CRITICAL_SECTION(intState);
    
  // if it appears that the current receiver client is terminating the message
  if( uart_rx_handler == handler )
    {
    // clear the handler to indicate the driver is free for acquisition
    uart_rx_handler = NULL;

    status = true;
    }
   
  BSP_EXIT_CRITICAL_SECTION(intState);

  return status; // return result
  }

bool uart_rx_message_suspend( uart_put_rx_data_type handler )
  {
  return false; // cannot suspend yet
  }

bool uart_rx_message_resume( uart_put_rx_data_type handler )
  {
  return true; // always receiving for now
  }


