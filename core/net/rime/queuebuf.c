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
 * $Id: queuebuf.c,v 1.1 2007/02/28 16:38:51 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/queuebuf.h"

#define QUEUEBUF_NUM 2
#define QUEUEBUF_REF_NUM 2

struct queuebuf {
  u16_t len;
  u8_t data[NETBUF_SIZE + NETBUF_HDR_SIZE];
};

struct queuebuf_ref {
  u16_t len;
  u8_t *ref;
  u8_t hdr[NETBUF_HDR_SIZE];
  u8_t hdrlen;
};

MEMB(bufmem, struct queuebuf, QUEUEBUF_NUM);
MEMB(refbufmem, struct queuebuf_ref, QUEUEBUF_REF_NUM);
/*---------------------------------------------------------------------------*/
void
queuebuf_init(void)
{
  memb_init(&bufmem);
  memb_init(&refbufmem);
}
/*---------------------------------------------------------------------------*/
struct queuebuf *
queuebuf_new_from_rimebuf(void)
{
  struct queuebuf *buf;
  struct queuebuf_ref *rbuf;

  if(rimebuf_is_reference()) {
    rbuf = memb_alloc(&refbufmem);
    if(rbuf != NULL) {
      rbuf->len = rimebuf_len();
      rbuf->ref = rimebuf_reference_ptr();
      rbuf->hdrlen = rimebuf_copyto_hdr(rbuf->hdr);
    }
    return (struct queuebuf *)rbuf;
  } else {
    buf = memb_alloc(&bufmem);
    if(buf != NULL) {
      buf->len = rimebuf_copyto(buf->data);
    }
    return buf;
  }
}
/*---------------------------------------------------------------------------*/
void
queuebuf_free(struct queuebuf *buf)
{
  if(memb_inmemb(&bufmem, buf)) {
    memb_free(&bufmem, buf);
  } else if(memb_inmemb(&refbufmem, buf)) {
    memb_free(&refbufmem, buf);
  }
}
/*---------------------------------------------------------------------------*/
void
queuebuf_to_rimebuf(struct queuebuf *b)
{
  struct queuebuf_ref *r;
  
  if(memb_inmemb(&bufmem, b)) {
    rimebuf_copyfrom(b->data, b->len);
  } else if(memb_inmemb(&refbufmem, b)) {
    r = (struct queuebuf_ref *)b;
    rimebuf_clear();
    rimebuf_copyfrom(r->ref, r->len);
    rimebuf_hdrextend(r->hdrlen);
    memcpy(rimebuf_hdrptr(), r->hdr, r->hdrlen);
  }
}
/*---------------------------------------------------------------------------*/
