/*
 * Copyright (c) 2016, Georgios Exarchakos
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
 /**
 *
 * \file
 *         Configuration of UDP traffic generator parameters
 *
 * \author Georgios Exarchakos <g.exarchakos@tue.nl>
 */

#ifndef __TRAFFIC_CONF_H__
#define __TRAFFIC_CONF_H__

#ifndef TRAFFIC_PORT
#define TRAFFIC_PORT 9011
#endif

#ifndef TRAFFIC_TRANSMIT_PAYLOAD
#define TRAFFIC_TRANSMIT_PAYLOAD traffic_transmit_hello
#endif

//#undef TRAFFIC_CDF_SHIFT_FACTOR
//#define TRAFFIC_CDF_SHIFT_FACTOR (TRAFFIC_CDF_DELTA_PULSE - 21845) // TRAFFIC_CDF_SHIFT_FACTOR = TRAFFIC_DELTA_CDF_PULSE - 21845

//temp
#define TRAFFIC_DESTINATIONS sinks
#define TRAFFIC_DESTINATIONS_COUNT 1
static const char *sinks[TRAFFIC_DESTINATIONS_COUNT] = {
#ifdef TRAFFIC_ROUTING_RPL
  "c30c:0:0:1",
  "c30c:0:0:2",
  "c30c:0:0:3",
  "c30c:0:0:4",
  "c30c:0:0:5",
  "c30c:0:0:6",
  "c30c:0:0:7",
  "c30c:0:0:8",
  "c30c:0:0:9",
  "c30c:0:0:a",
  "c30c:0:0:b",
  "c30c:0:0:c",
  "c30c:0:0:d",
  "c30c:0:0:e",
  "c30c:0:0:f",
  "c30c:0:0:10",
  "c30c:0:0:11",
  "c30c:0:0:12",
  "c30c:0:0:13",
  "c30c:0:0:14",
  "c30c:0:0:15",
  "c30c:0:0:16",
  "c30c:0:0:17",
  "c30c:0:0:18",
  "c30c:0:0:19",
  "c30c:0:0:1a",
  "c30c:0:0:1b",
  "c30c:0:0:1c",
  "c30c:0:0:1d",
  "c30c:0:0:1e",
  "c30c:0:0:1f",
  "c30c:0:0:21",
  "c30c:0:0:22",
  "c30c:0:0:22",
  "c30c:0:0:23",
  "c30c:0:0:24",
  "c30c:0:0:25",
  "c30c:0:0:26",
  "c30c:0:0:27",
  "c30c:0:0:28",
  "c30c:0:0:29",
  "c30c:0:0:2a",
  "c30c:0:0:2b",
  "c30c:0:0:2c",
  "c30c:0:0:2d",
  "c30c:0:0:2e",
  "c30c:0:0:2f",
  "c30c:0:0:30",
  "c30c:0:0:31",
  "c30c:0:0:32",
#endif
#ifdef TRAFFIC_ROUTING_UAODV
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  32,
  33,
  34,
  35,
  36,
  37,
  38,
  39,
  40,
  41,
  42,
  43,
  44,
  45,
  46,
  47,
  48,
  49,
  50
#endif
};
#define TRAFFIC_TRANSMIT_PAYLOAD traffic_transmit_hello


// Define TRAFFIC_NEW_SYSTEM_PRECISE in order to use less efficient, more exact probability calculations.
#define TRAFFIC_NEW_SYSTEM_PRECISE


#define TRAFFIC_NEW_SYSTEM_DELTA
#define TRAFFIC_NEW_SYSTEM_DELTA_MEAN 1000

//#define TRAFFIC_NEW_SYSTEM_UNIFORM
#define TRAFFIC_NEW_SYSTEM_UNIFORM_MAX 20

//#define TRAFFIC_NEW_SYSTEM_GEOMETRIC
#define TRAFFIC_NEW_SYSTEM_GEOMETRIC_PROBABILITY 63000 // Probability that no package is transmitted in the next time stap, as a fraction of 65535
#define TRAFFIC_NEW_SYSTEM_GEOMETRIC_DOWNSCALE 1 // Time step is usually 1 second (up to 2 minutes), modify this to downscale that. 

//#define TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO
#define TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_MEAN 20
#define TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_STANDARD_DEVIATION 10
#define TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_SHAPE 2
#define TRAFFIC_NEW_SYSTEM_GENERALIZED_PARETO_GENERATION_MAX 255 // Must be set low enough (such that GENERATION_MAX ^ SHAPE < 65535). A lower value means less precise calculations.

//#define TRAFFIC_NEW_SYSTEM_POISSON
#define TRAFFIC_NEW_SYSTEM_POISSON_E_NUMERATOR 693
#define TRAFFIC_NEW_SYSTEM_POISSON_E_DENOMINATOR 255
#define TRAFFIC_NEW_SYSTEM_POISSON_RATE 1 // Must be a very small (single-digit) positive integer for accurate resultsz

#endif