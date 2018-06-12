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
 * \file
 *       Implementation of a peer (client and server) of UDP random traffic
 *
 * \author Georgios Exarchakos <g.exarchakos@tue.nl>
 *
 */

#include <errno.h>

#include "contiki-conf.h"
#include "net/netstack.h"

// #define TRAFFIC_ROUTING_RPL
// #define TRAFFIC_ROUTING_UAODV

#ifdef TRAFFIC_ROUTING_RPL
#include "net/rpl/rpl-private.h"
#endif
#ifdef TRAFFIC_ROUTING_UAODV
#include "net/ipv4/uaodv-def.h"
#undef UAODV_RT_H_
#include "net/ipv4/uaodv-rt.h"
#include "net/ipv4/uaodv.h"
#endif

#include "net/ip/uip-debug.h"
#include "lib/random.h"
#include "net/ip/uiplib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/ip/uip-udp-packet.h"

#include "traffic-conf.h"
#include "traffic.h"

#include "net/ip/uip-debug.h"
#define DEBUG DEBUG_FULL

#if defined TRAFFIC_TRANSMIT_PAYLOAD && defined TRAFFIC_DESTINATIONS && TRAFFIC_DESTINATIONS_COUNT
static int total_time = 0;
static uint32_t interval = 0;
// static unsigned int interval = 0; //TODO
#endif

static struct uip_udp_conn *udp_conn;

// needed for checking interpacket arrival times
uint32_t previous_arrival_time = 0;
uint32_t previous_arrival_time_old = 0;
uint32_t et_arrival_timeout_count = 0;

int
traffic_str_to_ipaddr(uip_ipaddr_t* address, char *na_inbuf, int bufsize)
{
#ifdef TRAFFIC_ROUTING_RPL
	char *prefix = NULL, *suffix = NULL;
	int preblocks = 0, postblocks = 0;
	int prefixsize = 0, suffixsize = 0;
	int i;
	for(i=0; i<bufsize; i++) {
		if(na_inbuf[i]==':' && na_inbuf[i+1]==':')
		{
			prefix = na_inbuf;
			prefixsize = i;
			if(i>0) preblocks++;
			suffix = &na_inbuf[i+2];
			suffixsize = i+2;
		}
		else if(na_inbuf[i]==':')
		{
			if(suffix)
			{
				postblocks++;
			}
			else if(i>0)
			{
				preblocks++;
			}
		}
		else if(na_inbuf[i]=='\0')
		{
			if(suffix)
				suffixsize = i - suffixsize;
			else
			{
				prefix = na_inbuf;
				prefixsize = i;
				if(i>0) preblocks++;
			}
			break;
		}
	}
	if(prefix && preblocks < 8 && !suffix) {
		suffix = prefix;
		postblocks = preblocks;
		suffixsize = prefixsize;
		prefix = NULL;
		preblocks = 0;
		prefixsize = 0;
	}

	if(preblocks + postblocks < 8)
	{
		int template_found = 0;
		uint8_t state;
		for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
			state = uip_ds6_if.addr_list[i].state;
			if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED))
			{
				uip_ipaddr_copy(address, &uip_ds6_if.addr_list[i].ipaddr);
				template_found = 1;
				break;
			}
		}
		if(!template_found)
		{
			return 0;
		}
	}
	char next_end_char = ':';
	char *end;
	unsigned val;
	char *buf_end = prefix + prefixsize + 1;
	for(i=0; i<preblocks; i++)
	{
		if(prefix >= buf_end) {
			return 0;
		}
		if(i == 7) {
			next_end_char = '\0';
		}
		val = (unsigned)strtoul(prefix, &end, 16);
		if(end != prefix && *end == next_end_char && errno != ERANGE) {
			address->u8[2 * i] = val >> 8;
			address->u8[2 * i + 1] = val;
			prefix = end + 1;
		} else {
			return 0;
		}
	}
	buf_end = suffix + suffixsize + 1;
	for(i=8-postblocks; i<8; i++)
	{
		if(suffix >= buf_end) {
			return 0;
		}
		if(i == 7) {
			next_end_char = '\0';
		}
		val = (unsigned)strtoul(suffix, &end, 16);
		if(end != suffix && *end == next_end_char && errno != ERANGE) {
			address->u8[2 * i] = val >> 8;
			address->u8[2 * i + 1] = val;
			suffix = end + 1;
		} else {
			return 0;
		}
	}
	return 1;
#endif
}

int
traffic_transmit_hello(char* buffer, int max)
{
	buffer[0] = 'h';
	buffer[1] = 'e';
	buffer[2] = 'l';
	buffer[3] = 'l';
	buffer[4] = 'o';
	return 5;
}

