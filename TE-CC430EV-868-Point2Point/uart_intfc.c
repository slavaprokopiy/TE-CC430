/**************************************************************************************************
  Filename:       uart_intfc.c
  Revised:        $Date: 2009-08-17 10:50:58 -0700 (Mon, 17 Aug 2009) $
  Author:         $Author: jnoxon $

  Description:    This file supports the SimpliciTI-compatible UART API functions.

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
#include <stdlib.h>
#include "uart_intfc.h"

/******************************************************************************
 * CONSTANTS AND DEFINES
 */
#ifndef RX_TX_BUFFER_SIZE       /* this value must be at least 2. */
#define RX_TX_BUFFER_SIZE 30
#endif

#if RX_TX_BUFFER_SIZE < 2
  #error "ERROR:  The value of the macro RX_TX_BUFFER_SIZE must be at least 2."
#endif

#define UART_MINIMUM_BUFFER_MARGIN ( ( RX_TX_BUFFER_SIZE > 32 ) \
                                     ? 8 : ( ( RX_TX_BUFFER_SIZE > 16 ) \
                                           ? ( RX_TX_BUFFER_SIZE + 1 ) / 2 \
                                           : RX_TX_BUFFER_SIZE - 1 ) )

/******************************************************************************
 * LOCAL VARIABLES
 */
static unsigned char rx_buff[RX_TX_BUFFER_SIZE];
static unsigned char tx_buff[RX_TX_BUFFER_SIZE];

static unsigned char rf_buff[RX_TX_BUFFER_SIZE];

static unsigned char* rx_head = rx_buff;
static unsigned char* rx_tail = rx_buff;

static unsigned char* tx_head = tx_buff;
static unsigned char* tx_tail = tx_buff;

static unsigned char* rf_head = rf_buff;
static unsigned char* rf_tail = rf_buff;

/******************************************************************************
 * LOCAL FUNCTIONS
 */

/* uart data handler function prototypes */
bool tx_handler( unsigned char* c );
bool rx_handler( unsigned char c );
bool rf_handler( unsigned char* c );

static int buffer_free_space( unsigned char* head, unsigned char* tail );
static int buffer_used_space( unsigned char* head, unsigned char* tail );
static bool push_buffer( unsigned char** head, unsigned char* tail,
                         unsigned char* buff, unsigned char* data, int len );
static int pop_buffer( unsigned char* head, unsigned char** tail,
                       unsigned char* buff, unsigned char* data, int max_len );

/******************************************************************************
 * GLOBAL FUNCTIONS
 */

/******************************************************************************
 * @fn          uart_intfc_init
 *
 * @brief       Initialize UART interface. Buffer pointers are initialized.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   void
 */
void uart_intfc_init( void )
  {
  bspIState_t istate;

  uart_init( ); // initialize the uart for operations
  
  BSP_ENTER_CRITICAL_SECTION( istate );

  // initialize the buffer pointers in case we are re-initialized
  rx_head = rx_buff;
  rx_tail = rx_buff;
  tx_head = tx_buff;
  tx_tail = tx_buff;

  rf_head = rf_buff;
  rf_tail =	rf_buff;

  BSP_EXIT_CRITICAL_SECTION( istate );

  uart_rx_message( rx_handler ); // enable us to receive uart data
  uart_tx_message( tx_handler ); // enable us to transmit uart data

  rf_tx_message( rf_handler ); // enable us to transmit rf data

  return;
  }

/******************************************************************************
 * @fn          buffer_free_space
 *
 * @brief       Calculates amount of freespace in buffer from <head> and <tail>
 *              addresses
 *
 * input parameters
 * @param   head       - pointer to buffer head
 * @param   tail       - pointer to buffer tail
 *
 * output parameters
 *
 * @return   Buffer free space count in bytes
 */
int buffer_free_space( unsigned char* head, unsigned char* tail )
  {
  /* the free count is the buffer size minus the used count minus one
   * because we don't want the pointers to ever get back on top of each other
   * because that would indicate an empty buffer. */
  return RX_TX_BUFFER_SIZE - buffer_used_space( head, tail ) - 1;
  }

/******************************************************************************
 * @fn          buffer_used_space
 *
 * @brief       Calculates amount of used space in buffer from <head> and <tail>
 *              addresses
 *
 * input parameters
 * @param   head       - pointer to buffer head
 * @param   tail       - pointer to buffer tail
 *
 * output parameters
 *
 * @return   Buffer used space count in bytes
 */
