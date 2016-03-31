/*----------------------------------------------------------------------------
 *  Demo Application for SimpliciTI
 *
 *  L. Friedman
 *  Texas Instruments, Inc.
 *---------------------------------------------------------------------------- */

/**********************************************************************************************
  Copyright 2007-2009 Texas Instruments Incorporated. All rights reserved.

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

#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "uart.h"
#include "uart_intfc.h"

#include "app_remap_led.h"

#ifndef APP_AUTO_ACK
#error ERROR: Must define the macro APP_AUTO_ACK for this application.
#endif

extern uart_get_tx_data_type rf_handler;
extern bool rf_tx_suspend;
extern unsigned long PAUSE_COUNTER;
extern unsigned char uart_rx_bufer[];
extern int uart_rx_bufer_pointer;
extern int RF_MESSAGE_SENT;
extern int RADIO_INT_ENABLE;
extern int RF_IRQ_FLAG;



void toggleLED(uint8_t);

static void linkTo(void);

static uint8_t  sTid = 0;
static linkID_t sLinkID1 = 2;
unsigned char MSG_ID	= 0;

#define SPIN_ABOUT_A_SECOND   NWK_DELAY(1000)
#define SPIN_ABOUT_A_QUARTER_SECOND   NWK_DELAY(250)

/* How many times to try a Tx and miss an acknowledge before doing a scan */
#define MISSES_IN_A_ROW  2

unsigned int smpl_send_join_replyFlag, MrfiSyncPinRxIsrFlag, nwk_processJoinFlag, handleJoinRequestFlag, dispatchFrameFlag, nwk_sendFrameFlag;

void main (void)
{
  BSP_Init();
  uart_intfc_init();

  /* If an on-the-fly device address is generated it must be done before the
   * call to SMPL_Init(). If the address is set here the ROM value will not
   * be used. If SMPL_Init() runs before this IOCTL is used the IOCTL call
   * will not take effect. One shot only. The IOCTL call below is conformal.
   */
#ifdef I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE
  {
	    const addr_t lAddr = {0x00, 0x00, 0x01, 0x04};
	    MSG_ID = lAddr.addr[3];
	    // set device address
	    SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, (addr_t*) &lAddr);

	    //! оригинал
	    //addr_t lAddr;
	    //createRandomAddress(&lAddr);
	    //SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, &lAddr);
  }
#endif /* I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE */

  /* Keep trying to join (a side effect of successful initialization) until
   * successful. Toggle LEDS to indicate that joining has not occurred.
   */
  SMPL_Init(0);

  tx_send_wait("\r\nSimpliciTI - Simple Access Point with UART", sizeof("\r\nSimpliciTI - Simple Access Point with UART"));
  tx_send_wait("\r\nEnd Device.", sizeof("\r\nEnd Device."));
  tx_send_wait("\r\nTrying to join", sizeof("\r\nTrying to join"));

  // Псевдопрерывание от радиопередачи по аналогу с UART IRQ
	radio_processing();


  while (SMPL_SUCCESS != SMPL_Init(0))
  {
    toggleLED(1);
    toggleLED(2);
    SPIN_ABOUT_A_SECOND;
    tx_send_wait(".", sizeof("."));
  }
  tx_send_wait("\r\nSuccessful joined.", sizeof("\r\nSuccessful joined."));

  /* LEDs on solid to indicate successful join. */
  if (!BSP_LED2_IS_ON())
  {
    toggleLED(2);
  }
  if (!BSP_LED1_IS_ON())
  {
    toggleLED(1);
  }

  /* Unconditional link to AP which is listening due to successful join. */
  linkTo();

  while (1) ;
}

