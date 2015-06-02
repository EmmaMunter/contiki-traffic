/*
 * Original file:
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-aes
 * @{
 *
 * \file
 * Implementation of the cc2538 AES driver
 */
#include "contiki.h"
#include "dev/rom-util.h"
#include "dev/aes.h"
#include "reg.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
uint8_t
aes_load_key(const void *key, uint8_t key_area)
{
  uint32_t aligned_key[4];

  if(REG(AES_CTRL_ALG_SEL) != 0x00000000) {
    return CRYPTO_RESOURCE_IN_USE;
  }

  /* The key address needs to be 4-byte aligned */
  rom_util_memcpy(aligned_key, key, sizeof(aligned_key));

  /* Workaround for AES registers not retained after PM2 */
  REG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
  REG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_DMA_IN_DONE |
                         AES_CTRL_INT_EN_RESULT_AV;

  /* Configure master control module */
  REG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_KEYSTORE;

  /* Clear any outstanding events */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  /* Configure key store module (area, size): 128-bit key size */
  REG(AES_KEY_STORE_SIZE) = (REG(AES_KEY_STORE_SIZE) &
                             ~AES_KEY_STORE_SIZE_KEY_SIZE_M) |
                            AES_KEY_STORE_SIZE_KEY_SIZE_128;

  /* Enable keys to write (e.g. Key 0) */
  REG(AES_KEY_STORE_WRITE_AREA) = 0x00000001 << key_area;

  /* Configure DMAC
   * Enable DMA channel 0 */
  REG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH_CTRL_EN;

  /* Base address of the key in ext. memory */
  REG(AES_DMAC_CH0_EXTADDR) = (uint32_t)aligned_key;

  /* Total key length in bytes (e.g. 16 for 1 x 128-bit key) */
  REG(AES_DMAC_CH0_DMALENGTH) = (REG(AES_DMAC_CH0_DMALENGTH) &
                                 ~AES_DMAC_CH_DMALENGTH_DMALEN_M) |
                                (0x10 << AES_DMAC_CH_DMALENGTH_DMALEN_S);

  /* Wait for operation to complete */
  while(!(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV));

  /* Check for absence of errors in DMA and key store */
  if(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR) {
    REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_BUS_ERR;
    /* Disable master control / DMA clock */
    REG(AES_CTRL_ALG_SEL) = 0x00000000;
    return CRYPTO_DMA_BUS_ERROR;
  }
  if(REG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_WR_ERR) {
    REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_KEY_ST_WR_ERR;
    /* Disable master control / DMA clock */
    REG(AES_CTRL_ALG_SEL) = 0x00000000;
    return AES_KEYSTORE_WRITE_ERROR;
  }

  /* Acknowledge the interrupt */
  REG(AES_CTRL_INT_CLR) = AES_CTRL_INT_CLR_DMA_IN_DONE |
                          AES_CTRL_INT_CLR_RESULT_AV;

  /* Disable master control / DMA clock */
  REG(AES_CTRL_ALG_SEL) = 0x00000000;

  /* Check status, if error return error code */
  if(!(REG(AES_KEY_STORE_WRITTEN_AREA) & (0x00000001 << key_area))) {
    return AES_KEYSTORE_WRITE_ERROR;
  }

  return CRYPTO_SUCCESS;
}

/** @} */
