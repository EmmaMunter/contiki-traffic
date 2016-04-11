/*
 * Copyright (c) 2014, OpenMote Technologies, S.L.
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

/**
 * \addtogroup openmote-examples
 * @{
 *
 * \defgroup openmote-max44009
 *
 *    This example tests the correct functionality of the MAX44009 light
 *    sensor using the I2C bus.
 *
 * @{
 *
 * \file
 *          Testing the MAX44009 sensor on the OpenMote-CC2538 platform.
 * \author
 *         Pere Tuset <peretuset@openmote.com>
 */

#include "contiki.h"
#include "dev/max44009.h"
#include "dev/leds.h"
#include <stdio.h>

PROCESS(test_max44009_process, "MAX44009 test");
AUTOSTART_PROCESSES(&test_max44009_process);

PROCESS_THREAD(test_max44009_process, ev, data)
{
  static struct etimer et;
  static unsigned raw;
  static float light;

  PROCESS_BEGIN();
  max44009_init();

if (!max44009_is_present()) {
    leds_on(LEDS_ORANGE);
  }

  while(1) {
    etimer_set(&et, CLOCK_SECOND);

    PROCESS_YIELD();

    if (ev == PROCESS_EVENT_TIMER) {
      leds_on(LEDS_YELLOW);
      raw = max44009_read_light();
      light = max44009_convert_light(raw);
      printf("Light: %u.%u\n", (unsigned int) light, (unsigned int) (light * 100) % 100);
      leds_off(LEDS_YELLOW);
    }
  }

  PROCESS_END();
}