int buffer_used_space( unsigned char* head, unsigned char* tail )
  {
  ptrdiff_t used;
  
  used = head - tail; // get used count
  if( used < 0 ) // if the pointers were wrapped
    used += RX_TX_BUFFER_SIZE; // correct the count
  
  return used; // return used count
  }

/******************************************************************************
 * @fn          push_buffer
 *
 * @brief       Pushes bytes of data onto the specified buffer. Assumes on 
 *              entry that <data>, <buff>, <tail>, and <head> are all valid 
 *              pointers
 *
 * input parameters
 * @param   tail       - pointer to buffer tail
 * @param   buff       - pointer to buffer (push target)
 * @param   data       - pointer to data to be pushed into buffer
 * @param   len        - length in bytes of data to be pushed
 *
 * output parameters
 * @param   head       - updated buffer head pointer
 *
 * @return   status of operation.
 *             true          Data successfully pushed into buffer
 *             false         Data not pushed into buffer
 */

bool push_buffer( unsigned char** head, unsigned char* tail,
                  unsigned char* buff, unsigned char* data, int len )
  {
  unsigned char* local_head;

  BSP_CRITICAL_STATEMENT( local_head = *head );
  
  // if no room in the buffer
  if( buffer_free_space( local_head, tail ) < len )
    return false; // indicate failure to enqueue message
  
  // there is room for the data, put in the buffer
  
  do // put the data in the buffer
    {
    if( local_head == buff + RX_TX_BUFFER_SIZE ) // if wrapping around
      local_head = buff; // reset pointer
    
    *local_head++ = *data++; // copy over this byte of data
    } while( --len > 0 ); // copy all the data to the buffer
  
  BSP_CRITICAL_STATEMENT( *head = local_head ); // update reference value
  
  return true;
  }

/******************************************************************************
 * @fn          pop_buffer
 *
 * @brief       Pops the specified number of bytes off of the specified buffer. 
 *              Assumes on entry that <data>, <buff>, <tail>, and <head> are 
 *              all valid pointers.
 *
 * input parameters
 * @param   head       - pointer to buffer head
 * @param   buff       - pointer to buffer (pop source)
 * @param   data       - pointer to location to store popped data
 * @param   len        - amount of bytes to be popped from buffer
 *
 * output parameters
 * @param   tail       - updated buffer tail pointer
 *
 * @return   number of bytes popped from buffer
 */

int pop_buffer( unsigned char* head, unsigned char** tail,
                unsigned char* buff, unsigned char* data, int max_len )
  {
  unsigned char* local_tail;
  int cnt = 0;

  BSP_CRITICAL_STATEMENT( local_tail = *tail );
  
  // if the buffer is empty or no data requested
  if( local_tail == head || max_len <= 0 )
    return 0; // indicate so
  
  do // retrieve the data from the buffer
    {
    if( local_tail == buff + RX_TX_BUFFER_SIZE ) // if wrapping around
      local_tail = buff; // reset pointer
    
    *data++ = *local_tail++; // copy data from buffer
      
    // while the user needs more data and there is data left in the fifo
    } while( ++cnt < max_len && local_tail != head );
    
  BSP_CRITICAL_STATEMENT( *tail = local_tail ); // update reference value
  
  return cnt; // return number of characters retrieved from the buffer
  }

/******************************************************************************
 * @fn          tx_send_wait
 *
 * @brief       Enqueue's the message pointed to by <data> which is of length
 *              <len> and initiates its transfer across the UART.  This is a 
 *              blocking function in that if the transmit fifo doesn't have 
 *              enough room to enqueue the data in its entirety it will push 
 *              the data out a piece at a time as the room in the FIFO becomes 
 *              available.  The function returns true upon completion of moving 
 *              all the data into the FIFO and false if either a NULL pointer 
 *              or a length of zero was passed.
 *
 * input parameters
 * @param   data       - pointer to data to be sent
 * @param   len        - length in bytes of data to be sent
 *
 * output parameters
 *
 * @return   status of operation.
 *             true          Data successfully pushed into transmit buffer
 *             false         Invalid length or NULL data pointer
 */