PROCESS(traffic_process, "Traffic Generator process");

/*unsigned int
pow(unsigned int x, unsigned int exp)
{
	result = 1;
	unsigned int i = 0;
	while(i < exp) {
		result *= x;
		i++;
	}
}*/

#define fracnegpow(a, b, c) fracpow((b), (a), (c))
unsigned int
fracpow(unsigned int num, unsigned int den, unsigned int exp)
{
	unsigned int result = 1;
	unsigned int i = 0;
	while(i < exp) {
		printf("%u * %u\n", result, num);
		result *= num;
		i++;
	}
	i = 0;
	while(i < exp - 1) {
		printf("%u / %u\n", result, den);
		result /= den;
		i++;
	}
	return result;
}

/*#define fracnegpow(a, b, c) fracpow((b), (a), (c))
#define fracpow(a, b, c) (fracpow2frac((a), (b), (c)) / (b))
#define fracnegpow2frac(a, b, c) fracpow((b), (a), (c))
unsigned int
fracpow2frac(unsigned int num, unsigned int den, unsigned int exp)
{
	unsigned int result = 1;
	unsigned int i = 0;
	while(i < exp) {
		printf("%u * %u\n", result, num);
		result *= num;
		i++;
	}
	i = 0;
	while(i < exp - 1) {
		printf("%u / %u\n", result, den);
		result /= den;
		i++;
	}
	return result;
}*/

/*unsigned int
sqrt(unsigned int num) {
    short res = 0;
    short bit = 1 << 14; // The second-to-top bit is set: 1 << 30 for 32 bits
 
    // "bit" starts at the highest power of four <= the argument.
    while (bit > num)
        bit >>= 2;
        
    while (bit != 0) {
        if (num >= res + bit) {
            num -= res + bit;
            res = (res >> 1) + bit;
        }
        else
            res >>= 1;
        bit >>= 2;
    }
    return res;
}*/

uint32_t
// unsigned int //TODO
get_interval()
{
	uint32_t delay = 0; //TODO
	uint32_t olddelay;
	uint32_t rand;
// 	unsigned int delay = 0; //TODO
// 	unsigned int olddelay;
// 	unsigned int rand;
	
// 	delay = random_rand() * 65536 + random_rand();
// 	printf("Randval: %"PRIu32"\n", delay);
// 	delay = 0;
	
#ifdef TRAFFIC_NEW_SYSTEM_DELTA
	delay += TRAFFIC_NEW_SYSTEM_DELTA_MEAN * CLOCK_SECOND;
#endif
	
#ifdef TRAFFIC_NEW_SYSTEM_UNIFORM //TODO
	rand = (random_rand() << 16) | random_rand();
#ifdef TRAFFIC_NEW_SYSTEM_PRECISE
#if TRAFFIC_NEW_SYSTEM_UNIFORM_MAX < 65536
	unsigned int limit = 65535 - (65535 % (TRAFFIC_NEW_SYSTEM_UNIFORM_MAX * CLOCK_SECOND));
#else
	unsigned int limit = 4294967295 - (4294967295 % (TRAFFIC_NEW_SYSTEM_UNIFORM_MAX * CLOCK_SECOND));
#endif
	unsigned int counter = 0;
	while(rand >= limit && counter < 50){
		rand = random_rand();
#if TRAFFIC_NEW_SYSTEM_UNIFORM_MAX > 65535
		rand |= random_rand() << 16;
#endif
		counter++;
	}
#endif
	olddelay = delay;
	delay += rand % (TRAFFIC_NEW_SYSTEM_UNIFORM_MAX * CLOCK_SECOND);
	if(olddelay > delay) {
		delay = 65535;
	}
#endif
#ifdef TRAFFIC_NEW_SYSTEM_GEOMETRIC //TODO
	rand = random_rand();
	unsigned int step = 1;
	while(step < 500 && rand < TRAFFIC_NEW_SYSTEM_GEOMETRIC_PROBABILITY) {
		rand = random_rand();
		step++;
	}
	olddelay = delay;
	delay += step * CLOCK_SECOND / TRAFFIC_NEW_SYSTEM_GEOMETRIC_DOWNSCALE;
	if(olddelay > delay) {
		delay = 65535;
	}
#endif
#ifdef TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO //TODO
	rand = random_rand();
#ifdef TRAFFIC_NEW_SYSTEM_PRECISE
	unsigned int limit = 65535 - (65535 % TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_GENERATION_MAX);
	unsigned int counter = 0;
	while(rand >= limit && counter < 50){
		rand = random_rand();
		counter++;
	}
#endif
	rand %= TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_GENERATION_MAX;
	printf("Got rand: %u\n", rand);
	rand = fracpow(TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_GENERATION_MAX, rand, TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_SHAPE);
	printf("Powerized rand: %u\n", rand);
	rand = (rand - 1) * TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_STANDARD_DEVIATION / TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_SHAPE + TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_MEAN;
	printf("Final rand: %u\n", rand);
	olddelay = delay;
	delay += rand;
	if(olddelay > delay) {
		delay = 65535;
	}
#endif
#ifdef TRAFFIC_NEW_SYSTEM_GENERALIZED_POISSON //TODO
	rand = random_rand();
#ifdef TRAFFIC_NEW_SYSTEM_PRECISE
	unsigned int limit = 65535 - (65535 % TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_GENERATION_MAX);
	unsigned int counter = 0;
	while(rand >= limit && counter < 50){
		rand = random_rand();
		counter++;
	}
#endif
	rand %= TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_GENERATION_MAX;
	printf("Got rand: %u\n", rand);
	rand = fracpow(TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_GENERATION_MAX, rand, TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_SHAPE);
	printf("Powerized rand: %u\n", rand);
	rand = (rand - 1) * TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_STANDARD_DEVIATION / TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_SHAPE + TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_MEAN;
	printf("Final rand: %u\n", rand);
	olddelay = delay;
	delay += rand;
	if(olddelay > delay) {
		delay = 65535;
	}
#endif
	
	if(delay == 0) {
		delay = 1;
	}
	
// 	printf("interval: %"PRIu32"\n", delay);
// 	printf("interval: %i%i%i%i%i%i%i%i\n", delay % 10, (delay % 100)/10, (delay % 1000)/100, (delay % 10000)/1000, (delay % 100000)/10000, (delay % 1000000)/100000, (delay % 10000000)/1000000, (delay % 100000000)/10000000);
	
	return delay;
}

