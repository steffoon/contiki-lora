/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Bleeper board UART driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"

#include "board.h"

#include "usb_istr.h"
#include "usb_pwr.h"

#include "uart-usb-board.h"


#include "sys/ctimer.h"  //Contiki ctimer

/*!
 * FIFO buffers size
 */
#define FIFO_RX_SIZE                                1024
uint8_t CdcRxBuffer[FIFO_RX_SIZE];

#define FIFO_TX_SIZE                                1024
uint8_t CdcTxBuffer[FIFO_TX_SIZE];

volatile uint8_t UsbPacketTx = 1;
volatile  uint8_t UsbTxBuffer[VIRTUAL_COM_PORT_DATA_SIZE];
volatile  uint8_t UsbRxBuffer[VIRTUAL_COM_PORT_DATA_SIZE];

typedef enum{
	UsbPutBuffer,
	EP_IN_Callback
} TransmitType;

const TransmitType putBuffer = UsbPutBuffer;
const TransmitType EP_callback = EP_IN_Callback;

static struct ctimer UartUsbTimer;
uint8_t UartUsbTimerOperational = 0;


void UartUsbInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;
}

void UartUsbConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    FifoInit( &obj->FifoRx, CdcRxBuffer, FIFO_RX_SIZE );
    FifoInit( &obj->FifoTx, CdcTxBuffer, FIFO_TX_SIZE );
}

void UartUsbDeInit( Uart_t *obj )
{

}

static void UartUsbTransmit(void *ptr)
{
	//TransmitType t = (TransmitType)*ptr;
	if( (TransmitType*)ptr == EP_IN_Callback && FifoSize(&UartUsb.FifoTx) < VIRTUAL_COM_PORT_DATA_SIZE) //Callback, check if not a full transmit buffer
	{
			return; //Not a full transmit, do not send from USB callback
	}

	else if( (TransmitType*)ptr == UsbPutBuffer)
    {
    	if(UsbPacketTx == 0)
    	{
			// Already sending, reschedule ctimer
			UartUsbTimerOperational = 1;
			ctimer_set(&UartUsbTimer, CLOCK_SECOND/50, UartUsbTransmit, &putBuffer);
			return;
    	}
    	else
    	{
    		UartUsbTimerOperational = 0; //inactivate ctimer
    	}
    }

    UsbPacketTx = 0;

    uint16_t idx = 0;
    uint16_t usbBufferSize = VIRTUAL_COM_PORT_DATA_SIZE - 1;
    uint8_t tempBuffer[VIRTUAL_COM_PORT_DATA_SIZE];


    while(!IsFifoEmpty(&UartUsb.FifoTx) && idx < usbBufferSize)
    {
    	tempBuffer[idx++] = FifoPop(&UartUsb.FifoTx);
    }

    if(!idx){ //Nothing to send
    	UsbPacketTx = 1;
    	return;
    }

	UsbPacketTx = 0;
	UserToPMABufferCopy(tempBuffer, ENDP1_TXADDR, idx );
	SetEPTxCount( ENDP1, idx );
	SetEPTxValid( ENDP1 );


}


uint8_t UartUsbPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size )
{
	int i;
    for( i = 0; i < size; i++ )
    {
        if( !IsFifoFull( &UartUsb.FifoTx ) )
        {
            FifoPush( &UartUsb.FifoTx, buffer[i] );
        }
        else //Buffer full, start transmitting immediately and try again
        {
        	UartUsbTransmit(UsbPutBuffer);

        	while(IsFifoFull( &UartUsb.FifoTx ));

            FifoPush( &UartUsb.FifoTx, buffer[i] );
        }

    }

    if(!UartUsbTimerOperational) //Start timer if not already running
    	ctimer_set(&UartUsbTimer, CLOCK_SECOND/50, UartUsbTransmit, &putBuffer);

    return 0;


	/*
     //BACKUP!
	uint16_t idx = 0;
    uint16_t usbBufferSize = VIRTUAL_COM_PORT_DATA_SIZE - 1;

    if( UsbPacketTx == 0 )
    {
        return 1; // Busy
    }

    while( size > usbBufferSize )
    {
        if( UsbPacketTx == 1 )
        {
            UsbPacketTx = 0;

            UserToPMABufferCopy( buffer + idx, ENDP1_TXADDR, usbBufferSize );
            size -= usbBufferSize;
            idx += usbBufferSize;

            SetEPTxCount( ENDP1, usbBufferSize );
            SetEPTxValid( ENDP1 );
        }
    }

    if( size != 0 )
    {
        // Wait for previous transmission finalization
        while( UsbPacketTx != 1 );

        UsbPacketTx = 0;
        UserToPMABufferCopy( buffer + idx, ENDP1_TXADDR, size );
        SetEPTxCount( ENDP1, size );
        SetEPTxValid( ENDP1 );
    }
    return 0; // OK
*/
}

uint8_t UartUsbPutChar( Uart_t *obj, uint8_t data )
{
   return UartUsbPutBuffer( obj, &data, 1 );
}

uint8_t UartUsbGetChar( Uart_t *obj, uint8_t *data )
{
    if( UsbMcuIsDeviceConfigured( ) == false )
    {
        return 2;
    }
    SetEPRxValid( ENDP3 );
    if( IsFifoEmpty( &obj->FifoRx ) == false )
    {
        __disable_irq( );
        *data = FifoPop( &obj->FifoRx );
        __enable_irq( );
        return 0;
    }
    return 1;
}

void EP1_IN_Callback (void)
{
    UsbPacketTx = 1;
	UartUsbTransmit(&EP_callback);
}

void EP3_OUT_Callback(void)
{
    uint8_t i;
    uint32_t size;
    
    size = GetEPRxCount( ENDP3 );
    PMAToUserBufferCopy( ( unsigned char* )UsbRxBuffer, ENDP3_RXADDR, size );
    
    for( i = 0; i < size; i++ )
    {
        if( IsFifoFull( &UartUsb.FifoRx ) == false )
        {
            // Read one byte from the receive data register
            FifoPush( &UartUsb.FifoRx, UsbRxBuffer[i] );
        }
    }

    if( UartUsb.IrqNotify != NULL )
    {
        UartUsb.IrqNotify( UART_NOTIFY_RX );
    }
}
