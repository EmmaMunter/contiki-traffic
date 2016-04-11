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
 * \addtogroup openmote-cc2538
 * @{
 *
 * \defgroup openmote-examples OpenMote-CC2538 Example Projects
 * @{
 *
 * \defgroup openmote-demo OpenMote-CC2538 Demo Project
 *
 *   Example project demonstrating the OpenMote-CC2538 functionality
 *
 *   This assumes that you are using an OpenMote-CC2538
 *
 * - Boot sequence: LEDs flashing, LED2 followed by LED3 then LED4
 * - etimer/clock : Every LOOP_INTERVAL clock ticks the LED defined as
 *                  LEDS_PERIODIC will turn on
 * - rtimer       : Exactly LEDS_OFF_HYSTERISIS rtimer ticks later,
 *                  LEDS_PERIODIC will turn back off
 * - UART         : Every LOOP_INTERVAL the EM will print something over the
 *                  UART. Receiving an entire line of text over UART (ending
 *                  in \\r) will cause LEDS_SERIAL_IN to toggle
 * - Radio comms  : BTN_SELECT sends a rime broadcast. Reception of a rime
 *                  packet will toggle LEDs defined as LEDS_RF_RX
 *
 * @{
 *
 * \file
 *     Example demonstrating the OpenMote platform.
 * \author
 *         Pere Tuset <peretuset@openmote.com>
 */
#include "contiki.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/button-sensor.h"
#include "dev/watchdog.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "net/rime/broadcast.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define LOOP_INTERVAL       CLOCK_SECOND
#define LEDS_OFF_HYSTERISIS (RTIMER_SECOND >> 1)
#define LEDS_PERIODIC       LEDS_YELLOW
#define LEDS_BUTTON         LEDS_RED
#define LEDS_SERIAL_IN      LEDS_ORANGE
#define LEDS_REBOOT         LEDS_ALL
#define LEDS_RF_RX          (LEDS_YELLOW | LEDS_ORANGE)
#define BROADCAST_CHANNEL   129
/*---------------------------------------------------------------------------*/
static struct etimer et;
static struct rtimer rt;
static uint16_t counter;
/*---------------------------------------------------------------------------*/
PROCESS(openmote_demo_process, "OpenMote-CC2538 demo process");
AUTOSTART_PROCESSES(&openmote_demo_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  leds_toggle(LEDS_RF_RX);
  printf("Received %u bytes: '0x%04x'\n", packetbuf_datalen(),
         *(uint16_t *)packetbuf_dataptr());
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn bc;
/*---------------------------------------------------------------------------*/
void
rt_callback(struct rtimer *t, void *ptr)
{
  leds_off(LEDS_PERIODIC);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(openmote_demo_process, ev, data)
{

  PROCESS_EXITHANDLER(broadcast_close(&bc))

  PROCESS_BEGIN();

  counter = 0;
  broadcast_open(&bc, BROADCAST_CHANNEL, &bc_rx);

  while(1) {
    etimer_set(&et, CLOCK_SECOND);

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      leds_on(LEDS_PERIODIC);
      printf("Counter = 0x%08x\n", counter);

      etimer_set(&et, CLOCK_SECOND);
      rtimer_set(&rt, RTIMER_NOW() + LEDS_OFF_HYSTERISIS, 1,
                 rt_callback, NULL);
    } else if(ev == sensors_event) {
      if(data == &button_user_sensor) {
        packetbuf_copyfrom(&counter, sizeof(counter));
        broadcast_send(&bc);
      }
    } else if(ev == serial_line_event_message) {
      leds_toggle(LEDS_SERIAL_IN);
    }
    counter++;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
