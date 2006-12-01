/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: cle.h,v 1.1 2006/12/01 15:12:27 bg- Exp $
 */

#ifndef CLE_H
#define CLE_H

/*
 * The Contiki dynamic Link Editor (CLE) for small systems.
 */

/* These typedefs limits object file size! */
typedef u16_t cle_off;		/* Offset from start of file. */
typedef u16_t cle_word;
typedef u16_t cle_half;

/* Also used for address arithmetic (can't be void *). */
typedef unsigned char *cle_addr;
/* typedef uintptr_t cle_addr; */

struct cle_info {
  cle_addr text, data, bss;

  cle_off textrelaoff, datarelaoff;
  cle_word textrelasize, datarelasize;

  cle_off textoff, dataoff;
  cle_word textsize, datasize, bsssize;

  cle_off symtaboff, strtaboff;
  cle_word symtabsize;

  unsigned char text_shndx;
  unsigned char data_shndx;
  unsigned char bss_shndx;
  unsigned char unused_shndx;

  char name[32];		/* Scratch and errmsg buffer. */
};

int
cle_read_info(struct cle_info *info,
	      int (*read)(void *, int, off_t),
	      off_t hdr);	/* Offset to start of file. */

int
cle_relocate(struct cle_info *info,
	     int (*read)(void *, int, off_t),
	     off_t hdr,		/* Offset to start of file. */
	     cle_addr segmem,   /* Where segment is stored in memory. */
	     cle_off reloff,	/* .rela.<segment> start */
	     cle_word relsize);	/* .rela.<segment> size */

void *
cle_lookup(struct cle_info *info,
	   int (*read)(void *, int, off_t),
	   off_t hdr,		/* Offset to start of file. */
	   const char *symbol);

#define CLE_OK                  0
#define CLE_BAD_HEADER          1
#define CLE_NO_SYMTAB           2
#define CLE_NO_STRTAB           3
#define CLE_NO_TEXT             4
#define CLE_UNDEFINED           5
#define CLE_UNKNOWN_SEGMENT     6

#define CLE_MULTIPLY_DEFINED   20

#endif /* CLE_H */
