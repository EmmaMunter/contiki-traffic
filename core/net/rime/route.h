/**
 * \addtogroup rime
 * @{
 */
/**
 * \defgroup rimeroute Rime route table
 * @{
 *
 * The route module handles the route table in Rime.
 */

/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
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
 * $Id: route.h,v 1.6 2007/12/17 09:14:08 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the Rime route table
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "net/rime/rimeaddr.h"

struct route_entry {
  struct route_entry *next;
  rimeaddr_t dest;
  rimeaddr_t nexthop;
  u8_t seqno;
  u8_t hop_count;
  u8_t time;
};

void route_init(void);
int route_add(rimeaddr_t *dest, rimeaddr_t *nexthop,
	      u8_t hop_count, u8_t seqno);
struct route_entry *route_lookup(rimeaddr_t *dest);
void route_remove(struct route_entry *e);
void route_flush_all(void);
void route_set_lifetime(int seconds);

#endif /* __ROUTE_H__ */
/** @} */
/** @} */
