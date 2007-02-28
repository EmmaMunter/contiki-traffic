/**
 * \addtogroup rime
 * @{
*/

/**
 * \defgroup rime-sabc Stubborn Anonymous best-effort local area BroadCast
 * @{
 *
 * The sabc module provides stubborn anonymous best-effort local area
 * broadcast. A message sent with the sabc module is repeated until
 * either the mssage is canceled or a new message is sent. Messages
 * sent with the sabc module are not identified with a sender ID.
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
 * $Id: sabc.h,v 1.1 2007/02/28 16:38:52 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the Rime module Stubborn Anonymous BroadCast (sabc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __SABC_H__
#define __SABC_H__

#include "net/rime/uc.h"
#include "net/rime/ctimer.h"
#include "net/rime/queuebuf.h"

struct sabc_conn;

struct sabc_ulayer {
  void (* recv)(struct sabc_conn *c);
  void (* sent)(struct sabc_conn *c);
};

/**
 * A sabc connection. This is an opaque structure with no user-visible
 * fields. The sabc_setup() function is used for setting up a sabc
 * connection.
 */
struct sabc_conn {
  struct abc_conn c;
  struct ctimer t;
  struct queuebuf *buf;
  const struct sabc_ulayer *u;
};


/**
 * \brief      Set up a sabc connection.
 * \param c    A pointer to a user-supplied struct sabc variable.
 * \param channel The Rime channel on which messages should be sent.
 * \param u    Pointer to the upper layer functions that should be used
 *             for this connection.
 *
 *             This function sets up a sabc connection on the
 *             specified channel. No checks are made if the channel is
 *             currently used by another connection.
 *
 *             This function must be called before any other function
 *             that operates on the connection is called.
 *
 */
void sabc_setup(struct sabc_conn *c, u16_t channel,
		const struct sabc_ulayer *u);


/**
 * \brief      Send a stubborn message.
 * \param c    A sabc connection that must have been previously set up
 *             with sabc_setup()
 * \param t    The time between message retransmissions.
 *
 *             This function sends a message from the Rime buffer. The
 *             message must have been previously constructed in the
 *             Rime buffer. When this function returns, the message
 *             has been copied into a queue buffer.
 *
 *             If another message has previously been sent, the old
 *             message is canceled.
 *
 */
int sabc_send_stubborn(struct sabc_conn *c, clock_time_t t);


/**
 * \brief      Cancel the current stubborn message.
 * \param c    A sabc connection that must have been previously set up
 *             with sabc_setup()
 *
 *             This function cancels a stubborn message that has
 *             previously been sent with the sabc_send_stubborn()
 *             function.
 *
 */
void sabc_cancel(struct sabc_conn *c);



/**
 * \brief      Set the retransmission time of the current stubborn message.
 * \param c    A sabc connection that must have been previously set up
 *             with sabc_setup()
 * \param t    The new time between message retransmissions.
 *
 *             This function sets the retransmission timer for the
 *             current stubborn message to a new value.
 *
 */
void sabc_set_timer(struct sabc_conn *c, clock_time_t t);

#endif /* __SABC_H__ */

/** @} */
