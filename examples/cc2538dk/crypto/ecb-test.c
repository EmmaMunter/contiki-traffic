/*
 * Copyright (c) 2015, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
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
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-ecb-test cc2538dk AES-ECB Test Project
 *
 *   AES-ECB access example for CC2538 on SmartRF06EB.
 *
 *   This example shows how AES-ECB should be used. The example also verifies
 *   the AES-ECB functionality.
 *
 * @{
 *
 * \file
 *     Example demonstrating AES-ECB on the cc2538dk platform
 */
#include "contiki.h"
#include "sys/rtimer.h"
#include "dev/rom-util.h"
#include "dev/ecb.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define MDATA_MAX_LEN   160
/*---------------------------------------------------------------------------*/
PROCESS(ecb_test_process, "ecb test process");
AUTOSTART_PROCESSES(&ecb_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ecb_test_process, ev, data)
{
  static const char *const str_res[] = {
    "success",
    "invalid param",
    "NULL error",
    "resource in use",
    "DMA bus error",
    "keystore read error",
    "keystore write error",
    "authentication failed"
  };
  static const uint8_t keys128[][128 / 8] = {
    { 0xed, 0xfd, 0xb2, 0x57, 0xcb, 0x37, 0xcd, 0xf1,
      0x82, 0xc5, 0x45, 0x5b, 0x0c, 0x0e, 0xfe, 0xbb },
    { 0xef, 0x60, 0xfb, 0x14, 0x00, 0xc8, 0x39, 0x36,
      0x41, 0x4a, 0x25, 0x65, 0x1e, 0xb5, 0x1a, 0x1b },
    { 0x00, 0xcc, 0x73, 0xc9, 0x90, 0xd3, 0x76, 0xb8,
      0x22, 0x46, 0xe4, 0x5e, 0xa3, 0xae, 0x2e, 0x37 },
    { 0xeb, 0xea, 0x9c, 0x6a, 0x82, 0x21, 0x3a, 0x00,
      0xac, 0x1d, 0x22, 0xfa, 0xea, 0x22, 0x11, 0x6f },
    { 0x54, 0xb7, 0x60, 0xdd, 0x29, 0x68, 0xf0, 0x79,
      0xac, 0x1d, 0x5d, 0xd2, 0x06, 0x26, 0x44, 0x5d },
    { 0x9b, 0xa8, 0x52, 0x52, 0x0c, 0x9f, 0xd1, 0xeb,
      0x36, 0x7b, 0x6a, 0xd2, 0xae, 0xd0, 0x7a, 0xbd },
    { 0x8e, 0xc6, 0xa5, 0xa0, 0x54, 0xfe, 0xa2, 0xfc,
      0x8d, 0xaf, 0xb5, 0x93, 0x9a, 0x4b, 0xd7, 0x88 },
    { 0x44, 0xf0, 0xee, 0x62, 0x6d, 0x04, 0x46, 0xe0,
      0xa3, 0x92, 0x4c, 0xfb, 0x07, 0x89, 0x44, 0xbb }
  };
  static const uint8_t keys192[][192 / 8] = {
    { 0x61, 0x39, 0x6c, 0x53, 0x0c, 0xc1, 0x74, 0x9a,
      0x5b, 0xab, 0x6f, 0xbc, 0xf9, 0x06, 0xfe, 0x67,
      0x2d, 0x0c, 0x4a, 0xb2, 0x01, 0xaf, 0x45, 0x54 },
    { 0x4f, 0x41, 0xfa, 0x4d, 0x4a, 0x25, 0x10, 0x0b,
      0x58, 0x65, 0x51, 0x82, 0x83, 0x73, 0xbc, 0xca,
      0x55, 0x40, 0xc6, 0x8e, 0x9b, 0xf8, 0x45, 0x62 },
    { 0xf2, 0xd2, 0xb8, 0x22, 0x80, 0xc2, 0x59, 0x2e,
      0xcf, 0xbc, 0xf5, 0x00, 0xae, 0x64, 0x70, 0x78,
      0xc9, 0xc5, 0x76, 0x24, 0xcd, 0xe9, 0xbf, 0x6c },
    { 0x9c, 0xc2, 0x4e, 0xa1, 0xf1, 0x95, 0x9d, 0x9a,
      0x97, 0x2e, 0x71, 0x82, 0xef, 0x3b, 0x4e, 0x22,
      0xa9, 0x7a, 0x87, 0xd0, 0xda, 0x7f, 0xf6, 0x4b }
  };
  static const uint8_t keys256[][256 / 8] = {
    { 0xcc, 0x22, 0xda, 0x78, 0x7f, 0x37, 0x57, 0x11,
      0xc7, 0x63, 0x02, 0xbe, 0xf0, 0x97, 0x9d, 0x8e,
      0xdd, 0xf8, 0x42, 0x82, 0x9c, 0x2b, 0x99, 0xef,
      0x3d, 0xd0, 0x4e, 0x23, 0xe5, 0x4c, 0xc2, 0x4b },
    { 0x44, 0xa2, 0xb5, 0xa7, 0x45, 0x3e, 0x49, 0xf3,
      0x82, 0x61, 0x90, 0x4f, 0x21, 0xac, 0x79, 0x76,
      0x41, 0xd1, 0xbc, 0xd8, 0xdd, 0xed, 0xd2, 0x93,
      0xf3, 0x19, 0x44, 0x9f, 0xe6, 0x3b, 0x29, 0x48 },
    { 0xa8, 0x1f, 0xd6, 0xca, 0x56, 0x68, 0x3d, 0x0f,
      0x54, 0x45, 0x65, 0x9d, 0xde, 0x4d, 0x99, 0x5d,
      0xc6, 0x5f, 0x4b, 0xce, 0x20, 0x89, 0x63, 0x05,
      0x3e, 0x28, 0xd7, 0xf2, 0xdf, 0x51, 0x7c, 0xe4 },
    { 0xc4, 0xa7, 0x1e, 0x05, 0x5a, 0x72, 0x54, 0xdd,
      0xa3, 0x60, 0x69, 0x3f, 0xe1, 0xbe, 0x49, 0xf1,
      0x0f, 0xaa, 0x67, 0x31, 0xc3, 0x6d, 0xba, 0xa6,
      0x59, 0x0b, 0x05, 0x97, 0x4e, 0x18, 0x5c, 0x5b }
  };
  static const struct {
    const void *keys;
    uint8_t key_size;
    uint8_t count;
  } keys[] = {
    { keys128, AES_KEY_STORE_SIZE_KEY_SIZE_128,
      sizeof(keys128) / sizeof(keys128[0]) },
    { keys192, AES_KEY_STORE_SIZE_KEY_SIZE_192,
      sizeof(keys192) / sizeof(keys192[0]) },
    { keys256, AES_KEY_STORE_SIZE_KEY_SIZE_256,
      sizeof(keys256) / sizeof(keys256[0]) }
  };
  static const struct {
    bool encrypt;
    uint8_t key_size_index;
    uint8_t key_area;
    uint8_t mdata[MDATA_MAX_LEN];
    uint16_t mdata_len;
    uint8_t expected[MDATA_MAX_LEN];
  } vectors[] = {
    {
      true, /* encrypt */
      0, /* key_size_index */
      0, /* key_area */
      { 0x16, 0x95, 0xfe, 0x47, 0x54, 0x21, 0xca, 0xce,
        0x35, 0x57, 0xda, 0xca, 0x01, 0xf4, 0x45, 0xff }, /* mdata */
      16, /* mdata_len */
      { 0x78, 0x88, 0xbe, 0xae, 0x6e, 0x7a, 0x42, 0x63,
        0x32, 0xa7, 0xea, 0xa2, 0xf8, 0x08, 0xe6, 0x37 } /* expected */
    }, {
      true, /* encrypt */
      0, /* key_size_index */
      1, /* key_area */
      { 0x59, 0x35, 0x59, 0x31, 0x8c, 0xc6, 0x6b, 0xf6,
        0x95, 0xe4, 0x9f, 0xeb, 0x42, 0x79, 0x4b, 0xdf,
        0xb6, 0x6b, 0xce, 0x89, 0x5e, 0xc2, 0x22, 0xca,
        0x26, 0x09, 0xb1, 0x33, 0xec, 0xf6, 0x6a, 0xc7,
        0x34, 0x4d, 0x13, 0x02, 0x1e, 0x01, 0xe1, 0x1a,
        0x96, 0x9c, 0x46, 0x84, 0xcb, 0xe2, 0x0a, 0xba,
        0xe2, 0xb1, 0x9d, 0x3c, 0xeb, 0x2c, 0xac, 0xd4,
        0x14, 0x19, 0xf2, 0x1f, 0x1c, 0x86, 0x51, 0x49 }, /* mdata */
      64, /* mdata_len */
      { 0x3e, 0xa6, 0xf4, 0x30, 0x52, 0x17, 0xbd, 0x47,
        0xee, 0xbe, 0x77, 0x3d, 0xa4, 0xb5, 0x78, 0x54,
        0x9c, 0xac, 0x74, 0x4c, 0x00, 0xcb, 0xd8, 0xf9,
        0xd5, 0x96, 0xd3, 0x80, 0x10, 0x30, 0x4b, 0xd8,
        0x50, 0xcc, 0x2f, 0x4b, 0x19, 0xa9, 0x1c, 0x2e,
        0x02, 0x2e, 0xab, 0xf1, 0x00, 0x26, 0x61, 0x85,
        0xca, 0x27, 0x05, 0x12, 0x78, 0x15, 0xdf, 0xd4,
        0x6e, 0xfb, 0xe4, 0xec, 0xd4, 0x6a, 0x30, 0x58 } /* expected */
    }, {
      true, /* encrypt */
      0, /* key_size_index */
      2, /* key_area */
      { 0x37, 0xa1, 0x20, 0x5e, 0xa9, 0x29, 0x35, 0x5d,
        0x2e, 0x4e, 0xe5, 0x2d, 0x5e, 0x1d, 0x9c, 0xda,
        0x27, 0x9a, 0xe0, 0x1e, 0x64, 0x02, 0x87, 0xcc,
        0xb1, 0x53, 0x27, 0x6e, 0x7e, 0x0e, 0xcf, 0x2d,
        0x63, 0x3c, 0xf4, 0xf2, 0xb3, 0xaf, 0xae, 0xcb,
        0x54, 0x8a, 0x25, 0x90, 0xce, 0x04, 0x45, 0xc6,
        0xa1, 0x68, 0xba, 0xc3, 0xdc, 0x60, 0x18, 0x13,
        0xeb, 0x74, 0x59, 0x1b, 0xb1, 0xce, 0x8d, 0xfc,
        0xd7, 0x40, 0xcd, 0xbb, 0x63, 0x88, 0x71, 0x9e,
        0x8c, 0xd2, 0x83, 0xd9, 0xcc, 0x7e, 0x73, 0x69,
        0x38, 0x24, 0x0b, 0x41, 0x0d, 0xd5, 0xa6, 0xa4,
        0x8b, 0xa4, 0x9d, 0xd2, 0x06, 0x65, 0x03, 0xe6,
        0x3a, 0xb5, 0x92, 0xff, 0xdf, 0x3b, 0xe4, 0x9e,
        0x7d, 0x2d, 0xe7, 0x4f, 0x82, 0x15, 0x8b, 0x8c }, /* mdata */
      112, /* mdata_len */
      { 0xc8, 0x8e, 0x03, 0x38, 0x3b, 0xa9, 0xda, 0x6f,
        0x98, 0x2c, 0x05, 0x7f, 0xe9, 0x2c, 0x0b, 0xb3,
        0xed, 0x5b, 0x9c, 0xd1, 0x82, 0x95, 0xa1, 0x00,
        0xe1, 0x3a, 0x4e, 0x12, 0xd4, 0x40, 0xb9, 0x19,
        0xbb, 0xb8, 0xb2, 0x21, 0xab, 0xea, 0xd3, 0x62,
        0x90, 0x2c, 0xe4, 0x4d, 0x30, 0xd0, 0xb8, 0x0e,
        0x56, 0xbe, 0xe1, 0xf6, 0x6a, 0x7d, 0x8d, 0xe0,
        0xb1, 0xe1, 0xb4, 0xdb, 0xf7, 0x6c, 0x90, 0xc1,
        0x80, 0x7a, 0x3b, 0xc5, 0xf2, 0x77, 0xe9, 0x81,
        0x4c, 0x82, 0xab, 0x12, 0x0f, 0x7e, 0x10, 0x21,
        0x7d, 0xfd, 0xf6, 0x09, 0x2c, 0xe4, 0x95, 0x8f,
        0x89, 0x06, 0xc5, 0xe3, 0x22, 0x79, 0xc6, 0x53,
        0x7d, 0xd1, 0xfb, 0xae, 0x20, 0xcb, 0x7a, 0x1d,
        0x9f, 0x89, 0xd0, 0x49, 0x0b, 0x6a, 0xef, 0xc1 } /* expected */
    }, {
      true, /* encrypt */
      0, /* key_size_index */
      3, /* key_area */
      { 0x45, 0x1f, 0x45, 0x66, 0x3b, 0x44, 0xfd, 0x00,
        0x5f, 0x3c, 0x28, 0x8a, 0xe5, 0x7b, 0x38, 0x38,
        0x83, 0xf0, 0x2d, 0x9a, 0xd3, 0xdc, 0x17, 0x15,
        0xf9, 0xe3, 0xd6, 0x94, 0x85, 0x64, 0x25, 0x7b,
        0x9b, 0x06, 0xd7, 0xdd, 0x51, 0x93, 0x5f, 0xee,
        0x58, 0x0a, 0x96, 0xbb, 0xdf, 0xef, 0xb9, 0x18,
        0xb4, 0xe6, 0xb1, 0xda, 0xac, 0x80, 0x98, 0x47,
        0x46, 0x55, 0x78, 0xcb, 0x8b, 0x53, 0x56, 0xed,
        0x38, 0x55, 0x6f, 0x80, 0x1f, 0xf7, 0xc1, 0x1e,
        0xcb, 0xa9, 0xcd, 0xd2, 0x63, 0x03, 0x9c, 0x15,
        0xd0, 0x59, 0x00, 0xfc, 0x22, 0x8e, 0x1c, 0xaf,
        0x30, 0x2d, 0x26, 0x1d, 0x7f, 0xb5, 0x6c, 0xee,
        0x66, 0x35, 0x95, 0xb9, 0x6f, 0x19, 0x2a, 0x78,
        0xff, 0x44, 0x55, 0x39, 0x3a, 0x5f, 0xe8, 0x16,
        0x21, 0x70, 0xa0, 0x66, 0xfd, 0xae, 0xac, 0x35,
        0x01, 0x94, 0x69, 0xf2, 0x2b, 0x34, 0x70, 0x68,
        0x6b, 0xce, 0xd2, 0xf0, 0x07, 0xa1, 0xa2, 0xe4,
        0x3e, 0x01, 0xb4, 0x56, 0x2c, 0xaa, 0xa5, 0x02,
        0xed, 0x54, 0x1b, 0x82, 0x05, 0x87, 0x4e, 0xc1,
        0xff, 0xb1, 0xc8, 0xb2, 0x55, 0x76, 0x69, 0x42 }, /* mdata */
      160, /* mdata_len */
      { 0x01, 0x04, 0x30, 0x53, 0xf8, 0x32, 0xef, 0x9b,
        0x91, 0x1e, 0xd3, 0x87, 0xba, 0x57, 0x74, 0x51,
        0xe3, 0x0d, 0x51, 0xd4, 0xb6, 0xb1, 0x1f, 0x31,
        0x9d, 0x4c, 0xd5, 0x39, 0xd0, 0x67, 0xb7, 0xf4,
        0xf9, 0xb4, 0xf4, 0x1f, 0x7f, 0x3d, 0x4e, 0x92,
        0x0c, 0x57, 0xcb, 0xe2, 0xb5, 0xe1, 0x88, 0x5a,
        0xa6, 0x62, 0x03, 0xae, 0x49, 0x3e, 0x93, 0xa1,
        0xdf, 0x63, 0x79, 0x3a, 0x95, 0x63, 0xc1, 0x76,
        0xbc, 0x67, 0x75, 0xdd, 0x09, 0xcc, 0x91, 0x61,
        0xe2, 0x78, 0xa0, 0x1b, 0xeb, 0x8f, 0xd8, 0xa1,
        0x92, 0x00, 0x32, 0x6b, 0xd9, 0x5a, 0xbc, 0x5f,
        0x71, 0x67, 0x68, 0xe3, 0x4f, 0x90, 0xb5, 0x05,
        0x23, 0xd3, 0x0f, 0xda, 0xbb, 0x10, 0x3a, 0x3b,
        0xc0, 0x20, 0xaf, 0xbb, 0xb0, 0xcb, 0x3b, 0xd2,
        0xad, 0x51, 0x2a, 0x6f, 0xea, 0x79, 0xf8, 0xd6,
        0x4c, 0xef, 0x34, 0x74, 0x58, 0xde, 0xc4, 0x8b,
        0xe8, 0x94, 0x51, 0xcb, 0x0b, 0x80, 0x7d, 0x73,
        0x59, 0x3f, 0x27, 0x3d, 0x9f, 0xc5, 0x21, 0xb7,
        0x89, 0xa7, 0x75, 0x24, 0x40, 0x4f, 0x43, 0xe0,
        0x0f, 0x20, 0xb3, 0xb7, 0x7b, 0x93, 0x8b, 0x1a } /* expected */
    }, {
      false, /* encrypt */
      0, /* key_size_index */
      4, /* key_area */
      { 0x06, 0x5b, 0xd5, 0xa9, 0x54, 0x0d, 0x22, 0xd5,
        0xd7, 0xb0, 0xf7, 0x5d, 0x66, 0xcb, 0x8b, 0x30 }, /* mdata */
      16, /* mdata_len */
      { 0x46, 0xf2, 0xc9, 0x89, 0x32, 0x34, 0x9c, 0x33,
        0x8e, 0x9d, 0x67, 0xf7, 0x44, 0xa1, 0xc9, 0x88 } /* expected */
    }, {
      false, /* encrypt */
      0, /* key_size_index */
      5, /* key_area */
      { 0x6c, 0x53, 0x48, 0x9f, 0x6a, 0x4b, 0xd0, 0xf7,
        0xcd, 0x15, 0xd2, 0x0f, 0x6e, 0xbc, 0x7e, 0x64,
        0x9f, 0xd9, 0x5b, 0x76, 0xb1, 0x07, 0xe6, 0xda,
        0xba, 0x96, 0x7c, 0x8a, 0x94, 0x84, 0x79, 0x7f,
        0x29, 0xa8, 0xa2, 0x82, 0xee, 0x31, 0xc0, 0x3f,
        0xae, 0x4f, 0x8e, 0x9b, 0x89, 0x30, 0xd5, 0x47,
        0x3c, 0x2e, 0xd6, 0x95, 0xa3, 0x47, 0xe8, 0x8b,
        0x7c, 0xcd, 0x62, 0x37, 0x6d, 0x5e, 0xbb, 0x41 }, /* mdata */
      64, /* mdata_len */
      { 0x60, 0x61, 0xdf, 0x5b, 0xcd, 0x42, 0x1f, 0xab,
        0xdb, 0x52, 0x35, 0xfc, 0x03, 0x25, 0x02, 0x65,
        0x04, 0x48, 0xfd, 0x82, 0x33, 0xa0, 0x23, 0x7c,
        0x5f, 0x6f, 0x24, 0x9a, 0x63, 0xd7, 0xdb, 0x3e,
        0x42, 0x83, 0xac, 0x9a, 0x86, 0x84, 0xa3, 0x63,
        0xef, 0x64, 0xe7, 0x60, 0xc5, 0x88, 0x65, 0x64,
        0x65, 0x9d, 0xa6, 0x19, 0x46, 0x68, 0x51, 0x56,
        0x8b, 0x32, 0x09, 0x1d, 0xf5, 0x51, 0x6f, 0x57 } /* expected */
    }, {
      false, /* encrypt */
      0, /* key_size_index */
      6, /* key_area */
      { 0x4d, 0x6f, 0x97, 0x29, 0x46, 0xa2, 0xe4, 0xca,
        0x05, 0xbf, 0xbe, 0xec, 0xd0, 0x05, 0x35, 0xec,
        0xe6, 0xc8, 0x1e, 0xd9, 0x63, 0xc4, 0x16, 0x70,
        0x63, 0xb1, 0xc3, 0x4b, 0x6a, 0x18, 0x5d, 0x55,
        0x04, 0x64, 0x6d, 0x81, 0xa8, 0x3c, 0xd5, 0xbc,
        0xe2, 0x10, 0x11, 0x20, 0xb2, 0xf2, 0xbc, 0x6a,
        0x2f, 0xa8, 0x56, 0xae, 0x6f, 0xff, 0x44, 0x49,
        0xab, 0x62, 0x18, 0x18, 0x9f, 0xaa, 0x13, 0xe7,
        0x30, 0x38, 0xe8, 0x2c, 0x51, 0x41, 0xc3, 0xf6,
        0x27, 0x6a, 0x8a, 0x20, 0x6b, 0x95, 0x63, 0xca,
        0x11, 0xfa, 0x76, 0x09, 0x27, 0x79, 0xf4, 0xde,
        0xff, 0x2e, 0x58, 0x65, 0x90, 0x47, 0xed, 0x4f,
        0x1a, 0x12, 0x90, 0x92, 0x09, 0x5a, 0xb7, 0x5f,
        0x1c, 0xf4, 0xb2, 0x55, 0xc6, 0x95, 0x31, 0xf4 }, /* mdata */
      112, /* mdata_len */
      { 0x6e, 0x19, 0xce, 0xc7, 0x75, 0x59, 0x5c, 0x2f,
        0x76, 0x8d, 0xe3, 0xbd, 0xa3, 0x68, 0x2f, 0xb4,
        0x02, 0x6e, 0xd1, 0x88, 0x9f, 0xc3, 0xde, 0x11,
        0xbc, 0x45, 0xa2, 0xa3, 0xe2, 0xb2, 0x26, 0x94,
        0xdd, 0xaa, 0xb3, 0x27, 0x4c, 0x81, 0xb2, 0x92,
        0x04, 0xfa, 0x03, 0x4e, 0xed, 0xac, 0x56, 0x4a,
        0x7d, 0xfe, 0x27, 0xaa, 0xbb, 0x6f, 0xf4, 0x20,
        0xf9, 0xaa, 0x5a, 0xba, 0xdf, 0x15, 0xc7, 0x46,
        0x76, 0xef, 0xea, 0xbd, 0x96, 0xa7, 0xff, 0x24,
        0x91, 0x81, 0x3b, 0xa5, 0x3c, 0xb5, 0xab, 0x76,
        0xea, 0xcd, 0x37, 0x25, 0x5a, 0x28, 0xda, 0xec,
        0xbb, 0xe5, 0x80, 0xf5, 0xeb, 0x77, 0xf9, 0xbc,
        0x03, 0x39, 0xe0, 0xda, 0x59, 0xb2, 0xb3, 0x07,
        0xf5, 0x29, 0x74, 0xb6, 0x18, 0x2c, 0xb1, 0x06 } /* expected */
    }, {
      false, /* encrypt */
      0, /* key_size_index */
      7, /* key_area */
      { 0x93, 0x1b, 0x2f, 0x5f, 0x3a, 0x58, 0x20, 0xd5,
        0x3a, 0x6b, 0xea, 0xaa, 0x64, 0x31, 0x08, 0x3a,
        0x34, 0x88, 0xf4, 0xeb, 0x03, 0xb0, 0xf5, 0xb5,
        0x7e, 0xf8, 0x38, 0xe1, 0x57, 0x96, 0x23, 0x10,
        0x3b, 0xd6, 0xe6, 0x80, 0x03, 0x77, 0x53, 0x8b,
        0x2e, 0x51, 0xef, 0x70, 0x8f, 0x3c, 0x49, 0x56,
        0x43, 0x2e, 0x8a, 0x8e, 0xe6, 0xa3, 0x4e, 0x19,
        0x06, 0x42, 0xb2, 0x6a, 0xd8, 0xbd, 0xae, 0x6c,
        0x2a, 0xf9, 0xa6, 0xc7, 0x99, 0x6f, 0x3b, 0x60,
        0x04, 0xd2, 0x67, 0x1e, 0x41, 0xf1, 0xc9, 0xf4,
        0x0e, 0xe0, 0x3d, 0x1c, 0x4a, 0x52, 0xb0, 0xa0,
        0x65, 0x4a, 0x33, 0x1f, 0x15, 0xf3, 0x4d, 0xce,
        0x4a, 0xcb, 0x96, 0xbd, 0x65, 0x07, 0x81, 0x5c,
        0xa4, 0x34, 0x7a, 0x3d, 0xe1, 0x1a, 0x31, 0x1b,
        0x7d, 0xe5, 0x35, 0x1c, 0x97, 0x87, 0xc4, 0x53,
        0x81, 0x58, 0xe2, 0x89, 0x74, 0xff, 0xa8, 0x3d,
        0x82, 0x96, 0xdf, 0xe9, 0xcd, 0x09, 0xcd, 0x87,
        0xf7, 0xbf, 0x4f, 0x54, 0xd9, 0x7d, 0x28, 0xd4,
        0x78, 0x87, 0x99, 0x16, 0x34, 0x08, 0x32, 0x39,
        0x43, 0xb3, 0xe7, 0x2f, 0x5e, 0xab, 0x66, 0xc1 }, /* mdata */
      160, /* mdata_len */
      { 0x9c, 0x29, 0xee, 0xcb, 0x2d, 0xe0, 0x42, 0x54,
        0xfa, 0xfb, 0x89, 0x6a, 0x99, 0x41, 0x02, 0xd1,
        0xda, 0x30, 0xdd, 0xb4, 0x9d, 0x82, 0x72, 0x8e,
        0xb2, 0x3d, 0xbd, 0x02, 0x99, 0x01, 0xe9, 0xb7,
        0x5b, 0x3d, 0x0a, 0xee, 0x03, 0xf7, 0xa0, 0x5f,
        0x6c, 0x85, 0x2d, 0x8f, 0xad, 0xa0, 0xb5, 0xc2,
        0x8e, 0x8c, 0x9a, 0xed, 0x33, 0x4f, 0xad, 0x11,
        0x82, 0x9d, 0xf3, 0xdf, 0xad, 0xc5, 0xc2, 0xe4,
        0x71, 0xeb, 0x41, 0xaf, 0x9e, 0x48, 0xa8, 0xa4,
        0x65, 0xe0, 0x3d, 0x5e, 0xbd, 0xb0, 0x21, 0x69,
        0x15, 0x08, 0x1f, 0x3b, 0x5a, 0x0e, 0xbb, 0x23,
        0x08, 0xdf, 0xc2, 0xd2, 0x8e, 0x5a, 0x8b, 0xa3,
        0xf3, 0x2a, 0xda, 0xe4, 0xc3, 0x57, 0x59, 0x21,
        0xbc, 0x65, 0x7b, 0x63, 0xd4, 0x6b, 0xa5, 0xa6,
        0x18, 0x88, 0x0e, 0xe9, 0xad, 0x8a, 0xf3, 0xfb,
        0xa5, 0x64, 0x3a, 0x50, 0x26, 0xfa, 0xcd, 0x7d,
        0x66, 0x7c, 0xe5, 0x99, 0x32, 0x7f, 0x93, 0x6c,
        0xdd, 0xa7, 0xe1, 0xbb, 0x74, 0x2a, 0x33, 0xa0,
        0x19, 0x99, 0x0b, 0x76, 0xbe, 0x64, 0x8a, 0x6e,
        0xc7, 0x25, 0xda, 0xed, 0x54, 0x0e, 0xd9, 0xe7 } /* expected */
    }, {
      true, /* encrypt */
      1, /* key_size_index */
      0, /* key_area */
      { 0x60, 0xbc, 0xdb, 0x94, 0x16, 0xba, 0xc0, 0x8d,
        0x7f, 0xd0, 0xd7, 0x80, 0x35, 0x37, 0x40, 0xa5 }, /* mdata */
      16, /* mdata_len */
      { 0x24, 0xf4, 0x0c, 0x4e, 0xec, 0xd9, 0xc4, 0x98,
        0x25, 0x00, 0x0f, 0xcb, 0x49, 0x72, 0x64, 0x7a } /* expected */
    }, {
      true, /* encrypt */
      1, /* key_size_index */
      2, /* key_area */
      { 0x7c, 0x72, 0x7b, 0xd3, 0xe7, 0x04, 0x8e, 0x7a,
        0x89, 0x95, 0xb7, 0xb1, 0x16, 0x9a, 0xe4, 0xb5,
        0xa5, 0x5e, 0x85, 0x4b, 0xb4, 0xf7, 0xa9, 0x57,
        0x6d, 0x78, 0x63, 0xab, 0x28, 0x68, 0x73, 0x1d,
        0x30, 0x73, 0x22, 0xdc, 0xca, 0x60, 0x6e, 0x04,
        0x73, 0x43, 0x67, 0x6f, 0x6a, 0xf4, 0xd9, 0xcf,
        0x6e, 0xbf, 0x2b, 0xf9, 0xc9, 0x5d, 0x87, 0x84,
        0x8d, 0x23, 0x3c, 0x93, 0x1e, 0x7a, 0x60, 0xef,
        0xf0, 0x8f, 0xb9, 0x59, 0x92, 0x4c, 0xde, 0x1e,
        0xec, 0x86, 0x99, 0xeb, 0xc5, 0x78, 0x90, 0xe3,
        0x88, 0x70, 0x24, 0xef, 0x47, 0xc8, 0x9a, 0x55,
        0x00, 0x18, 0x78, 0x8d, 0x1f, 0xaa, 0x32, 0x50,
        0x45, 0x2e, 0x06, 0xf1, 0x48, 0xaf, 0x25, 0xf0,
        0x7b, 0xc6, 0x13, 0xcd, 0x2f, 0x0e, 0x50, 0x1a,
        0x79, 0xd7, 0x38, 0xd4, 0x36, 0x1f, 0x28, 0xf3,
        0x4d, 0xbe, 0xe2, 0x40, 0x34, 0xe0, 0x33, 0x67,
        0xb6, 0xb8, 0xd3, 0x4d, 0xf3, 0x73, 0x8c, 0xa3,
        0xa8, 0x6b, 0x9e, 0xbc, 0xb0, 0x9e, 0x63, 0x9b,
        0xcb, 0x5e, 0x2f, 0x51, 0x9f, 0x4a, 0x7a, 0x86,
        0xfc, 0x7c, 0x41, 0x55, 0x64, 0x04, 0xa9, 0x5d }, /* mdata */
      160, /* mdata_len */
      { 0x92, 0x28, 0x12, 0xad, 0x5f, 0xea, 0xcd, 0xf1,
        0x1f, 0xe7, 0xfd, 0xae, 0x96, 0x30, 0x01, 0x49,
        0x41, 0x9e, 0x31, 0xcf, 0xf5, 0x40, 0x61, 0xb3,
        0xc5, 0xed, 0x27, 0xfd, 0xb8, 0xb5, 0x0c, 0x9c,
        0x09, 0x32, 0xb5, 0x22, 0xa6, 0xc0, 0x4e, 0x48,
        0x24, 0x99, 0xb0, 0x11, 0xef, 0x3c, 0x3e, 0x9d,
        0xc5, 0x6a, 0x1a, 0x61, 0xcf, 0xeb, 0x78, 0xb3,
        0x40, 0x32, 0xd2, 0x6d, 0xbd, 0xc3, 0xca, 0xc5,
        0x1a, 0x32, 0x79, 0xbc, 0x93, 0x4b, 0x9b, 0xce,
        0x2d, 0x9c, 0x19, 0xbf, 0x85, 0x82, 0x35, 0x61,
        0x3b, 0xa7, 0x84, 0xe4, 0x8e, 0x29, 0x2d, 0x22,
        0xc6, 0xb5, 0xa2, 0x8e, 0x1d, 0x1b, 0xb8, 0x60,
        0x52, 0x4f, 0xb7, 0xb5, 0xf9, 0xb3, 0xd9, 0xa5,
        0xf4, 0xda, 0x66, 0xe3, 0x40, 0x58, 0x5b, 0xd2,
        0x49, 0x6f, 0xe6, 0xd6, 0x94, 0x2d, 0xb8, 0xd0,
        0x5d, 0x71, 0x6f, 0xec, 0x03, 0xb1, 0x7d, 0x19,
        0xab, 0xb5, 0x8b, 0x33, 0x33, 0x2e, 0x24, 0xbe,
        0xae, 0xc7, 0x99, 0x5d, 0x69, 0x52, 0x53, 0x64,
        0xfe, 0x13, 0x9a, 0xa1, 0xfd, 0x62, 0x05, 0x46,
        0x68, 0xc5, 0x8f, 0x23, 0xf1, 0xf9, 0x4c, 0xfd } /* expected */
    }, {
      false, /* encrypt */
      1, /* key_size_index */
      4, /* key_area */
      { 0x21, 0xc8, 0x22, 0x9a, 0x4d, 0xce, 0xaf, 0x53,
        0x3f, 0xe4, 0xe9, 0x6e, 0xce, 0xd4, 0x82, 0xa6 }, /* mdata */
      16, /* mdata_len */
      { 0x49, 0xaa, 0xbe, 0x67, 0xda, 0x53, 0x22, 0xb6,
        0xe1, 0x1d, 0x63, 0xb7, 0x8b, 0x5a, 0x0e, 0x15 } /* expected */
    }, {
      false, /* encrypt */
      1, /* key_size_index */
      6, /* key_area */
      { 0x95, 0x2f, 0x45, 0x46, 0xa8, 0xbf, 0x71, 0x66,
        0x96, 0x49, 0x17, 0xec, 0xe0, 0x1b, 0xda, 0x3c,
        0x68, 0x57, 0xe4, 0x27, 0xce, 0xf5, 0xda, 0x0f,
        0xf9, 0x0b, 0x0e, 0x4b, 0xf4, 0x4c, 0xf7, 0xcc,
        0xfc, 0xcf, 0xdf, 0x01, 0xd7, 0x13, 0xdc, 0xf9,
        0x67, 0x3f, 0x01, 0xc8, 0x7e, 0xae, 0xd5, 0x2b,
        0xf4, 0xaa, 0x04, 0x6f, 0xf7, 0x78, 0x55, 0x8e,
        0xa3, 0x96, 0xdc, 0x9c, 0xd2, 0x40, 0x71, 0x61,
        0x36, 0x38, 0x61, 0x48, 0xa5, 0xc7, 0x63, 0x78,
        0xb3, 0xff, 0xcd, 0x40, 0x86, 0x44, 0x07, 0xb8,
        0xe6, 0x0b, 0x40, 0xa5, 0x94, 0xe0, 0x61, 0x9e,
        0xdd, 0xae, 0x3f, 0x6d, 0x6e, 0x3b, 0x15, 0xb8,
        0x6a, 0xf2, 0x31, 0xe1, 0xba, 0xe5, 0xed, 0x2a,
        0xa5, 0x12, 0xe1, 0x1d, 0xa0, 0xe5, 0x57, 0x2b,
        0x67, 0xff, 0xff, 0x93, 0x4c, 0x36, 0xe5, 0x85,
        0xcf, 0xdd, 0x9f, 0x87, 0x70, 0x45, 0xcb, 0x19,
        0xc1, 0x83, 0xb9, 0x94, 0xbf, 0x74, 0x64, 0x58,
        0x62, 0xff, 0xa7, 0x26, 0x73, 0x9a, 0xad, 0xcb,
        0x9e, 0x10, 0xaa, 0xff, 0xc8, 0x81, 0xc8, 0x8c,
        0xa3, 0xaa, 0x65, 0xb3, 0x7f, 0x66, 0x7b, 0xcb }, /* mdata */
      160, /* mdata_len */
      { 0xb8, 0xbb, 0x5c, 0xe5, 0x3a, 0x15, 0xaa, 0x6d,
        0xfd, 0xf2, 0xcb, 0x61, 0xbc, 0x8e, 0x36, 0x17,
        0xd1, 0xd0, 0xfe, 0xfe, 0x9b, 0xa5, 0xd1, 0x75,
        0x55, 0x04, 0x70, 0xe3, 0x23, 0x97, 0xf6, 0xf3,
        0xb3, 0xe6, 0x5b, 0x43, 0xbd, 0xed, 0x2b, 0x21,
        0xe5, 0xc1, 0x81, 0xd3, 0xc4, 0xc4, 0xc5, 0x26,
        0xc4, 0x1c, 0xea, 0xb0, 0x44, 0x28, 0x95, 0x08,
        0x45, 0x80, 0x48, 0xb6, 0x33, 0x52, 0xdf, 0xc3,
        0x79, 0xde, 0x37, 0x3f, 0xd1, 0x9a, 0x2c, 0x90,
        0x0c, 0x43, 0x52, 0x4b, 0x75, 0x94, 0x9e, 0x67,
        0x7c, 0xce, 0xda, 0x86, 0x6f, 0x7f, 0x2b, 0xcc,
        0x48, 0x44, 0xef, 0x2e, 0x5d, 0xac, 0x5b, 0x80,
        0x4b, 0x40, 0x45, 0xe6, 0x57, 0xc8, 0x15, 0x6d,
        0x1d, 0xcd, 0xb4, 0x3c, 0xbf, 0x2f, 0x5e, 0x00,
        0xa4, 0xf9, 0x25, 0x5e, 0x3b, 0xe2, 0x43, 0x94,
        0x36, 0xc4, 0xd0, 0x44, 0x9a, 0x8d, 0x2c, 0x4c,
        0x1a, 0x56, 0xbe, 0xce, 0x98, 0xea, 0x0f, 0xd6,
        0x8a, 0xba, 0xf1, 0x23, 0x98, 0x03, 0x99, 0x94,
        0xae, 0xbf, 0xfc, 0x69, 0x2b, 0x90, 0x00, 0xe5,
        0x80, 0x47, 0x9b, 0x4f, 0x4b, 0x28, 0xb5, 0xfe } /* expected */
    }, {
      true, /* encrypt */
      2, /* key_size_index */
      0, /* key_area */
      { 0xcc, 0xc6, 0x2c, 0x6b, 0x0a, 0x09, 0xa6, 0x71,
        0xd6, 0x44, 0x56, 0x81, 0x8d, 0xb2, 0x9a, 0x4d }, /* mdata */
      16, /* mdata_len */
      { 0xdf, 0x86, 0x34, 0xca, 0x02, 0xb1, 0x3a, 0x12,
        0x5b, 0x78, 0x6e, 0x1d, 0xce, 0x90, 0x65, 0x8b } /* expected */
    }, {
      true, /* encrypt */
      2, /* key_size_index */
      2, /* key_area */
      { 0xc9, 0x1b, 0x8a, 0x7b, 0x9c, 0x51, 0x17, 0x84,
        0xb6, 0xa3, 0x7f, 0x73, 0xb2, 0x90, 0x51, 0x6b,
        0xb9, 0xef, 0x1e, 0x8d, 0xf6, 0x8d, 0x89, 0xbf,
        0x49, 0x16, 0x9e, 0xac, 0x40, 0x39, 0x65, 0x0c,
        0x43, 0x07, 0xb6, 0x26, 0x0e, 0x9c, 0x4e, 0x93,
        0x65, 0x02, 0x23, 0x44, 0x02, 0x52, 0xf5, 0xc7,
        0xd3, 0x1c, 0x26, 0xc5, 0x62, 0x09, 0xcb, 0xd0,
        0x95, 0xbf, 0x03, 0x5b, 0x97, 0x05, 0x88, 0x0a,
        0x16, 0x28, 0x83, 0x2d, 0xaf, 0x9d, 0xa5, 0x87,
        0xa6, 0xe7, 0x73, 0x53, 0xdb, 0xbc, 0xe1, 0x89,
        0xf9, 0x63, 0x23, 0x5d, 0xf1, 0x60, 0xc0, 0x08,
        0xa7, 0x53, 0xe8, 0xcc, 0xea, 0x1e, 0x07, 0x32,
        0xaa, 0x46, 0x9a, 0x97, 0x65, 0x9c, 0x42, 0xe6,
        0xe3, 0x1c, 0x16, 0xa7, 0x23, 0x15, 0x3e, 0x39,
        0x95, 0x8a, 0xbe, 0x5b, 0x8a, 0xd8, 0x8f, 0xf2,
        0xe8, 0x9a, 0xf4, 0x06, 0x22, 0xca, 0x0b, 0x0d,
        0x67, 0x29, 0xa2, 0x6c, 0x1a, 0xe0, 0x4d, 0x3b,
        0x83, 0x67, 0xb5, 0x48, 0xc4, 0xa6, 0x33, 0x5f,
        0x0e, 0x5a, 0x9e, 0xc9, 0x14, 0xbb, 0x61, 0x13,
        0xc0, 0x5c, 0xd0, 0x11, 0x25, 0x52, 0xbc, 0x21 }, /* mdata */
      160, /* mdata_len */
      { 0x05, 0xd5, 0x1a, 0xf0, 0xe2, 0xb6, 0x1e, 0x2c,
        0x06, 0xcb, 0x1e, 0x84, 0x3f, 0xee, 0x31, 0x72,
        0x82, 0x5e, 0x63, 0xb5, 0xd1, 0xce, 0x81, 0x83,
        0xb7, 0xe1, 0xdb, 0x62, 0x68, 0xdb, 0x5a, 0xa7,
        0x26, 0x52, 0x1f, 0x46, 0xe9, 0x48, 0x02, 0x8a,
        0xa4, 0x43, 0xaf, 0x9e, 0xbd, 0x8b, 0x7c, 0x6b,
        0xaf, 0x95, 0x80, 0x67, 0xab, 0x0d, 0x4a, 0x8a,
        0xc5, 0x30, 0xec, 0xbb, 0x68, 0xcd, 0xfc, 0x3e,
        0xb9, 0x30, 0x34, 0xa4, 0x28, 0xeb, 0x7e, 0x8f,
        0x6a, 0x38, 0x13, 0xce, 0xa6, 0x18, 0x90, 0x68,
        0xdf, 0xec, 0xfa, 0x26, 0x8b, 0x7e, 0xcd, 0x59,
        0x87, 0xf8, 0xcb, 0x27, 0x32, 0xc6, 0x88, 0x2b,
        0xbe, 0xc8, 0xf7, 0x16, 0xba, 0xc2, 0x54, 0xd7,
        0x22, 0x69, 0x23, 0x0a, 0xec, 0x5d, 0xc7, 0xf5,
        0xa6, 0xb8, 0x66, 0xfd, 0x30, 0x52, 0x42, 0x55,
        0x2d, 0x40, 0x0f, 0x5b, 0x04, 0x04, 0xf1, 0x9c,
        0xbf, 0xe7, 0x29, 0x1f, 0xab, 0x69, 0x0e, 0xcf,
        0xe6, 0x01, 0x8c, 0x43, 0x09, 0xfc, 0x63, 0x9d,
        0x1b, 0x65, 0xfc, 0xb6, 0x5e, 0x64, 0x3e, 0xdb,
        0x0a, 0xd1, 0xf0, 0x9c, 0xfe, 0x9c, 0xee, 0x4a } /* expected */
    }, {
      false, /* encrypt */
      2, /* key_size_index */
      4, /* key_area */
      { 0x41, 0x54, 0xc0, 0xbe, 0x71, 0x07, 0x29, 0x45,
        0xd8, 0x15, 0x6f, 0x5f, 0x04, 0x6d, 0x19, 0x8d }, /* mdata */
      16, /* mdata_len */
      { 0x8b, 0x2b, 0x1b, 0x22, 0xf7, 0x33, 0xac, 0x09,
        0xd1, 0x19, 0x6d, 0x6b, 0xe6, 0xa8, 0x7a, 0x72 } /* expected */
    }, {
      false, /* encrypt */
      2, /* key_size_index */
      6, /* key_area */
      { 0x2c, 0x48, 0x7f, 0xa9, 0x6f, 0x40, 0x90, 0xc5,
        0x6a, 0xa1, 0xb5, 0xbe, 0x81, 0x91, 0x8a, 0x93,
        0x4c, 0x94, 0x92, 0x87, 0x8f, 0xb0, 0xcd, 0x68,
        0x6d, 0xcf, 0x8d, 0x17, 0xd8, 0x64, 0x85, 0x45,
        0x4c, 0x51, 0x23, 0x7b, 0xbd, 0x09, 0x20, 0x5d,
        0xce, 0xf1, 0x55, 0x2f, 0x43, 0x0d, 0xd0, 0x98,
        0xb9, 0xd8, 0x27, 0xa6, 0x94, 0x73, 0x0c, 0x13,
        0x3a, 0x02, 0x22, 0xc7, 0x7f, 0x54, 0x0f, 0x9d,
        0x5f, 0xc2, 0xd3, 0x6a, 0xf3, 0x59, 0x58, 0x3c,
        0x9e, 0x3b, 0x49, 0xdf, 0x88, 0x42, 0x28, 0xa6,
        0x4d, 0xe7, 0x9b, 0x67, 0xf6, 0x62, 0x07, 0xc8,
        0x28, 0x13, 0x60, 0xb9, 0x9b, 0x21, 0x40, 0x42,
        0xce, 0x61, 0x36, 0x7f, 0xf9, 0x79, 0x60, 0xe9,
        0x44, 0x45, 0x3c, 0xd6, 0x36, 0x79, 0xbb, 0x44,
        0x70, 0x88, 0x97, 0xd2, 0x9b, 0xc5, 0xe7, 0x0f,
        0x9f, 0xc8, 0xf1, 0xf7, 0x15, 0x14, 0x3f, 0xbb,
        0x00, 0xf7, 0xf5, 0xc1, 0xb7, 0xb1, 0x61, 0xec,
        0x26, 0xd8, 0xd4, 0x1d, 0x36, 0xfa, 0xb0, 0xfa,
        0x8a, 0x85, 0xc3, 0xee, 0x6c, 0xe4, 0xd3, 0x70,
        0x07, 0xeb, 0x7a, 0x89, 0xd6, 0x75, 0x35, 0x90 }, /* mdata */
      160, /* mdata_len */
      { 0x31, 0xfd, 0x5a, 0x30, 0x7e, 0x27, 0x9b, 0x2f,
        0x34, 0x58, 0x1e, 0x2c, 0x43, 0x23, 0x79, 0xdf,
        0x8e, 0xcc, 0xba, 0xf7, 0x95, 0x32, 0x93, 0x89,
        0x16, 0x71, 0x1c, 0xd3, 0x77, 0x54, 0x0b, 0x90,
        0x45, 0x37, 0x3e, 0x47, 0xf2, 0x21, 0x4b, 0x8f,
        0x87, 0x60, 0x40, 0xaf, 0x73, 0x3f, 0x6c, 0x9d,
        0x8f, 0x03, 0xa7, 0xc5, 0x8f, 0x87, 0x14, 0xd2,
        0xfb, 0xb4, 0xc1, 0x4a, 0xf5, 0x9c, 0x75, 0xb4,
        0x83, 0xad, 0xc7, 0x18, 0x94, 0x6e, 0xe9, 0x07,
        0xa1, 0x82, 0x86, 0xcc, 0x4e, 0xfd, 0x20, 0x67,
        0x89, 0x06, 0x4b, 0x6f, 0x1b, 0x19, 0x5f, 0x0d,
        0x0d, 0x23, 0x44, 0x68, 0xe4, 0xf0, 0x0e, 0x6f,
        0x1c, 0xad, 0x5c, 0xd3, 0xb9, 0xc0, 0xa6, 0x43,
        0xb3, 0xc0, 0xdd, 0x09, 0x28, 0x0f, 0xf2, 0xe2,
        0xa5, 0x92, 0x91, 0x83, 0x40, 0x93, 0x84, 0xdd,
        0x72, 0xdc, 0x94, 0xe3, 0x96, 0x87, 0xea, 0x2b,
        0x62, 0x3d, 0x5d, 0x77, 0x67, 0x00, 0xbd, 0x8b,
        0x36, 0xe6, 0x13, 0x0f, 0xfd, 0xe9, 0x66, 0xf1,
        0x34, 0xc4, 0xb1, 0xf3, 0x5f, 0x29, 0xc5, 0xcc,
        0x4a, 0x03, 0x29, 0x7e, 0x1c, 0xcc, 0x95, 0x39 } /* expected */
    }
  };
  static uint8_t mdata[MDATA_MAX_LEN];
  static int i;
  static uint8_t key_size_index = -1, ret;
  static int8_t res;
  static rtimer_clock_t time, time2, total_time;

  PROCESS_BEGIN();

  puts("-----------------------------------------\n"
       "Initializing cryptoprocessor...");
  crypto_init();

  for(i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++) {
    if(key_size_index != vectors[i].key_size_index) {
      key_size_index = vectors[i].key_size_index;
      printf("-----------------------------------------\n"
             "Filling %d-bit key store...\n", 128 + (key_size_index << 6));
      time = RTIMER_NOW();
      ret = aes_load_keys(keys[key_size_index].keys,
              keys[key_size_index].key_size, keys[key_size_index].count, 0);
      time = RTIMER_NOW() - time;
      printf("aes_load_keys(): %s, %lu us\n", str_res[ret],
             (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
      PROCESS_PAUSE();
      if(ret != CRYPTO_SUCCESS) {
        break;
      }
    }

    printf("-----------------------------------------\n"
           "Test vector #%d: %s\n"
           "key_area=%d mdata_len=%d\n",
           i, vectors[i].encrypt ? "encrypt" : "decrypt",
           vectors[i].key_area, vectors[i].mdata_len);

    /* mdata has to be in SRAM. */
    rom_util_memcpy(mdata, vectors[i].mdata, vectors[i].mdata_len);

    time = RTIMER_NOW();
    ret = ecb_crypt_start(vectors[i].encrypt, vectors[i].key_area, mdata, mdata,
                          vectors[i].mdata_len, &ecb_test_process);
    time2 = RTIMER_NOW();
    time = time2 - time;
    total_time = time;
    if(ret == CRYPTO_SUCCESS) {
      PROCESS_WAIT_EVENT_UNTIL((res = ecb_crypt_check_status()) !=
                               CRYPTO_PENDING);
      time2 = RTIMER_NOW() - time2;
      total_time += time2;
    }
    printf("ecb_crypt_start(): %s, %lu us\n", str_res[ret],
           (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
    if(ret != CRYPTO_SUCCESS) {
      PROCESS_PAUSE();
      continue;
    }
    printf("ecb_crypt_check_status() wait: %s, %lu us\n", str_res[res],
           (uint32_t)((uint64_t)time2 * 1000000 / RTIMER_SECOND));
    PROCESS_PAUSE();
    if(res != CRYPTO_SUCCESS) {
      continue;
    }

    if(rom_util_memcmp(mdata, vectors[i].expected, vectors[i].mdata_len)) {
      puts("Output message does not match expected one");
    } else {
      puts("Output message OK");
    }

    printf("Total duration: %lu us\n",
           (uint32_t)((uint64_t)total_time * 1000000 / RTIMER_SECOND));
  }

  puts("-----------------------------------------\n"
       "Disabling cryptoprocessor...");
  crypto_disable();

  puts("Done!");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
