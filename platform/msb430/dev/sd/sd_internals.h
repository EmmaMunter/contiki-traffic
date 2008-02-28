
/*
Copyright 2007, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universitšt Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2007
*/

/**
 * @file	ScatterWeb.sd.internals.h
 * @ingroup	libsd
 * @brief	MMC-/SD-Card library
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @date	Jan 2007
 * @version	0.2
 * 
 * Header file containing private declarations used MMC-/SD-Card library.
 */

#ifndef SD_INTERNALS_H
#define SD_INTERNALS_H

#include <string.h>
#include "contiki-msb430.h"

#define RETF(x)		if( ! x ) return false;
#define	MIN(x, y)	( x < y ) ? x : y

/**
 * @name SD Card SPI responses
 */

struct sd_response_r1_bits {
  uint8_t in_idle_state:1;
  uint8_t erase_reset:1;
  uint8_t illegal_cmd:1;
  uint8_t crc_err:1;
  uint8_t erase_seq_err:1;
  uint8_t address_err:1;
  uint8_t param_err:1;
  uint8_t start_bit:1;
};

struct sd_response_r2_bits {
  uint8_t card_locked:1;
  uint8_t write_failed:1;
  uint8_t unspecified_err:1;
  uint8_t controller_err:1;
  uint8_t ecc_failed:1;
  uint8_t protect_violation:1;
  uint8_t erase_param:1;
  uint8_t out_of_range:1;
};

struct sd_read_error_token {
  uint8_t unspecified_err:1;
  uint8_t controller_err:1;
  uint8_t ecc_failed:1;
  uint8_t out_of_range:1;
  uint8_t card_locked:1;
    uint8_t:3;
};

struct sd_data_error_token {
  uint8_t error:1;
  uint8_t cc_error:1;
  uint8_t ecc_error:1;
  uint8_t out_of_range:1;
  uint8_t:4;			// always 0 (-> SD_DATA_ERROR_TOKEN_MASK)
};

typedef struct sd_response_r1 {
  struct sd_response_r1_bits r1;
} sd_response_r1_t;

typedef struct sd_response_r2 {
  struct sd_response_r1_bits r1;
  struct sd_response_r2_bits r2;
} sd_response_r2_t;

typedef struct sd_response_r3 {
  uint32_t ocr;
  struct sd_response_r1_bits r1;
} sd_response_r3_t;

typedef struct sd_read_error_token sd_read_error_token_t;
typedef struct sd_data_error_token sd_data_error_t;

#define SD_DATA_ERROR_TOKEN_MASK	0x0F	// mask for error token
#define SD_R1_ERROR_MASK		0x7C	// mask for error bits in r1 response
///@}

/**
 * @name Private interface
 */

// Read operating condition
bool sd_get_op_cond(sd_response_r1_t * pResponse);

// Reset card
bool sd_reset(void);

// Send command and read response
bool sd_send_cmd(const uint8_t command, const uint8_t response_type,
		 const void *pArg, void (*const pResponse));

// Select card
void sd_select(void);

// Unselect card
void sd_unselect(void);

// Wait for card to leave idle mode
bool sd_wait_standby(void);

// Read card register
uint16_t sd_read_register(void (*const pBuffer), const uint8_t cmd,
			const uint16_t size);

// Begin block read operation
bool sd_read_start(const uint8_t cmd, const uint32_t address);

// Wait for begin of data
bool sd_read_wait(void);

// Cancel block read operation
void sd_read_stop(const uint16_t count);

#if SD_WRITE
uint16_t sd_write_block_x(const uint32_t * pAddress, const void * pBuffer,
		      bool incPtr);

#endif /*  */

#if SD_CACHE
void sd_cache_init(void);
void sd_cache_flush(void);

#endif /*  */
///@}

#define SD_TOKEN_READ		0xFE
#define SD_TOKEN_WRITE		0xFE
#define SD_TOKEN_ZP		0xFF

#define SD_TIMEOUT_IDLE		1000	// # of poll-cycles for reset procedure (takes some time)
#define SD_TIMEOUT_READ		20000
#define SD_TIMEOUT_NCR		8	// 8-64 cycles

#define	SD_V_MASK		0x003E0000	// 3,4 - 2,9 V

#define SD_RESPONSE_TYPE_R1	1
#define SD_RESPONSE_TYPE_R2	2
#define SD_RESPONSE_TYPE_R3	5

/**
 * @name Command classes
 * @{
 */
#define SD_CCC_BASIC		BIT0
#define SD_CCC_BLOCK_READ	BIT2
#define	SD_CCC_BLOCK_WRITE	BIT4
#define	SD_CCC_APP_SPECIFIC	BIT8

#if SD_WRITE
#define SD_DEFAULT_MINCCC	(SD_CCC_BASIC |			\
  SD_CCC_BLOCK_READ | SD_CCC_APP_SPECIFIC | SD_CCC_BLOCK_WRITE)
#else /*  */
#define SD_DEFAULT_MINCCC	(SD_CCC_BASIC |			\
  SD_CCC_BLOCK_READ | SD_CCC_APP_SPECIFIC)
#endif /*  */
//@}

/**
 * @name Commands
 * @{
 */
#define SD_CMD_GO_IDLE_STATE		 0	// R1 "reset command"
#define SD_CMD_SEND_OP_COND		 1	// R1 (MMC only!)
#define SD_CMD_SEND_CSD			 9	// R1
#define SD_CMD_SEND_CID			10	// R1
#define SD_CMD_STOP_TRANSMISSION	12	// R1b
#define SD_CMD_SEND_STATUS		13	// R2
#define SD_CMD_SET_BLOCKLENGTH		16
#define SD_CMD_READ_SINGLE_BLOCK	17	// R1
#define SD_CMD_READ_MULTI_BLOCK		18	// R1
#define SD_CMD_WRITE_SINGLE_BLOCK	24	// R1
#define SD_CMD_ERASE_WR_BLK_START_ADDR	32	// R1
#define SD_CMD_ERASE_WR_BLK_END_ADDR	33	// R1
#define SD_CMD_ERASE			38	// R1b
#define SD_CMD_APP_SECIFIC_CMD		55
#define SD_CMD_READ_OCR			58	// R3 OCR = voltage table

#define SD_ACMD_SEND_OP_COND		41	// R1
//@}

#endif /* !SD_INTERNALS_H */
