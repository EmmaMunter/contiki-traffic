/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimemh Best-effort multihop forwarding
 * @{
 *
 * The mh module implements a multihop forwarding mechanism. Routes
 * must have already been setup with the route_add() function. Setting
 * up routes is done with another Rime module such as the \ref
 * routediscovery "route-discovery module".
 *
 * \section channels Channels
 *
 * The mh module uses 1 channel.
 *
 */

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
 * $Id: mh.h,v 1.6 2008/02/24 22:05:27 adamdunkels Exp $
 */

/**
 * \file
 *         Multihop forwarding header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __MH_H__
#define __MH_H__

#include "net/rime/uc.h"
#include "net/rime/rimeaddr.h"

struct mh_conn;

struct mh_callbacks {
  void (* recv)(struct mh_conn *ptr,
		rimeaddr_t *sender,
		rimeaddr_t *prevhop,
		uint8_t hops);
  rimeaddr_t *(* forward)(struct mh_conn *ptr,
			  rimeaddr_t *originator,
			  rimeaddr_t *dest,
			  rimeaddr_t *prevhop,
			  uint8_t hops);
};

struct mh_conn {
  struct uc_conn c;
  const struct mh_callbacks *cb;
};

void mh_open(struct mh_conn *c, uint16_t channel,
	     const struct mh_callbacks *u);
void mh_close(struct mh_conn *c);
int mh_send(struct mh_conn *c, rimeaddr_t *to);

#endif /* __MH_H__ */
/** @} */
/** @} */