bool tx_send_wait( const void* data, size_t len )
  {
  if( len > 0 && data != NULL ) // if the information looks viable
    {
    while( len > 0 ) // while there is data left to transfer
      {
      int sz = tx_peek( ); // get free space in the fifo

      if( sz > 0 ) // if there is room for at least some data
        {
        if( sz > len ) // if more room than required
          sz = len; // limit size to just the data to send

        tx_send( data, sz ); // send this portion of the information

        data = (unsigned char*)data + sz; // move the pointer

        len -= sz; // adjust the count of remaining data to send
        }
      
      #ifdef FREQUENCY_HOPPING
      // run the pll charge pump if frequency hopping active
      //   only send pump requests if there are still characters still to send
      nwk_pllBackgrounder( len == 0 );
      #endif
      }
    
    return true; // indicate success
    }
  
  return false; // otherwise indicate failure
  }

bool rf_tx_send( const void* data, size_t len )
  {
  bool status;
  unsigned char* tail;

  // get current state of tail pointer
  BSP_CRITICAL_STATEMENT( tail = rf_tail );

  // put data into transmit buffer
  status = push_buffer( &rf_head, tail, rf_buff, (unsigned char*)data, len );

  if( status != false ) // if data was put in the buffer properly
    rf_tx_message_resume( rf_handler ); // notify the irq that data is ready to send

  return status; /* return status */
  }

bool rf_tx_send_wait( const void* data, size_t len )
  {
  if( len > 0 && data != NULL ) // if the information looks viable
    {
    while( len > 0 ) // while there is data left to transfer
      {
      int sz = rf_tx_peek( ); // get free space in the fifo

      if( sz > 0 ) // if there is room for at least some data
        {
        if( sz > len ) // if more room than required
          sz = len; // limit size to just the data to send

        rf_tx_send( data, sz ); // send this portion of the information

        data = (unsigned char*)data + sz; // move the pointer

        len -= sz; // adjust the count of remaining data to send
        }

      #ifdef FREQUENCY_HOPPING
      // run the pll charge pump if frequency hopping active
      //   only send pump requests if there are still characters still to send
      nwk_pllBackgrounder( len == 0 );
      #endif
      }

    return true; // indicate success
    }

  return false; // otherwise indicate failure
  }
/******************************************************************************
 * @fn          tx_send
 *
 * @brief       Enqueue's the message pointed to by <data> which is of length 
 *              <len> and initiates its transfer across the uart.  true is 
 *              returned if there was space in the FIFO to send the data, false 
 *              if the FIFO didn't have enough free space to enqueue the data. 
 *
 * input parameters
 * @param   data       - pointer to data to be sent
 * @param   len        - length in bytes of data to be sent
 *
 * output parameters
 *
 * @return   status of operation.
 *             true          Data successfully pushed into transmit buffer
 *             false         Data not successfully pushed into transmit buffer
 */

bool tx_send( const void* data, size_t len )
  {
  bool status;
  unsigned char* tail;
  
  // get current state of tail pointer
  BSP_CRITICAL_STATEMENT( tail = tx_tail );

  // put data into transmit buffer
  status = push_buffer( &tx_head, tail, tx_buff, (unsigned char*)data, len );

  if( status != false ) // if data was put in the buffer properly
    uart_tx_message_resume( tx_handler ); // notify the irq that data is ready to send
  
  return status; /* return status */
  }


bool rf_buffer_fill( const void* data, size_t len )
  {
  bool status;
  unsigned char* tail;

  // get current state of tail pointer
  BSP_CRITICAL_STATEMENT( tail = rf_tail );

  // put data into transmit buffer
  status = push_buffer( &rf_head, tail, rf_buff, (unsigned char*)data, len );

  if( status != false ) // if data was put in the buffer properly
    rf_tx_message_resume( rf_handler ); // notify the irq that data is ready to send

  return status; /* return status */
  }


/******************************************************************************
 * @fn          tx_peek
 *
 * @brief       Returns the number of bytes of free space in the transmit FIFO. 
 *
 * input parameters
 *
 * output parameters
 *
 * @return   Number of bytes of free space in the transmit FIFO
 */

int tx_peek( void )
  {
  unsigned char* head;
  unsigned char* tail;
  
  BSP_CRITICAL_STATEMENT( head = tx_head; tail = tx_tail );
  
  return buffer_free_space( head, tail );
  }

