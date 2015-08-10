/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
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

#include <stdlib.h>
#include "helpers.h"
#include "paging.h"
#include "prot-domains.h"
#include "syscalls.h"
#include "uart-16x50.h"

/* Refer to Intel Quark SoC X1000 Datasheet, Chapter 18 for more details on
 * UART operation.
 */

/* Divisor Latch Access Bit (DLAB) mask for Line Control Register (LCR).
 *
 * When bit is set, enables access to divisor registers to set baud rate.  When
 * clear, enables access to other registers mapped to the same addresses as the
 * divisor registers.
 */
#define UART_LCR_7_DLAB BIT(7)
/* Setting for LCR that configures the UART to operate with no parity, 1 stop
 * bit, and eight bits per character.
 */
#define UART_LCR_8BITS 0x03

/* FIFO Control Register (FCR) bitmasks */
#define UART_FCR_0_FIFOE  BIT(0) /*< enable FIFOs  */
#define UART_FCR_1_RFIFOR BIT(1) /*< reset RX FIFO */
#define UART_FCR_2_XFIFOR BIT(2) /*< reset TX FIFO */

/* Line Status Register (LSR) Transmit Holding Register Empty bitmask to check
 * whether the Transmit Holding Register (THR) or TX FIFO is empty.
 */
#define UART_LSR_5_THRE BIT(5)

/* MMIO registers for UART */
typedef struct uart_16x50_regs {
  volatile uint32_t rbr_thr_dll, ier_dlh, iir_fcr, lcr;
  volatile uint32_t mcr, lsr, msr, scr, usr, htx, dmasa;
} uart_16x50_regs_t;

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__PAGING
/* When paging-based protection domains are in use, at least one page of memory
 * must be reserved to facilitate access to the MMIO region, since that is the
 * smallest unit of memory that can be managed with paging:
 */
#define UART_MMIO_SZ MIN_PAGE_SIZE
#else
#define UART_MMIO_SZ sizeof(uart_16x50_regs_t)
#endif

void uart_16x50_setup(uart_16x50_driver_t c_this, uint16_t dl);

/*---------------------------------------------------------------------------*/
SYSCALLS_DEFINE(uart_16x50_setup, uart_16x50_driver_t c_this, uint16_t dl)
{
  uart_16x50_regs_t *regs = (uart_16x50_regs_t *)PROT_DOMAINS_MMIO(c_this);

  /* Set the DLAB bit to enable access to divisor settings. */
  regs->lcr = UART_LCR_7_DLAB;

  /* The divisor settings configure the baud rate, and may need to be defined
   * on a per-device basis.
   */
  regs->rbr_thr_dll = dl & UINT8_MAX;
  regs->ier_dlh = dl >> 8;

  /* Clear the DLAB bit to enable access to other settings and configure other
   * UART parameters.
   */
  regs->lcr = UART_LCR_8BITS;

  /* Enable the FIFOs. */
  regs->iir_fcr = UART_FCR_0_FIFOE | UART_FCR_1_RFIFOR | UART_FCR_2_XFIFOR;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief        Transmit a character through a UART.
 * \param c_this Initialized structure representing the device.
 * \param c      Character to be transmitted.
 *
 *               This procedure will block indefinitely until the UART is ready
 *               to accept the character to be transmitted.
 */
SYSCALLS_DEFINE(uart_16x50_tx, uart_16x50_driver_t c_this, uint8_t c)
{
  uart_16x50_regs_t *regs = (uart_16x50_regs_t *)PROT_DOMAINS_MMIO(c_this);

  /* Wait for space in TX FIFO. */
  while((regs->lsr & UART_LSR_5_THRE) == 0);

  /* Add character to TX FIFO. */
  regs->rbr_thr_dll = c;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief          Initialize an MMIO-programmable 16X50 UART.
 * \param c_this   Structure that will be initialized to represent the device.
 * \param pci_addr PCI address of device.
 * \param dl       Divisor setting to configure the baud rate.
 */
void
uart_16x50_init(uart_16x50_driver_t *c_this,
                pci_config_addr_t pci_addr,
                uint16_t dl)
{
  /* This assumes that the UART had an MMIO range assigned to it by the
   * firmware during boot.
   */
  pci_init(c_this, pci_addr, UART_MMIO_SZ, 0, 0);
  SYSCALLS_INIT(uart_16x50_setup);
  SYSCALLS_AUTHZ(uart_16x50_setup, *c_this);
  SYSCALLS_INIT(uart_16x50_tx);
  SYSCALLS_AUTHZ(uart_16x50_tx, *c_this);

  uart_16x50_setup(*c_this, dl);
}
/*---------------------------------------------------------------------------*/
