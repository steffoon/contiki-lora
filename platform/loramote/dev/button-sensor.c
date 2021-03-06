/*
 * Copyright (c) 2012, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
#include "dev/button-sensor.h"
#include "lora-contiki-interface.h"
/*---------------------------------------------------------------------------*/
static int _initialized = 0;
static int _active = 0;
extern Gpio_t NIrqSx9500;
extern Gpio_t TxEnSx9500;
/*---------------------------------------------------------------------------*/
void cascaded_button_interrupt(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  uint8_t statusSX1509 = 0;
  uint8_t statusSX9500 = 0;

  SX1509Read(RegInterruptSourceB, &statusSX1509);
  SX9500Read(SX9500_REG_IRQSRC, &statusSX9500);

  /* Check if interrupt is generated by SX1509 IOE8 pin & SX9500 close proximity sensor */
  if(((statusSX1509 & 0x01) == 0x01) && ((statusSX9500 & 0x40) == 0x40)){
    sensors_changed(&button_sensor);
  }

  /* Clear NINT interrupt */
  /* (NIRQ interrupt is automatically cleared by reading interrupt source register) */
  SX1509Read(RegInterruptSourceB, &statusSX1509);
  SX1509Write(RegInterruptSourceB, (statusSX1509 & 0xFE));

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
static void init(void)
{
  uint8_t status = 0;

  /* Initialize the SX9500 proximity sensor */
  GpioInit(&NIrqSx9500, N_IRQ_SX9500, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1);
  GpioInit(&TxEnSx9500, TX_EN_SX9500, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1);

  SX9500Init();
  SX9500Write(SX9500_REG_IRQMSK, 0x10);
  SX9500Write(SX9500_REG_IRQSRC, 0x10);
  
  //HANGS! LoraMAC based SX9500 code changed?
  //do {
  //  SX9500Read(SX9500_REG_IRQSRC, &status);
  //}while((status & 0x10) == 0x00); /* While compensation for CS0 is pending */


  /* Enable SX9500 close proximity interrupt (NIRQ) */
  SX9500Write(SX9500_REG_IRQMSK, 0x40);
  SX9500Write(SX9500_REG_IRQSRC, 0x00);
  /* Increase proximity detection threshold */
  SX9500Write(SX9500_REG_PROXCTRL6, 0x1F);

  /* Enable the SX1509 IO Expander interrupt (NINT) */
  SX1509Write(RegInterruptMaskB, 0xFE);
  /* Set edge detection to falling */
  SX1509Write(RegSenseLowB, 0x02);
  SX1509Read(RegPullUpB, &status);
  SX1509Write(RegPullUpB, (status | 0x01));

  /* Initialize WKUP1/EXTI0 interrupt */
  Gpio_t IrqSX1509;
  GpioInit(&IrqSX1509, WKUP1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
  GpioSetInterrupt(&IrqSX1509, IRQ_FALLING_EDGE, IRQ_VERY_LOW_PRIORITY, &cascaded_button_interrupt);

  _initialized = 1;
  _active = 1;
}
/*---------------------------------------------------------------------------*/
static void activate(void)
{
  if(!_initialized){
    init();
  }
  _active = 1;
}
/*---------------------------------------------------------------------------*/
static void deactivate(void)
{
  _active = 0;
}
/*---------------------------------------------------------------------------*/
static int active(void)
{
  return _active;
}
/*---------------------------------------------------------------------------*/
static int value(int type)
{
  uint8_t regValue = 0;
  uint16_t offset = 0;

  /* Read 1st sensor offset */
  SX9500Read(SX9500_REG_OFFSETMSB, (uint8_t*)&regValue);
  offset = regValue << 8;
  SX9500Read(SX9500_REG_OFFSETLSB, (uint8_t*)&regValue);
  offset |= regValue;

  return (offset > 2000);
}
/*---------------------------------------------------------------------------*/
static int configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    init();
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      activate();
    } else {
      deactivate();
    }
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int status(int type)
{
  switch(type) {
  case SENSORS_READY:
    return active();
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
