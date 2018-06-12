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
 *         Configuration of node
 *
 * \author Georgios Exarchakos <g.exarchakos@tue.nl>
 */

#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

// #define TRAFFIC_ROUTING_RPL
// #define TRAFFIC_ROUTING_UAODV

#include "../common-conf.h"
#ifdef TRAFFIC_ROUTING_UAODV
#define CONTIKI_WITH_RIME
#endif

//#define TRAFFIC DELTA
//#define TRAFFIC_CDF_DELTA_PULSE 10

//#define TRAFFIC UNIFORM
//#define TRAFFIC_CDF_SHRINK_FACTOR 10

#define TRAFFIC STDNORMAL
#define TRAFFIC_CDF_SHRINK_FACTOR 10

//#define TRAFFIC GPARETO
//#define TRAFFIC_CDF_SHRINK_FACTOR 10

#define TRAFFIC_DESTINATIONS sinks
#define TRAFFIC_DESTINATIONS_COUNT 3

#define TRAFFIC_TRANSMIT_PAYLOAD my_awesome_payload

int my_awesome_payload(char* buffer, int max);

#endif /* __PROJECT_CONF_H__ */
