/**
******************************************************************************
* @file    uart-msg.c
* @author  System LAB
* @version V1.0.0
* @date    17-June-2015
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/

#include "contiki.h"
#include "dev/leds.h"
#include "stm32l1xx_nucleo.h"
#include "platform-conf.h"
#include <stdio.h>
#include "dev/slip.h"
#include "hw-config.h"
#include "stm32l1xx_hal.h"

void UART_SendMsg(char *Msg);
extern UART_HandleTypeDef UartHandle;

/**
* @brief  Rx Transfer completed callbacks.
* @param  huart: Pointer to a UART_HandleTypeDef structure that contains
*                the configuration information for the specified UART module.
* @retval None
*/

static unsigned char databyte[1] = {0};
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{   
  slip_input_byte(databyte[0]);
  HAL_UART_Receive_IT(&UartHandle, databyte, 1);
}

/*---------------------------------------------------------------------------*/
void
uart1_set_input(int (*input) (unsigned char c))
{
  HAL_UART_Receive_IT(&UartHandle, databyte, 1);
}

/*--------------------------------------------------------------------------*/
void
slip_arch_init(unsigned long ubr)
{
  __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE);
  //uart1_set_input(slip_input_byte);
}
/*--------------------------------------------------------------------------*/
void
slip_arch_writeb(unsigned char c)
{
  UART_SendMsg(&c);
}
/*--------------------------------------------------------------------------*/

/**
  * @brief  Send a message via UART
  * @param  Msg the pointer to the message to be sent
  * @retval None
  */
void UART_SendMsg(char *Msg)
{
     HAL_UART_Transmit(&UartHandle, (uint8_t*)Msg, 1, 5000);
}
/*--------------------------------------------------------------------------*/
