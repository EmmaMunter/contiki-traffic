/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: lpp.c,v 1.13 2009/03/31 14:11:25 nvt-se Exp $
 */

/**
 * \file
 *         Low power probing (R. Musaloiu-Elefteri, C. Liang,
 *         A. Terzis. Koala: Ultra-Low Power Data Retrieval in
 *         Wireless Sensor Networks, IPSN 2008)
 *
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 *
 * This is an implementation of the LPP (Low-Power Probing) MAC
 * protocol. LPP is a power-saving MAC protocol that works by sending
 * a probe packet each time the radio is turned on. If another node
 * wants to transmit a packet, it can do so after hearing the
 * probe. To send a packet, the sending node turns on its radio to
 * listen for probe packets.
 *
 */

#include "dev/leds.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "net/rime.h"
#include "net/mac/mac.h"
#include "net/mac/lpp.h"
#include "net/rime/packetbuf.h"
#include "net/rime/announcement.h"

#include <stdlib.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


struct announcement_data {
  uint16_t id;
  uint16_t value;
};

#define ANNOUNCEMENT_MSG_HEADERLEN 2
struct announcement_msg {
  uint16_t num;
  struct announcement_data data[];
};

#define LPP_PROBE_HEADERLEN 2

#define TYPE_PROBE        1
#define TYPE_DATA         2
struct lpp_hdr {
  uint16_t type;
  rimeaddr_t sender;
  rimeaddr_t receiver;
};

static const struct radio_driver *radio;
static void (* receiver_callback)(const struct mac_driver *);
static struct pt pt;
static struct ctimer timer;

static uint8_t is_listening = 0;

#define LISTEN_TIME CLOCK_SECOND / 32
#define OFF_TIME CLOCK_SECOND * 1
#define PACKET_LIFETIME (LISTEN_TIME + OFF_TIME)
#define UNICAST_TIMEOUT	2 * PACKET_LIFETIME

struct queue_list_item {
  struct queue_list_item *next;
  struct queuebuf *packet;
  struct ctimer timer;
};

#ifdef QUEUEBUF_CONF_NUM
#define MAX_QUEUED_PACKETS QUEUEBUF_CONF_NUM / 2
#else /* QUEUEBUF_CONF_NUM */
#define MAX_QUEUED_PACKETS 4
#endif /* QUEUEBUF_CONF_NUM */

LIST(queued_packets_list);
MEMB(queued_packets_memb, struct queue_list_item, MAX_QUEUED_PACKETS);

/*---------------------------------------------------------------------------*/
static void
turn_radio_on(void)
{
  radio->on();
  leds_on(LEDS_YELLOW);
}
/*---------------------------------------------------------------------------*/
static void
turn_radio_off(void)
{
  radio->off();
  leds_off(LEDS_YELLOW);
}
/*---------------------------------------------------------------------------*/
static void
remove_queued_packet(void *item)
{
  struct queue_list_item *i = item;

  ctimer_stop(&i->timer);  
  queuebuf_free(i->packet);
  list_remove(queued_packets_list, i);
  memb_free(&queued_packets_memb, i);

  /* XXX potential optimization */
  if(list_length(queued_packets_list) == 0 && is_listening == 0) {
    turn_radio_off();
  }
}
/*---------------------------------------------------------------------------*/
static void
listen_callback(int periods)
{
  is_listening = periods;
  turn_radio_on();
}
/*---------------------------------------------------------------------------*/
/**
 * Send a probe packet.
 */
