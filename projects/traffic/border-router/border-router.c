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
 *         RPL border router, sink of all random UDP traffic generated.
 *         Example of traffic app.
 *
 * \author Georgios Exarchakos <g.exarchakos@tue.nl>
 */

#include "contiki.h"
#include "dev/slip.h"
#include "net/rpl/rpl.h"
#include "traffic.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

// needed for checking interpacket arrival times
// uint previous_arrival_time = 0;
// uint previous_arrival_time_old = 0;

PROCESS(border_router_process, "Border router process");

AUTOSTART_PROCESSES(&border_router_process);

/*---------------------------------------------------------------------------*/
void
request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
/*  uip_buf[2] = '\n'; */
  uip_len = 2;
  slip_send();
  uip_len = 0;
}
/*---------------------------------------------------------------------------*/
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  memcpy(&prefix, prefix_64, 16);
  prefix_set = 1;
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(border_router_process, ev, data)
{
//  static struct etimer et;

  PROCESS_BEGIN();

/* While waiting for the prefix to be sent through the SLIP connection, the future
 * border router can join an existing DAG as a parent or child, or acquire a default
 * router that will later take precedence over the SLIP fallback interface.
 * Prevent that by turning the radio off until we are initialized as a DAG root.
 */
//  prefix_set = 0;

  PROCESS_PAUSE();

  printf("RPL-Border router started\n");

  /* Request prefix until it has been received */
//  while(!prefix_set) {
//    etimer_set(&et, CLOCK_SECOND);
//    request_prefix();
//    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
//    printf("Waiting for prefix\n");
//  }

  prefix_set = 1;
  uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  printf("Obtained prefix: ");
  uip_debug_ipaddr_print(&prefix);
  printf("\n");
  
  uip_ipaddr_t global_ipaddr;

  NETSTACK_RDC.off(1);
  memcpy(&global_ipaddr, &prefix, 16);
  uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
  rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
  rpl_set_prefix(rpl_get_any_dag(), &prefix, 64);
  rpl_repair_root(RPL_DEFAULT_INSTANCE);
  
  NETSTACK_MAC.on();
  traffic_init();
  
  static struct etimer et_arrival_timeout;
  etimer_set(&et_arrival_timeout, 65535);
  uint16_t et_arrival_timeout_count = 0;
  while(1) {
    PROCESS_WAIT_EVENT();
	if(ev == PROCESS_EVENT_EXIT) {
		break;
	}
// 	if (ev == tcpip_event) {
//       if(uip_newdata()) {
// 		previous_arrival_time_old = previous_arrival_time;
// 		previous_arrival_time = clock_time();
//         ((char *)uip_appdata)[uip_datalen()] = '\0';
//         printf("TRAFFIC: <- [");
// 		uip_debug_ipaddr_print(&UIP_IP_BUF->srcipaddr);
// 		printf("]:, \"%s\", after %u + %u * 65535 second fractions\n", (char *)uip_appdata, previous_arrival_time - previous_arrival_time_old, et_arrival_timeout_count);
// 		et_arrival_timeout_count = 0;
// 		
// #ifdef TRAFFIC_RECEIVE_CALLBACK
//         TRAFFIC_RECEIVE_CALLBACK(&UIP_IP_BUF->srcipaddr, uip_ntohs(UIP_UDP_BUF->srcport), (char *)uip_appdata);
// #endif
// 		
//       }
//     }
  }
  traffic_end();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/