/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: rudolph0.h,v 1.1 2007/03/25 11:56:59 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RUDOLPH0_H__
#define __RUDOLPH0_H__

#include "net/rime.h"
#include "net/rime/sabc.h"
#include "net/rime/uabc.h"
#include "contiki-net.h"

struct rudolph0_conn;

enum {
  RUDOLPH0_FLAG_NONE,
  RUDOLPH0_FLAG_NEWFILE,
  RUDOLPH0_FLAG_LASTCHUNK,
};

struct rudolph0_callbacks {
  void (* write_chunk)(struct rudolph0_conn *c, int offset, int flag,
		       char *data, int len);
  int (* read_chunk)(struct rudolph0_conn *c, int offset, char *to,
		     int maxsize);
};

#define RUDOLPH0_DATASIZE 64

struct rudolph0_hdr {
  u8_t type;
  u8_t version;
  u16_t chunk;
};

struct rudolph0_datapacket {
  struct rudolph0_hdr h;
  u8_t datalen;
  u8_t data[RUDOLPH0_DATASIZE];
};

struct rudolph0_conn {
  struct sabc_conn c;
  struct uabc_conn nackc;
  const struct rudolph0_callbacks *cb;
  u8_t state;
  struct rudolph0_datapacket current;
};

void rudolph0_open(struct rudolph0_conn *c, u16_t channel,
		   const struct rudolph0_callbacks *cb);
void rudolph0_close(struct rudolph0_conn *c);
void rudolph0_send(struct rudolph0_conn *c);
void rudolph0_stop(struct rudolph0_conn *c);

void rudolph0_set_version(struct rudolph0_conn *c, int version);
int rudolph0_version(struct rudolph0_conn *c);

#endif /* __RUDOLPH0_H__ */
