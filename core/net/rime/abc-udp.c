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
 * $Id: abc-udp.c,v 1.2 2007/03/13 13:02:33 adamdunkels Exp $
 */

/**
 * \file
 *         A Rime driver that uses UDP local broadcasts.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime.h"

#define PORT 8096
static struct uip_udp_conn *c;
/*---------------------------------------------------------------------------*/
PROCESS(abc_udp_process, "abc_udp");
PROCESS_THREAD(abc_udp_process, ev, data)
{
  struct abc_conn *c;

  c = uip_udp_new(HTONS(PORT), NULL);
  
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event && uip_newdata());
    DEBUGF(0, "%d: abc_udp_process: new data %p\n", node_id, data);
    rimebuf_copyfrom(uip_appdata, uip_datalen());
    abc_input_packet();
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
abc_arch_send(u8_t *buf, int len)
{
  uip_udp_packet_send(c, uip_buf, uip_len);
  return 1;
}
/*---------------------------------------------------------------------------*/