static void
send_probe(void)
{
  struct lpp_hdr *hdr;
  struct announcement_msg *adata;
  struct announcement *a;

  /* Set up the probe header. */
  packetbuf_clear();
  packetbuf_set_datalen(sizeof(struct lpp_hdr));
  hdr = packetbuf_dataptr();
  hdr->type = TYPE_PROBE;
  rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
  rimeaddr_copy(&hdr->receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));


  /* Construct the announcements */
  adata = (struct announcement_msg *)((char *)hdr + sizeof(struct lpp_hdr));
  
  adata->num = 0;
  for(a = announcement_list(); a != NULL; a = a->next) {
    adata->data[adata->num].id = a->id;
    adata->data[adata->num].value = a->value;
    adata->num++;
  }

  packetbuf_set_datalen(sizeof(struct lpp_hdr) +
		      ANNOUNCEMENT_MSG_HEADERLEN +
		      sizeof(struct announcement_data) * adata->num);

  /*  PRINTF("Sending probe\n");*/
  radio->send(packetbuf_hdrptr(), packetbuf_totlen());
}
/*---------------------------------------------------------------------------*/
/**
 * Duty cycle the radio. The protothread is driven by a ctimer that is
 * initiated in the lpp_init() function.
 */
static int
dutycycle(void *ptr)
{
  struct ctimer *t = ptr;
  
  PT_BEGIN(&pt);

  while(1) {
    turn_radio_on();
    send_probe();
    ctimer_set(t, LISTEN_TIME, (void (*)(void *))dutycycle, t);
    PT_YIELD(&pt);

    if(list_length(queued_packets_list) == 0) {
      if(is_listening == 0) {
	turn_radio_off();
      /* There is a bit of randomness here right now to avoid collisions
	 due to synchronization effects. Not sure how needed it is
	 though. XXX */
	ctimer_set(t, OFF_TIME / 2 + (random_rand() % (OFF_TIME / 2)),
		   (void (*)(void *))dutycycle, t);
	PT_YIELD(&pt);
      } else {
	is_listening--;
	ctimer_set(t, OFF_TIME, (void (*)(void *))dutycycle, t);
	PT_YIELD(&pt);
      }
    } else {
     ctimer_set(t, OFF_TIME, (void (*)(void *))dutycycle, t);
     PT_YIELD(&pt);
    }
  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
/**
 *
 * Send a packet. This function builds a complete packet with an LPP
 * header and queues the packet. When a probe is heard (in the
 * read_packet() function), and the sender of the probe matches the
 * receiver of the queued packet, the queued packet is sent.
 *
 * ACK packets are treated differently from other packets: if a node
 * sends a packet that it expects to be ACKed, the sending node keeps
 * its radio on for some time after sending its packet. So we do not
 * need to wait for a probe packet: we just transmit the ACK packet
 * immediately.
 *
 */
static int
send_packet(void)
{
  struct lpp_hdr *hdr;
  clock_time_t timeout;

  packetbuf_hdralloc(sizeof(struct lpp_hdr));
  hdr = packetbuf_hdrptr();

  rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
  rimeaddr_copy(&hdr->receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  hdr->type = TYPE_DATA;

  packetbuf_compact();
  PRINTF("%d.%d: queueing packet to %d.%d, channel %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 hdr->receiver.u8[0], hdr->receiver.u8[1],
	 packetbuf_attr(PACKETBUF_ATTR_CHANNEL));

  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) == PACKETBUF_ATTR_PACKET_TYPE_ACK) {
    /* Immediately send ACKs - we're assuming that the other node is
       listening. */
    /*    printf("Immediately sending ACK\n");*/
    return radio->send(packetbuf_hdrptr(), packetbuf_totlen());
  } else {
    struct queue_list_item *i;
    i = memb_alloc(&queued_packets_memb);
    if(i != NULL) {
      i->packet = queuebuf_new_from_packetbuf();
      if(i->packet == NULL) {
	memb_free(&queued_packets_memb, i);
	return 0;
      } else {
	list_add(queued_packets_list, i);
        timeout = UNICAST_TIMEOUT;
        if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_null)) {
          timeout = PACKET_LIFETIME;
        }
	ctimer_set(&i->timer, timeout, remove_queued_packet, i);
        /* Wait for a probe packet from a neighbor */
        turn_radio_on();
      }
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Read a packet from the underlying radio driver. If the incoming
 * packet is a probe packet and the sender of the probe matches the
 * destination address of the queued packet (if any), the queued packet
 * is sent.
 */
static int
read_packet(void)
{
  int len;
  struct lpp_hdr *hdr;
  
  packetbuf_clear();
  len = radio->read(packetbuf_dataptr(), PACKETBUF_SIZE);
  if(len > 0) {
    packetbuf_set_datalen(len);
    hdr = packetbuf_dataptr();
    packetbuf_hdrreduce(sizeof(struct lpp_hdr));
    /*    PRINTF("got packet type %d\n", hdr->type);*/
    if(hdr->type == TYPE_PROBE) {
      /* Parse incoming announcements */
      struct announcement_msg *adata = packetbuf_dataptr();
      int i;
	
      /*	PRINTF("%d.%d: probe from %d.%d with %d announcements\n",
		rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		hdr->sender.u8[0], hdr->sender.u8[1], adata->num);*/
	
      for(i = 0; i < adata->num; ++i) {
	/*	  PRINTF("%d.%d: announcement %d: %d\n",
		  rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		  adata->data[i].id,
		  adata->data[i].value);*/

	announcement_heard(&hdr->sender,
			   adata->data[i].id,
			   adata->data[i].value);
      }
      
      if(list_length(queued_packets_list) > 0) {
	struct queue_list_item *i;
	for(i = list_head(queued_packets_list); i != NULL; i = i->next) {
	  struct lpp_hdr *qhdr;
	  
	  qhdr = queuebuf_dataptr(i->packet);
	  if(rimeaddr_cmp(&qhdr->receiver, &hdr->sender) ||
	     rimeaddr_cmp(&qhdr->receiver, &rimeaddr_null)) {
	    PRINTF("%d.%d: got a probe from %d.%d, sending packet to %d.%d\n",
		   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
		   hdr->sender.u8[0], hdr->sender.u8[1],
		   qhdr->receiver.u8[0], qhdr->receiver.u8[1]);
	    
	    radio->send(queuebuf_dataptr(i->packet),
			queuebuf_datalen(i->packet));
	    
	    /* If the packet was not a broadcast packet, we dequeue it
	       now. Broadcast packets should be transmitted to all
	       neighbors, and are dequeued by the dutycycling function
	       instead, after the appropriate time. */
	    if(!rimeaddr_cmp(&qhdr->receiver, &rimeaddr_null)) {
	      remove_queued_packet(i);
	    }
	    
	    
	    turn_radio_on(); /* XXX Awaiting an ACK: we should check the
				packet type of the queued packet to see
				if it is a data packet. If not, we
				should not turn the radio on. */

	    break;
	  }
	}
      }

    } else if(hdr->type == TYPE_DATA) {
      PRINTF("%d.%d: got data from %d.%d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     hdr->sender.u8[0], hdr->sender.u8[1]);

      /* XXX send probe after receiving a packet to facilitate data
        streaming. We must first copy the contents of the packetbuf into
        a queuebuf to avoid overwriting the data with the probe packet. */
      if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr)) {
        struct queuebuf *q;
        q = queuebuf_new_from_packetbuf();
        if(q != NULL) {
	  send_probe();
	  queuebuf_to_packetbuf(q);
          queuebuf_free(q);
        }
      }
    }
    len = packetbuf_datalen();
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static void
set_receive_function(void (* recv)(const struct mac_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  turn_radio_on();
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  if(keep_radio_on) {
    turn_radio_on();
  } else {
    turn_radio_off();
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static const struct mac_driver lpp_driver = {
  "LPP",
  send_packet,
  read_packet,
  set_receive_function,
  on,
  off,
};
/*---------------------------------------------------------------------------*/
static void
input_packet(const struct radio_driver *d)
{
  if(receiver_callback) {
    receiver_callback(&lpp_driver);
  }
}
/*---------------------------------------------------------------------------*/
const struct mac_driver *
lpp_init(const struct radio_driver *d)
{
  radio = d;
  radio->set_receive_function(input_packet);
  ctimer_set(&timer, LISTEN_TIME, (void (*)(void *))dutycycle, &timer);

  announcement_register_listen_callback(listen_callback);

  memb_init(&queued_packets_memb);
  list_init(queued_packets_list);
  return &lpp_driver;
}
/*---------------------------------------------------------------------------*/