static void linkTo()
{
  uint8_t     msg[2];
  uint8_t     button, misses, done;

  /* Keep trying to link... */
  tx_send_wait("\r\nTrying to link.", sizeof("\r\nTrying to link."));

  while (SMPL_SUCCESS != SMPL_Link(&sLinkID1))
  {
    toggleLED(1);
    toggleLED(2);
    SPIN_ABOUT_A_SECOND;
    tx_send_wait(".", sizeof("."));
  }

  tx_send_wait("\r\nLink established!", sizeof("\r\nLink established!"));
  /* Turn off LEDs. */
  if (BSP_LED2_IS_ON())
  {
    toggleLED(2);
  }
  if (BSP_LED1_IS_ON())
  {
    toggleLED(1);
  }


//====================================================================
tx_send_wait("\r\nEnter a message for transmitting on radio channel >",sizeof("\r\nEnter a message for transmitting on radio channel >"));
  	  while(true)
	{
  		PAUSE_COUNTER++;
  				if(PAUSE_COUNTER == 120000)		// Если задержка выдержана
  				{
  					PAUSE_COUNTER = 0;				// Сбрасываем счетчик
  					if(uart_rx_bufer_pointer != 0)	// Если буфер приема UART не пустой
  					{
  						UART_IRQ_DISABLE( UART_NUMBER, UART_LOCATION, RX ); // Запрещаем прерывание UART на прием
  						tx_send_wait(uart_rx_bufer,uart_rx_bufer_pointer);	// Передаем по UART передаваемый по RF символ

  				      uint8_t      noAck;
  				      smplStatus_t rc;

  				    /* get radio ready...awakens in idle state */
  				    // SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);

  				      /* Set TID and designate which LED to toggle */
  				      msg[1] = ++sTid;

  				      while(uart_rx_bufer_pointer)
  				      {
  				    	  msg[0] = uart_rx_bufer[uart_rx_bufer_pointer-1];
  				    	  msg[1] = MSG_ID;
  				    	  done = 0;
  				    	  while (!done)
  				    	  {
  				    		  noAck = 0;
  				    		  /* Try sending message MISSES_IN_A_ROW times looking for ack */
  				    		  for (misses=0; misses < MISSES_IN_A_ROW; ++misses)
  				    		  {
  				    			  if (SMPL_SUCCESS == (rc=SMPL_SendOpt(sLinkID1, msg, sizeof(msg), SMPL_TXOPTION_ACKREQ)))
  				    			  {
  				    				  /* Message acked. We're done.*/
  				    				tx_send_wait("\r\nMessage acked.",sizeof("\r\nMessage acked."));
  				    				tx_send_wait("\r\n>",sizeof("\r\n>"));
  				    				  break;
  				    			  }
  				    			  if (SMPL_NO_ACK == rc)
  				    			  {
  				    				  /* Count ack failures. Could also fail becuase of CCA and
  				    				   * we don't want to scan in this case.
  				    				   */
  				    				  noAck++;
  				    			  }
  				    		  }
  				    		  if (MISSES_IN_A_ROW == noAck)
  				    		  {
  				    			  /* Message not acked. */
  				    			tx_send_wait("\r\nMessage not acked.",sizeof("\r\nMessage not acked."));
  					#ifdef FREQUENCY_AGILITY
  				    			  /* Assume we're on the wrong channel so look for channel by
  				    			   * using the Ping to initiate a scan when it gets no reply. With
  				    			   * a successful ping try sending the message again. Otherwise,
  				    			   * for any error we get we will wait until the next button
  				    			   * press to try again.
  				    			   */
  				    			  if (SMPL_SUCCESS != SMPL_Ping(sLinkID1))
  				    			  {
  				    				  done = 1;
  				    			  }
  					#else
  				    			  done = 1;
  					#endif  /* FREQUENCY_AGILITY */
  				    		  }
  				    		  else
  				    		  {
  				    			  /* Got the ack or we don't care. We're done. */
  				    			  done = 1;
  				    		  }
  				    	  }
  				    	  uart_rx_bufer_pointer--;
  				      }

  				    	  /* radio back to sleep */
  				    	  //SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);


  						/*if(RF_MESSAGE_SENT == true)	// Если прошлая передача завершена
  						{
  							RF_MESSAGE_SENT = false;
  							// turn on RX. default is RX off.
  							SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);
  							rf_tx_send(uart_rx_bufer,uart_rx_bufer_pointer);	// передаем символ по RF
  							uart_rx_bufer_pointer = 0;		// Сбрасываем указатель
  							while(RF_MESSAGE_SENT != true) {;}	// Ожидаем завершения передачи по RF
  							tx_send("\r\nMessage is sended.\r\nEnter new message >",sizeof("\r\nMessage is sended.\r\nEnter new message >"));
  						}*/

  						UART_IRQ_ENABLE( UART_NUMBER, UART_LOCATION, RX ); // Разрешаем прерывание UART на прием
  					}
  				}
	}


//====================================================================



/* sleep until button press... */
  SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);

  while (1)
  {
    button = 0;
    /* Send a message when either button pressed */
    if (BSP_BUTTON1())
    {
      SPIN_ABOUT_A_QUARTER_SECOND;  /* debounce... */
      /* Message to toggle LED 1. */
      button = 1;
    }
    else if (BSP_BUTTON2())
    {
      SPIN_ABOUT_A_QUARTER_SECOND;  /* debounce... */
      /* Message to toggle LED 2. */
      button = 2;
    }
    if (button)
    {
      uint8_t      noAck;
      smplStatus_t rc;

      /* get radio ready...awakens in idle state */
      SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);

      /* Set TID and designate which LED to toggle */
      msg[1] = ++sTid;
      msg[0] = (button == 1) ? 1 : 2;
      done = 0;
      while (!done)
      {
        noAck = 0;

        /* Try sending message MISSES_IN_A_ROW times looking for ack */
        for (misses=0; misses < MISSES_IN_A_ROW; ++misses)
        {
          if (SMPL_SUCCESS == (rc=SMPL_SendOpt(sLinkID1, msg, sizeof(msg), SMPL_TXOPTION_ACKREQ)))
          {
            /* Message acked. We're done. Toggle LED 1 to indicate ack received. */
            toggleLED(1);
            break;
          }
          if (SMPL_NO_ACK == rc)
          {
            /* Count ack failures. Could also fail becuase of CCA and
             * we don't want to scan in this case.
             */
            noAck++;
          }
        }
        if (MISSES_IN_A_ROW == noAck)
        {
          /* Message not acked. Toggle LED 2. */
          toggleLED(2);
#ifdef FREQUENCY_AGILITY
          /* Assume we're on the wrong channel so look for channel by
           * using the Ping to initiate a scan when it gets no reply. With
           * a successful ping try sending the message again. Otherwise,
           * for any error we get we will wait until the next button
           * press to try again.
           */
          if (SMPL_SUCCESS != SMPL_Ping(sLinkID1))
          {
            done = 1;
          }
#else
          done = 1;
#endif  /* FREQUENCY_AGILITY */
        }
        else
        {
          /* Got the ack or we don't care. We're done. */
          done = 1;
        }
      }

      /* radio back to sleep */
      SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
    }
  }
}


void toggleLED(uint8_t which)
{
  if (1 == which)
  {
    BSP_TOGGLE_LED1();
  }
  else if (2 == which)
  {
    BSP_TOGGLE_LED2();
  }
  return;
}