/******************************************************************************
 * @fn          rx_peek
 *
 * @brief       Returns the number of bytes currently available to be read from
 *              the receive FIFO. 
 *
 * input parameters
 *
 * output parameters
 *
 * @return   Number of bytes of available in the receive FIFO
 */

int rx_peek( void )
  {
  unsigned char* head;
  unsigned char* tail;
  
  BSP_CRITICAL_STATEMENT( head = rx_head; tail = rx_tail );
  
  return buffer_used_space( head, tail );
  }

/******************************************************************************
 * @fn          rx_receive
 *
 * @brief       Fills in the buffer <data> with data from the receive FIFO until
 *              either <max_len> bytes have been transferred into <data> or the 
 *              receive queue is emptied.  The actual number of bytes put into 
 *              <data> is returned. 
 *
 * input parameters
 * @param   data           - pointer to data to be sent
 * @param   max_len        - length in bytes of data to be sent
 *
 * output parameters
 *
 * @return   status of operation.
 *             true          Data successfully pushed into transmit buffer
 *             false         Data not successfully pushed into transmit buffer
 */

int rx_receive( void* data, int max_len )
  {
  int cnt;
  unsigned char* head;
  unsigned char* tail;
  
  // get current state of head pointer
  BSP_CRITICAL_STATEMENT( head = rx_head );
  
  // retrieve data from buffer
  cnt = pop_buffer( head, &rx_tail, rx_buff, data, max_len );

  // get current state of tail pointer
  BSP_CRITICAL_STATEMENT( tail = rx_tail );  if( buffer_free_space( head, tail ) >= UART_MINIMUM_BUFFER_MARGIN )
    uart_rx_message_resume( rx_handler ); // turn reception back on if buffer has room

  return cnt; // indicate the number of bytes retrieved from the buffer
  }

/******************************************************************************
 * @fn          uart_busy
 *
 * @brief       Returns true if there are characters in the receive FIFO or 
 *              transmit buffer and returns false if both FIFOs are empty. 
 *
 * input parameters
 *
 * output parameters
 *
 * @return   UART status
 *             true          Data in receive and/or transmit FIFO
 *             false         Both receive and transmit FIFOS empty
 */

bool uart_busy( void )
  {
  int cnt;
  unsigned char* head;
  unsigned char* tail;
  
  /* get receive buffer count */
  BSP_CRITICAL_STATEMENT( head = rx_head; tail = rx_tail );
  cnt = buffer_used_space( head, tail );
  
  /* get transmit buffer count */
  BSP_CRITICAL_STATEMENT( head = tx_head; tail = tx_tail );
  cnt += buffer_used_space( head, tail );
  
  return ( ( cnt == 0 ) ? false : true ); /* return status of uart */
  }

static int tx_cntr = 0;
bool tx_handler( unsigned char* c )
  {
  int cnt;
  unsigned char* head;

  /* get current state of head pointer */
  BSP_CRITICAL_STATEMENT( head = tx_head );

  // get the next character from the buffer
  cnt = pop_buffer( head, &tx_tail, tx_buff, c, 1 );
  
  tx_cntr += cnt;
  
  return ( cnt != 0 ) ? true : false; // return status
  }

static int rf_cntr = 0;
bool rf_handler( unsigned char* c )
  {
  int cnt;
  unsigned char* head;

  /* get current state of head pointer */
  BSP_CRITICAL_STATEMENT( head = rf_head );

  // get the next character from the buffer
  cnt = pop_buffer( head, &rf_tail, rf_buff, c, 1 );

  rf_cntr += cnt;

  return ( cnt != 0 ) ? true : false; // return status
  }

static int rx_cntr = 0;
bool rx_handler( unsigned char c )
  {
  unsigned char* tail;
  unsigned char* head;

  // get current state of tail pointer
  BSP_CRITICAL_STATEMENT( tail = rx_tail );

  // put data onto the receive buffer
  push_buffer( &rx_head, tail, rx_buff, &c, 1 );
  
  rx_cntr++;
  
  // get current state of head pointer
  BSP_CRITICAL_STATEMENT( head = rx_head );

  if( buffer_free_space( head, tail ) < UART_MINIMUM_BUFFER_MARGIN )
    uart_rx_message_suspend( rx_handler ); // halt reception if buffer is full

  return true; // always accept data received from the uart
  }
