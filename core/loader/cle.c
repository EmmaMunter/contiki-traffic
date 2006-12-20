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
 * @(#)$Id: cle.c,v 1.2 2006/12/20 13:43:47 bg- Exp $
 */

/*
 * The Contiki dynamic Link Editor (CLE), ELF version.
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"

#include "loader/cle.h"
#include "loader/elf32.h"
#include "loader/symtab.h"

#define NDEBUG
#include "lib/assert.h"

#ifdef NDEBUG
#define PRINTF(...) do {} while (0)
#else
#define PRINTF(...) printf(__VA_ARGS__)
#endif

/*
 * Parse object file located at offset start reading data using function
 * pread. Save what is useful in info.
 */
int
cle_read_info(struct cle_info *info,
	      int (*pread)(void *, int, off_t),
	      off_t start)
{
  /*
   * Save stackspace by using a union!
   *
   * Beware that the contents of ehdr is gone when shdr is written!!!
   */
  union {
    struct elf32_ehdr ehdr;
    struct elf32_shdr shdr;
  } huge;
#define ehdr huge.ehdr
#define shdr huge.shdr

  off_t shdrptr; 
  cle_off strs;
  cle_half shnum;		/* number shdrs */
  cle_half shentsize;		/* sizeof shdr */
  cle_word strtabsize = 0;
  int i, ret;

  memset(info, 0x0, sizeof(*info));

  ret = pread(&ehdr, sizeof(ehdr), start);
  assert(ret > 0);

  /* Make sure that we have a correct and compatible ELF header. */
  if(memcmp(ehdr.e_ident, ELF_MAGIC_HEADER, ELF_MAGIC_HEADER_SIZE) != 0) {
    return CLE_BAD_HEADER;
  }

  shdrptr = ehdr.e_shoff;
  shentsize = ehdr.e_shentsize;
  shnum = ehdr.e_shnum;

  /* The string table section: holds the names of the sections. */
  ret = pread(&shdr, sizeof(shdr), start + shdrptr + shentsize*ehdr.e_shstrndx);
  assert(ret > 0);

  /* BEWARE THAT ehdr IS NOW OVERWRITTEN!!! */

  /*
   * Get a pointer to the actual table of strings. This table holds
   * the names of the sections, not the names of other symbols in the
   * file (these are in the symtab section).
   */
  strs = shdr.sh_offset;

  /*
   * The ".text" segment holds the actual code from the ELF file, the
   * ".data" segment contains initialized data, the ".bss" segment
   * holds the size of the unitialized data segment. The ".rela.text"
   * and ".rela.data" segments contains relocation information for the
   * contents of the ".text" and ".data" segments, respectively. The
   * ".symtab" segment contains the symbol table for this file. The
   * ".strtab" segment points to the actual string names used by the
   * symbol table.
   *
   * In addition to grabbing pointers to the relevant sections, we
   * also save the section index for resolving addresses in the
   * relocator code.
   */
  for(i = 0; i < shnum; ++i) {
    ret = pread(&shdr, sizeof(shdr), shdrptr);
    assert(ret > 0);
    
    /* The name of the section is contained in the strings table. */
    ret = pread(info->name, sizeof(info->name), start + strs + shdr.sh_name);
    assert(ret > 0);

    if(strncmp(info->name, ".text", 5) == 0) {
      info->textoff = shdr.sh_offset;
      info->textsize = shdr.sh_size;
      info->text_shndx = i;
    } else if(strncmp(info->name, ".rela.text", 10) == 0) {
      info->textrelaoff = shdr.sh_offset;
      info->textrelasize = shdr.sh_size;
    } else if(strncmp(info->name, ".data", 5) == 0) {
      info->dataoff = shdr.sh_offset;
      info->datasize = shdr.sh_size;
      info->data_shndx = i;
    } else if(strncmp(info->name, ".rela.data", 10) == 0) {
      info->datarelaoff = shdr.sh_offset;
      info->datarelasize = shdr.sh_size;
    } else if(strncmp(info->name, ".symtab", 7) == 0) {
      info->symtaboff = shdr.sh_offset;
      info->symtabsize = shdr.sh_size;
    } else if(strncmp(info->name, ".strtab", 7) == 0) {
      info->strtaboff = shdr.sh_offset;
      strtabsize = shdr.sh_size;
    } else if(strncmp(info->name, ".bss", 4) == 0) {
      info->bsssize = shdr.sh_size;
      info->bss_shndx = i;
    } else {
      info->name[sizeof(info->name) - 1] = 0;
      PRINTF("cle: unknown section %.12s\n", info->name);
    }

    /* Move on to the next section header. */
    shdrptr += shentsize;
  }

  if(info->symtabsize == 0) {
    return CLE_NO_SYMTAB;
  }
  if(strtabsize == 0) {
    return CLE_NO_STRTAB;
  }
  if(info->textsize == 0) {
    return CLE_NO_TEXT;
  }

  return CLE_OK;
}

/*
 * Update one reloc.
 *
 * Writing relocs is machine dependent and this function is MSP430
 * specific!
 */
static inline int
cle_upd_reloc(cle_addr segmem, struct elf32_rela *rela, cle_addr addr)
{
  memcpy(segmem + rela->r_offset, &addr, 2); /* Write reloc */
  return 0;
}

/*
 * Relocate one segment that has been copied to the location pointed
 * to by segmem.
 *
 * Relocation info is read from offset reloff to (reloff + relsize)
 * and the start of the object file is at start. Data is read using
 * function pread.
 */
int
cle_relocate(struct cle_info *info,
	     int (*pread)(void *, int, off_t),
	     off_t start,	/* Offset to start of file. */
	     cle_addr segmem,	/* Where segment is stored in memory. */
	     cle_off reloff,	/* .rela.<segment> start */
	     cle_word relsize)	/* .rela.<segment> size */
{
  struct elf32_rela rela;
  struct elf32_sym s;
  off_t off;
  cle_addr addr;
  int ret;
  
  for(off = start + reloff;
      off < start + reloff + relsize;
      off += sizeof(struct elf32_rela)) {
    ret = pread(&rela, sizeof(rela), off);
    assert(ret > 0);
    ret = pread(&s, sizeof(s),
	       start + info->symtaboff
	       + sizeof(struct elf32_sym)*ELF32_R_SYM(rela.r_info));
    assert(ret > 0);

    if(s.st_shndx == info->bss_shndx) {
      addr = info->bss;
    } else if(s.st_shndx == info->data_shndx) {
      addr = info->data;
    } else if(s.st_shndx == info->text_shndx) {
      addr = info->text;
    } else {
      addr = NULL;
    }

    if(s.st_name == 0) {	/* No name, local symbol? */
      if(addr == NULL) {
	return CLE_UNKNOWN_SEGMENT;
      }
    } else {
      ret = pread(info->name, sizeof(info->name),
		  start + info->strtaboff + s.st_name);
      assert(ret > 0);
      cle_addr sym = symtab_lookup(info->name);

      if(addr == NULL && sym != NULL) { /* Imported symbol. */
	addr = sym;
      } else if(addr != NULL && sym == NULL) { /* Exported symbol. */
	addr = addr + s.st_value;
      } else if(addr == NULL && sym == NULL) {
	PRINTF("cle: undefined reference to %.32s (%d)\n",
	       info->name, s.st_info);
	return CLE_UNDEFINED;	/* Or COMMON symbol. */
      } else if(addr != NULL && sym != NULL) {
	PRINTF("cle: multiple definitions of %.32s (%d)\n",
	       info->name, s.st_info);
	return CLE_MULTIPLY_DEFINED;
      }
    }

    addr += rela.r_addend;

    cle_upd_reloc(segmem, &rela, addr);
  }
  return CLE_OK;
}

/*
 * Search object file located at offset start using function
 * pread. Search for symbol named symbol and return its address after
 * relocation or NULL on failure.
 */
void *
cle_lookup(struct cle_info *info,
	   int (*pread)(void *, int, off_t),
	   off_t start,		/* Offset to start of file. */
	   const char *symbol)

{
  struct elf32_sym s;
  off_t a;
  cle_addr addr;
  int ret;

  for(a = start + info->symtaboff;
      a < start + info->symtaboff + info->symtabsize;
      a += sizeof(s)) {
    ret = pread(&s, sizeof(s), a);
    assert(ret > 0);

    if(s.st_name != 0) {
      ret = pread(info->name, sizeof(info->name),
		 start + info->strtaboff + s.st_name);
      assert(ret > 0);

      if(strcmp(info->name, symbol) == 0) { /* Exported symbol found. */
	if(s.st_shndx == info->bss_shndx) {
	  addr = info->bss;
	} else if(s.st_shndx == info->data_shndx) {
	  addr = info->data;
	} else if(s.st_shndx == info->text_shndx) {
	  addr = info->text;
	} else {
	  return NULL;		/* Really an error! */
	}
	return addr + s.st_value;
      }
    }
  }
  return NULL;
}
