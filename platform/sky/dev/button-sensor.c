/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: button-sensor.c,v 1.4 2010/01/14 20:01:19 nifi Exp $
 */
#include "lib/sensors.h"
#include "dev/hwconf.h"
#include "dev/button-sensor.h"

#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <signal.h>
#endif

const struct sensors_sensor button_sensor;

static struct timer debouncetimer;
static int status(int type);

HWCONF_PIN(BUTTON, 2, 7);
HWCONF_IRQ(BUTTON, 2, 7);

/*---------------------------------------------------------------------------*/
#ifdef __IAR_SYSTEMS_ICC__
#pragma vector=PORT2_VECTOR
__interrupt void
#else
interrupt(PORT2_VECTOR)
#endif
     irq_p2(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(BUTTON_CHECK_IRQ()) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 4);
      sensors_changed(&button_sensor);
      LPM4_EXIT;
    }
  }
  P2IFG = 0x00;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return BUTTON_READ() || !timer_expired(&debouncetimer);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch (type) {
  case SENSORS_ACTIVE:
    if (c) {
      if(!status(SENSORS_ACTIVE)) {
	timer_set(&debouncetimer, 0);
	BUTTON_IRQ_EDGE_SELECTD();

	BUTTON_SELECT();
	BUTTON_MAKE_INPUT();

	BUTTON_ENABLE_IRQ();
      }
    } else {
      BUTTON_DISABLE_IRQ();
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return BUTTON_IRQ_ENABLED();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);
