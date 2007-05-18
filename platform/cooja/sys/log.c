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
 * $Id: log.c,v 1.3 2007/05/18 13:50:08 fros4943 Exp $
 */

#include <stdio.h>
#include <string.h>
#include "sys/log.h"
#include "lib/simEnvChange.h"

#define MAX_LOG_LENGTH 1024
const struct simInterface simlog_interface;

// COOJA variables
char simLoggedData[MAX_LOG_LENGTH];
int simLoggedLength;
char simLoggedFlag;

void simlog(const char *message);

/*-----------------------------------------------------------------------------------*/
void
log_message(const char *part1, const char *part2)
{
  simlog(part1);
  simlog(part2);
}
/*-----------------------------------------------------------------------------------*/
void
simlog(const char *message)
{
  if (simLoggedLength + strlen(message) > MAX_LOG_LENGTH) {
  	/* Dropping message due to buffer overflow */
  	printf("Warning. Dropping log message due to buffer overflow\n");
  	return;
  }
  
  memcpy(&simLoggedData[0] + simLoggedLength, &message[0], strlen(message));
  simLoggedLength += strlen(message);
  simLoggedFlag = 1;
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*-----------------------------------------------------------------------------------*/

SIM_INTERFACE(simlog_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);
