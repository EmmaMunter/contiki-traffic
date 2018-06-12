/*
 * Copyright (c) 2016, Georgios Exarchakos
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
 /**
 *
 * \file
 *         RPL node producing and receiving random UDP traffic
 *
 * \author Georgios Exarchakos <g.exarchakos@tue.nl>
 */

// #define TRAFFIC_ROUTING_RPL
// #define TRAFFIC_ROUTING_UAODV

#include "contiki.h"
#ifdef TRAFFIC_ROUTING_RPL 
#include "net/rpl/rpl.h"
#endif
#ifdef TRAFFIC_ROUTING_UAODV
#include "net/ipv4/uaodv.h"
#endif
#include "traffic.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

const char *sinks[TRAFFIC_DESTINATIONS_COUNT] = {
#ifdef TRAFFIC_ROUTING_RPL
  "c30c:0:0:1"
#endif
#ifdef TRAFFIC_ROUTING_UAODV
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  32,
  33,
  34,
  35,
  36,
  37,
  38,
  39,
  40,
  41,
  42,
  43,
  44,
  45,
  46,
  47,
  48,
  49,
  50
#endif
};

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");
#ifdef TRAFFIC_ROUTING_RPL
AUTOSTART_PROCESSES(&node_process);
#endif
#ifdef TRAFFIC_ROUTING_UAODV
AUTOSTART_PROCESSES(&uaodv_process, &node_process);
#endif

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_BEGIN();

  NETSTACK_MAC.on();
  
#ifdef TRAFFIC_ROUTING_RPL
  rpl_dag_t* dodag = rpl_get_any_dag();

  etimer_set(&et, CLOCK_SECOND);
  while(!dodag->joined) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    dodag = rpl_get_any_dag();
    etimer_restart(&et);
  }
#endif
#ifdef TRAFFIC_ROUTING_UAODV
  
#endif
  
  traffic_init();
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT);
  }
  traffic_end();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

int my_awesome_payload(char* buffer, int max) {
  char* temp = "i spam";
  int i;
  for(i=0; i<7; i++)
  {
    buffer[i] = temp[i];
  }
  return 6;
}