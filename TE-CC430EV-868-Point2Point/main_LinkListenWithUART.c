/*----------------------------------------------------------------------------
 *  Demo Application for SimpliciTI
 *
 *  L. Friedman
 *  Texas Instruments, Inc.
 *----------------------------------------------------------------------------
 */
/******************************************************************************************

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
#include "app_remap_led.h"
#include "uart.h"
#include "uart_intfc.h"

extern uint8_t  msg[2];

static void linkFrom(void);
void toggleLED(uint8_t);

static uint8_t  sRxTid = 0;
extern linkID_t sLinkID2;

extern unsigned long PAUSE_COUNTER;

extern unsigned char 	uart_rx_bufer[];
extern int 				uart_rx_bufer_pointer;

extern unsigned char 	rf_rx_bufer[];
extern int 				rf_rx_bufer_pointer;
extern uart_get_tx_data_type rf_handler;
extern bool				rf_tx_suspend;
extern int 				RF_MESSAGE_SENT;
extern int				RADIO_INT_ENABLE;
extern int				RF_ACK_FLAG;

/* Rx callback handler */
static uint8_t sRxCallback(linkID_t);

void main (void){
  BSP_Init(); // Инициализация платы
  // Инициализация UART

  const addr_t myAddr = {0x??, 0x??, 0x??, 0x??};
  // set device address
  SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, (addr_t*) &myAddr);

  /* This call will fail because the join will fail since there is no Access Point
   * in this scenario. But we don't care -- just use the default link token later.
   * We supply a callback pointer to handle the message returned by the peer.
   */
  SMPL_Init(sRxCallback);

  // Псевдопрерывание от радиопередачи по аналогу с UART IRQ
  radio_processing();

  /* turn on LED */
  if (!BSP_LED1_IS_ON()){
	  toggleLED(1);
  }

  //Тестовое моргание с периодом 1 сек для проверки тактирующей частоты
 /*while(true)
  {
	  toggleLED(1);
	  __delay_cycles(6000000); //Задержка в 0.5 секунды
  }
*/
  tx_send_wait("\r\nSimpliciTI - Simple Peer to Peer Example with UART", sizeof("\r\nSimpliciTI - Simple Peer to Peer Example with UART"));
  tx_send_wait("\r\nLinkListen. Press Button S2 to continue...", sizeof("\r\nLinkListen. Press Button S2 to continue..."));

  /* wait for a button press... */
  do {if (BSP_BUTTON1()){
      break;
    }
  } while (1);

  tx_send_wait("\r\nWaiting for a link - linkListen()",sizeof("\r\nWaiting for a link - linkListen()"));

  /* never coming back... */
  linkFrom();

  /* but in case we do... */
  while (1) ;
}

static void linkFrom(){
	uint8_t	tid = 0;
	/* Turn off one LED so we can tell the device is now listening.
	* Received messages will toggle the other LED.
	*/
	toggleLED(1);

	/* listen for link forever... */
	while (1){
		if (SMPL_LinkListen(&sLinkID2) == SMPL_SUCCESS){
			break;
		}
		// Implement fail-to-link policy here. otherwise, listen again.
	}
	// We're linked. turn on LED.
	toggleLED(1);
	tx_send_wait("\r\nLink established!\r\nWaiting for input message...",sizeof("\r\nLink established!\r\nWaiting for input message..."));
	/* turn on RX. default is RX off. */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

	while(true){
   	   	PAUSE_COUNTER++;
   		if(PAUSE_COUNTER == 120000){		// Если задержка выдержана
   			PAUSE_COUNTER = 0;				// Сбрасываем счетчик
   			// Обработка сообщения, принятого по UART =======================================================================
   			if(uart_rx_bufer_pointer != 0){								// Если буфер приема UART не пустой
   				UART_IRQ_DISABLE( UART_NUMBER, UART_LOCATION, RX );		// Запрещаем прерывание UART на прием
   				tx_send_wait(uart_rx_bufer,uart_rx_bufer_pointer);		// Передаем по UART передаваемый по RF символ
   				if(RF_MESSAGE_SENT == true){								// Если прошлая передача завершена
   					RF_MESSAGE_SENT = false;
   					rf_tx_send(uart_rx_bufer,uart_rx_bufer_pointer);	// передаем символ по RF
   					uart_rx_bufer_pointer = 0;							// Сбрасываем указатель
   					while(RF_MESSAGE_SENT != true) {;}					// Ожидаем завершения передачи по RF
   					tx_send("\r\nMessage sent.\r\nEnter new message >",sizeof("\r\nMessage sent.\r\nEnter new message >"));
   				}
   				UART_IRQ_ENABLE( UART_NUMBER, UART_LOCATION, RX );		// Разрешаем прерывание UART на прием
   			}
   			//================================================================================================================
   			// Обработка сообщения, принятого по радиоканалу ========================================================================
   			if(rf_rx_bufer_pointer != 0){								// Если буфер радиоприема не пустой
   				RADIO_INT_ENABLE = false;								// Запрещаем обработку радиопрерывания
   				tx_send_wait("\r\nReceived message >",sizeof("\r\nReceived message >"));
   				tx_send_wait(rf_rx_bufer,rf_rx_bufer_pointer);			// Передаем по UART принятое по радиоканалу сообщение
   				tx_send_wait("\r\nWaiting for input message...",sizeof("\r\nWaiting for input message..."));
   				rf_rx_bufer_pointer = 0;
   				RADIO_INT_ENABLE = true;								// Разрешаем обработку радиопрерывания
   			}
   			//==================================================================================================================================
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

/* handle received frames. */
static uint8_t sRxCallback(linkID_t port) //Прием сообщения
{
	uint8_t msg[2], len, tid;
	PAUSE_COUNTER = 0;
	if(RADIO_INT_ENABLE == true){	// Если разрешена обработка радиопрерывания
		/* is the callback for the link ID we want to handle? */
		if (port == sLinkID2){
			// go get the frame. we know this call will succeed
			if ((SMPL_SUCCESS == SMPL_Receive(sLinkID2, msg, &len)) && len){
					if(msg[0] == RCVR_ACK){		// Если получено подтверждение от приемника
						RF_ACK_FLAG = true;		// Ставим флаг получения подтверждения
						radio_processing();		// Обрабатываем прерывание радиоприема
					}
					if(msg[0] == SYMBOL_IDENT){	// Если в принятом сообщении стоит идентификатор символа
						if(rf_rx_bufer_pointer != RX_TX_BUFFER_SIZE){	// Если буфер радиоприема не переполнен
							rf_rx_bufer[rf_rx_bufer_pointer] =  msg[1];	//принимаем сообщение
							rf_rx_bufer_pointer++;
						}
						// Отправляем подтверждение о приеме символа
						msg[0] = RCVR_ACK;
						//!SMPL_Send(sLinkID2, msg, 2);
					}
			}
				return 1;
		}
			return 0;
	}
}
