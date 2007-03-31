/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: ruc.c,v 1.9 2007/03/31 18:33:27 adamdunkels Exp $
 */

/**
 * \file
 *         Reliable unicast
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/ruc.h"
#include "net/rime/neighbor.h"
#include "net/rime.h"
#include <string.h>

#define TYPE_DATA 0
#define TYPE_ACK  1

struct ruc_hdr {
  u8_t type;
  u8_t seqno;
};

enum {
  STATE_READY,
  STATE_SENDING
};

static u8_t seqno;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
sent_by_suc(struct suc_conn *suc)
{
  struct ruc_conn *c = (struct ruc_conn *)suc;
  c->transmissions_left--;
  if(c->transmissions_left == 0) {
    suc_cancel(&c->c);
  }
}
/*---------------------------------------------------------------------------*/
static void
recv_from_suc(struct suc_conn *suc, rimeaddr_t *from)
{
  struct ruc_conn *c = (struct ruc_conn *)suc;
  struct ruc_hdr *hdr = rimebuf_dataptr();

  PRINTF("%d.%d: ruc: recv_from_suc from %d.%d type %d seqno %d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1],
	 hdr->type, hdr->seqno);
  
  if(hdr->type == TYPE_ACK) {
    if(hdr->seqno == seqno) {
      PRINTF("%d.%d: ruc: ACKed %d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     hdr->seqno);
      ++seqno;
      suc_cancel(&c->c);
      if(c->u->sent != NULL) {
	c->u->sent(c);
      }
    }
  } else if(hdr->type == TYPE_DATA) {
    int send_ack = 1;
    u16_t packet_seqno;

    packet_seqno = hdr->seqno;

    rimebuf_hdrreduce(sizeof(struct ruc_hdr));
    if(c->u->recv != NULL) {
      send_ack = c->u->recv(c, from, packet_seqno);
    }
    
    if(send_ack) {
      PRINTF("%d.%d: ruc: Sending ACK to %d.%d for %d\n",
	     rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	     from->u8[0], from->u8[1],
	     packet_seqno);
      rimebuf_clear();
      rimebuf_hdralloc(sizeof(struct ruc_hdr));
      hdr = rimebuf_hdrptr();
      hdr->type = TYPE_ACK;
      hdr->seqno = packet_seqno;
      suc_send(&c->c, from);
    } else {
      PRINTF("%d.%d: Not sending ACK\n",
	     rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct suc_callbacks ruc = {recv_from_suc, sent_by_suc};
/*---------------------------------------------------------------------------*/
void
ruc_open(struct ruc_conn *c, u16_t channel,
	  const struct ruc_callbacks *u)
{
  suc_open(&c->c, channel, &ruc);
  c->u = u;
}
/*---------------------------------------------------------------------------*/
void
ruc_close(struct ruc_conn *c)
{
  suc_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
ruc_send(struct ruc_conn *c, rimeaddr_t *receiver, u8_t max_retransmissions)
{
  if(rimebuf_hdralloc(sizeof(struct ruc_hdr))) {
    struct ruc_hdr *hdr = rimebuf_hdrptr();
    hdr->type = TYPE_DATA;
    hdr->seqno = seqno;
    c->transmissions_left = max_retransmissions;
    return suc_send_stubborn(&c->c, receiver);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
