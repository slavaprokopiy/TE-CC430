/*
 * radio.c
 *
 *  Created on: 18.09.2013
 *      Author: prokopiy
 */

#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "app_remap_led.h"
#include "uart.h"
#include "uart_intfc.h"

extern uart_get_tx_data_type rf_tx_handler;
extern bool rf_tx_suspend;
int RF_IRQ_ENABLED = true;
int RF_MESSAGE_SENT = true;
int RADIO_INT_ENABLE = true;
int  RF_ACK_FLAG = true;

unsigned char  msg[2];

linkID_t sLinkID1;
linkID_t sLinkID2;

void radio_processing( void )
{
	unsigned char c;
	if(RF_ACK_FLAG)	// ≈сли получено подтверждение от приемника
  	  {
	  uart_get_tx_data_type handler;
	  BSP_CRITICAL_STATEMENT( handler = rf_tx_handler );

	  // if not currently in suspend mode and a handler exists
	  if( rf_tx_suspend == false && handler != NULL )
	  {
		  if( (*handler)( &c ) != false ) // if data available, reset the interrupt
		  {
			  msg[0] = SYMBOL_IDENT;
			  msg[1] = c;
#ifdef LINK_TO
			  SMPL_Send(sLinkID1, msg, sizeof(msg));
#endif

#ifdef LINK_LISTEN
			  SMPL_Send(sLinkID2, msg, sizeof(msg));
#endif
			  RF_ACK_FLAG = false;
		  }
		  else // if no data suspend transmission
		  {
			  RF_MESSAGE_SENT = true;
			  rf_tx_message_suspend( handler );
			  RF_ACK_FLAG = true;//!UART_IRQ_FLAG_SET( UART_NUMBER, UART_LOCATION, TX );
		  }
      }
	  else
	  {
		  //!bspIState_t istate;
		  //!BSP_ENTER_CRITICAL_SECTION( istate );
		  // if we are in suspended mode or we just sent an xon or xoff character
		  // while transmits were disabled or the message has been completely sent,
		  // then simply disable irq's so we don't get stuck in an infinite loop
		  RF_IRQ_ENABLED = false;//!UART_IRQ_DISABLE( UART_NUMBER, UART_LOCATION, TX );
		  //!UART_IRQ_FLAG_SET( UART_NUMBER, UART_LOCATION, TX );
		  //!BSP_EXIT_CRITICAL_SECTION( istate );
		  ;
      	  }
  	  }
 }


uart_get_tx_data_type rf_tx_handler = NULL;
bool rf_tx_suspend = false;
extern int RF_IRQ_ENABLED;
unsigned long PAUSE_COUNTER = 0;
unsigned char rf_rx_bufer[RX_TX_BUFFER_SIZE];
int rf_rx_bufer_pointer = 0;

bool rf_tx_message_resume( uart_get_tx_data_type handler )
  {
	bool status = false; // assume failure initially
	bspIState_t intState;
	BSP_ENTER_CRITICAL_SECTION( intState );

	if( rf_tx_handler == handler )
    {
		rf_tx_suspend = false; // indicate we are no longer suspended
		radio_processing();//UART_IRQ_ENABLE( UART_NUMBER, UART_LOCATION, TX ); // enable interrupt
		status = true; // indicate success
	}

  BSP_EXIT_CRITICAL_SECTION( intState );
  return status;
  }

bool rf_tx_message( uart_get_tx_data_type handler )
  {
  bspIState_t  intState;
  bool status = false; /* assume failure initially */

  /* updates required, store interrupt state and disable interrupts */
  BSP_ENTER_CRITICAL_SECTION(intState);

  /* if no message is currently being sent and handler looks valid */
  if( rf_tx_handler == NULL && handler != NULL )
    {
    rf_tx_handler = handler; /* install the handler */

    /* once the handler has been setup, enable the interrupt.
     * this will cause the message to begin transmission */
    RF_IRQ_ENABLED = true;//UART_IRQ_ENABLE( UART_NUMBER, UART_LOCATION, TX );

    status = true; /* indicate success */
    }
  }
bool rf_tx_message_suspend( uart_get_tx_data_type handler )
  {
  bool status = false; // assume failure initially
  bspIState_t intState;
  BSP_ENTER_CRITICAL_SECTION( intState );

  if( rf_tx_handler == handler )
    {
    rf_tx_suspend = true; // indicate we are in suspended status
    status = true; // indicate success
    }

  BSP_EXIT_CRITICAL_SECTION( intState );
  return status;
  }