void
set_destination(uip_ipaddr_t *destination)
{
#ifdef TRAFFIC_ROUTING_UAODV
	unsigned int rand;
	uip_ipaddr_t hostaddr;
	
	rand = random_rand() % TRAFFIC_DESTINATIONS_COUNT;
	while(TRAFFIC_DESTINATIONS[rand] == uip_hostaddr.u8[2]) {
		rand = random_rand() % TRAFFIC_DESTINATIONS_COUNT;
	}
	uip_ipaddr(destination, 172, 16, TRAFFIC_DESTINATIONS[rand], 0);
#endif
}

PROCESS_THREAD(traffic_process, ev, data)
{
  PROCESS_BEGIN();
  printf("TRAFFIC: process started\n");
  
  /* Listen to any host */
  udp_conn = udp_new(NULL, 0, NULL);
  udp_bind(udp_conn, UIP_HTONS(TRAFFIC_PORT));
  /* Wait for timer event 
     On timer event, handle next sample */
  
#if defined TRAFFIC_TRANSMIT_PAYLOAD && defined TRAFFIC_DESTINATIONS && TRAFFIC_DESTINATIONS_COUNT
  printf("Creating timer.\n");
  static struct etimer et;
  
  interval = get_interval();
  
// #ifdef TRAFFIC_SHRINK_FACTOR
//   interval = interval >> TRAFFIC_CDF_SHRINK_FACTOR;
// #endif
  
  printf("interval: %"PRIu32"\n", interval);
  etimer_set(&et, interval);
#endif
  
  static struct etimer et_arrival_timeout;
  etimer_set(&et_arrival_timeout, 20000);
  while(1) {
// 	printf("start of loop: %u\n", previous_arrival_time);
    PROCESS_WAIT_EVENT();
// 	printf("got event: %u\n", previous_arrival_time);
    if (ev == tcpip_event) {
      if(uip_newdata()) {
// 		printf("oldnew: %u\n", previous_arrival_time);
		previous_arrival_time_old = previous_arrival_time;
		previous_arrival_time = clock_time();
// 		printf("%u\n", previous_arrival_time);
// 		printf("%u\n", previous_arrival_time_old);
// 		printf("%u\n", previous_arrival_time - previous_arrival_time_old);
        ((char *)uip_appdata)[uip_datalen()] = '\0';
        printf("TRAFFIC: <- [");
		uip_debug_ipaddr_print(&UIP_IP_BUF->srcipaddr);
		printf("]:, \"%s\", after %"PRIu32" second fractions\n", (char *)uip_appdata, previous_arrival_time - previous_arrival_time_old);
		et_arrival_timeout_count = 0;
		etimer_restart(&et_arrival_timeout);
// 		printf("after: %u\n", previous_arrival_time);
		
#ifdef TRAFFIC_RECEIVE_CALLBACK
        TRAFFIC_RECEIVE_CALLBACK(&UIP_IP_BUF->srcipaddr, uip_ntohs(UIP_UDP_BUF->srcport), (char *)uip_appdata);
#endif
		
      }
    }
    
    if (etimer_expired(&et_arrival_timeout)) {
		et_arrival_timeout_count++;
		etimer_reset(&et_arrival_timeout);
	}
	
#if defined TRAFFIC_TRANSMIT_PAYLOAD && defined TRAFFIC_DESTINATIONS && TRAFFIC_DESTINATIONS_COUNT
#ifdef TRAFFIC_ROUTING_RPL
#ifdef TRAFFIC_SINGLE_BORDER_ROUTER_DESTINATION //TODO: Contiki can't get ipv6 hostaddr?
	if (etimer_expired(&et)) {
#else
	uip_ipaddr_t tmp_addr;
	uip_ipaddr_t host_addr;
#if TRAFFIC_DESTINATIONS_COUNT == 1
	traffic_str_to_ipaddr(&tmp_addr, TRAFFIC_DESTINATIONS[0], strlen(TRAFFIC_DESTINATIONS[0])+1);
	uip_gethostaddr(&host_addr);
#endif
    if (etimer_expired(&et) && (TRAFFIC_DESTINATIONS_COUNT > 1 || uip_ipaddr_cmp(&tmp_addr, &host_addr))) {
#endif
#endif
#ifdef TRAFFIC_ROUTING_UAODV
    if (etimer_expired(&et) && (TRAFFIC_DESTINATIONS_COUNT > 1 || TRAFFIC_DESTINATIONS[0] != uip_hostaddr.u8[2])) {
#endif
      total_time += interval;
      uip_ipaddr_t destination;
	  int skip = 0;
#ifdef TRAFFIC_ROUTING_RPL
	  char *addr_str = (char*)TRAFFIC_DESTINATIONS[random_rand() % TRAFFIC_DESTINATIONS_COUNT];
      traffic_str_to_ipaddr(&destination,addr_str,strlen(addr_str)+1);
	  int i;
      uint8_t state;
      for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
        state = uip_ds6_if.addr_list[i].state;
        if((uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED))
			&& (uip_ipaddr_cmp(&uip_ds6_if.addr_list[i].ipaddr,&destination)))
        {
			skip = 1;
			break;
		}
	  }  
#endif
#ifdef TRAFFIC_ROUTING_UAODV
      struct uaodv_rt_entry *route;
	  

//       uip_ipaddr(&destination, 172, 16, 1, 0);
	  set_destination(&destination);
      route = uaodv_rt_lookup_any(&destination);
      if (route == NULL || route->is_bad) {
        printf("Getting route from %d.%d.%d.%d to %d.%d.%d.%d\n", uip_ipaddr_to_quad(&uip_hostaddr), uip_ipaddr_to_quad(&destination));
        uaodv_request_route_to(&destination);
		//route = uaodv_rt_lookup_any(&destination); // NOTE: added this line and changed from if to while loop
      }
		if (route == NULL) {
			printf("Route is null.\n");
		} else if (route->is_bad) {
			printf("Route is bad.\n");
		}
#endif
	  if(!skip)
      {
        char buffer[UIP_APPDATA_SIZE];
		int siz = TRAFFIC_TRANSMIT_PAYLOAD(buffer, UIP_APPDATA_SIZE);
        printf("TRAFFIC: -> [");
		uip_debug_ipaddr_print(&destination);
		printf("]:%u, \"%s\" //after delay of %"PRIu32"\n",TRAFFIC_PORT, buffer, interval);
		uip_udp_packet_sendto(udp_conn, buffer, siz, &destination, UIP_HTONS(TRAFFIC_PORT));
      }
      
      interval = get_interval();
// #ifdef TRAFFIC_SHRINK_FACTOR
//       interval = interval >> TRAFFIC_CDF_SHRINK_FACTOR;
// #endif
	  
	  printf("interval: %"PRIu32"\n", interval);
      etimer_reset_with_new_interval(&et, interval);
    }
#endif

// 	printf("end of loop: %u\n", previous_arrival_time);
  }
  PROCESS_END();
  printf("TRAFFIC: process ended\n");
}

void
traffic_init()
{
  process_start(&traffic_process, NULL);
}

void
traffic_end()
{
  process_exit(&traffic_process);
}