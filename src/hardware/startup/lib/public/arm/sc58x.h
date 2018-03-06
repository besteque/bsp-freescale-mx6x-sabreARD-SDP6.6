/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */


/*
 * Analog Device ADSP-SC58x processor with ARMv7 Cortex-A5 core
 */

#ifndef __ARM_SC58X_H_INCLUDED
#define __ARM_SC58X_H_INCLUDED

/*
 * L2 SRAM
 */
#define SC58X_L2_SRAM_BASE          0x20080000
#define SC58X_L2_SRAM_SIZE          0x40000

/*
 * DMA0
 */
#define SC58X_DMA0_BASE             0x31022000  /* Base address of DMA0 */
#define SC58X_DMA1_BASE             0x31022080  /* Base address of DMA1 */
#define SC58X_DMA2_BASE             0x31022100  /* Base address of DMA2 */
#define SC58X_DMA3_BASE             0x31022180  /* Base address of DMA3 */
#define SC58X_DMA4_BASE             0x31022200  /* Base address of DMA4 */
#define SC58X_DMA5_BASE             0x31022280  /* Base address of DMA5 */
#define SC58X_DMA6_BASE             0x31022300  /* Base address of DMA6 */
#define SC58X_DMA7_BASE             0x31022380  /* Base address of DMA7 */
#define SC58X_DMA10_BASE            0x31024000  /* Base address of DMA10 */
#define SC58X_DMA11_BASE            0x31024080  /* Base address of DMA11 */
#define SC58X_DMA12_BASE            0x31024100  /* Base address of DMA12 */
#define SC58X_DMA13_BASE            0x31024180  /* Base address of DMA13 */
#define SC58X_DMA14_BASE            0x31024200  /* Base address of DMA14 */
#define SC58X_DMA15_BASE            0x31024280  /* Base address of DMA15 */
#define SC58X_DMA16_BASE            0x31024300  /* Base address of DMA16 */
#define SC58X_DMA17_BASE            0x31024380  /* Base address of DMA17 */
#define SC58X_DMA20_BASE            0x31026080  /* Base address of DMA20 */
#define SC58X_DMA21_BASE            0x31026000  /* Base address of DMA21 */
#define SC58X_DMA22_BASE            0x31046000  /* Base address of DMA22 */
#define SC58X_DMA23_BASE            0x31046080  /* Base address of DMA23 */
#define SC58X_DMA24_BASE            0x31046100  /* Base address of DMA24 */
#define SC58X_DMA25_BASE            0x31046180  /* Base address of DMA25 */
#define SC58X_DMA26_BASE            0x31046200  /* Base address of DMA26 */
#define SC58X_DMA27_BASE            0x31046280  /* Base address of DMA27 */
#define SC58X_DMA28_BASE            0x31046300  /* Base address of DMA28 */
#define SC58X_DMA29_BASE            0x31046380  /* Base address of DMA29 */
#define SC58X_DMA30_BASE            0x30FFF000  /* Base address of DMA30 */
#define SC58X_DMA31_BASE            0x3102A100  /* Base address of DMA31 */
#define SC58X_DMA32_BASE            0x3102A000  /* Base address of DMA32 */
#define SC58X_DMA33_BASE            0x3102A080  /* Base address of DMA33 */
#define SC58X_DMA34_BASE            0x31026180  /* Base address of DMA34 */
#define SC58X_DMA35_BASE            0x31026100  /* Base address of DMA35 */
#define SC58X_DMA36_BASE            0x30FFF080  /* Base address of DMA36 */
#define SC58X_DMA37_BASE            0x31026280  /* Base address of DMA37 */
#define SC58X_DMA38_BASE            0x31026200  /* Base address of DMA38 */
#define SC58X_DMA_SIZE              0x80

    #define SC58X_DMA_DSCPTR_NXT    0x00        // DMA0 Pointer to Next Initial Descriptor Register
    #define SC58X_DMA_ADDRSTART     0x04        // DMA0 Start Address of Current Buffer Register
    #define SC58X_DMA_CFG           0x08        // DMA0 Configuration Register
        #define DMA_CFG_EN          (1 << 0)
        #define DMA_CFG_WNR         (1 << 1)
        #define DMA_CFG_SYNC        (1 << 2)
        #define DMA_CFG_CADDR       (1 << 3)
        #define DMA_CFG_PS_SHIFT    4
        #define DMA_CFG_PSIZE8      (0 << DMA_CFG_PS_SHIFT)
        #define DMA_CFG_PSIZE16     (1 << DMA_CFG_PS_SHIFT)
        #define DMA_CFG_PSIZE32     (2 << DMA_CFG_PS_SHIFT)
        #define DMA_CFG_PSIZE64     (3 << DMA_CFG_PS_SHIFT)
        #define DMA_CFG_MS_SHIFT    8
        #define DMA_CFG_MSIZE8      (0 << DMA_CFG_MS_SHIFT)
        #define DMA_CFG_MSIZE16     (1 << DMA_CFG_MS_SHIFT)
        #define DMA_CFG_MSIZE32     (2 << DMA_CFG_MS_SHIFT)
        #define DMA_CFG_MSIZE64     (3 << DMA_CFG_MS_SHIFT)
        #define DMA_CFG_MSIZE126    (4 << DMA_CFG_MS_SHIFT)
        #define DMA_CFG_MSIZE256    (5 << DMA_CFG_MS_SHIFT)
        #define DMA_CFG_FLOW_STOP   (0 << 12)
        #define DMA_CFG_FLOW_AUTO   (1 << 12)
        #define DMA_CFG_FLOW_DSCL   (4 << 12)
        #define DMA_CFG_FLOW_DSCA   (5 << 12)
        #define DMA_CFG_FLOW_DODL   (6 << 12)
        #define DMA_CFG_FLOW_DODA   (7 << 12)
        #define DMA_CFG_TWAIT       (1 << 15)
        #define DMA_CFG_NDSIZE(n)   ((n - 1) << 16)
        #define DMA_CFG_INT_N       (0 << 20)
        #define DMA_CFG_INT_X       (1 << 20)
        #define DMA_CFG_INT_Y       (2 << 20)
        #define DMA_CFG_INT_P       (3 << 20)
        #define DMA_CFG_TRIG_N      (0 << 22)
        #define DMA_CFG_TRIG_X      (1 << 22)
        #define DMA_CFG_TRIG_Y      (2 << 22)
        #define DMA_CFG_TOVEN       (1 << 24)
        #define DMA_CFG_DESCIDCPY   (1 << 25)
        #define DMA_CFG_TWOD        (1 << 26)
        #define DMA_CFG_PDRF        (1 << 27)
    #define SC58X_DMA_XCNT          0x0C        // DMA0 Inner Loop Count Start Value Register
    #define SC58X_DMA_XMOD          0x10        // DMA0 Inner Loop Address Increment Register
    #define SC58X_DMA_YCNT          0x14        // DMA0 Outer Loop Count Start Value (2D only) Register
    #define SC58X_DMA_YMOD          0x18        // DMA0 Outer Loop Address Increment (2D only) Register
    #define SC58X_DMA_DSCPTR_CUR    0x24        // DMA0 Current Descriptor Pointer Register
    #define SC58X_DMA_DSCPTR_PRV    0x28        // DMA0 Previous Initial Descriptor Pointer Register
    #define SC58X_DMA_ADDR_CUR      0x2C        // DMA0 Current Address Register
    #define SC58X_DMA_STAT          0x30        // DMA0 Status Register
        #define DMA_STAT_IRQDONE    (1 << 0)
        #define DMA_STAT_IRQERR     (1 << 1)
        #define DMA_STAT_PIRQ       (1 << 2)
    #define SC58X_DMA_XCNT_CUR      0x34        // DMA0 Current Count(1D) or intra-row XCNT (2D) Register
    #define SC58X_DMA_YCNT_CUR      0x38        // DMA0 Current Row Count (2D only) Register
    #define SC58X_DMA_BWLCNT        0x40        // DMA0 Bandwidth Limit Count Register
    #define SC58X_DMA_BWLCNT_CUR    0x44        // DMA0 Bandwidth Limit Count Current Register
    #define SC58X_DMA_BWMCNT        0x48        // DMA0 Bandwidth Monitor Count Register
    #define SC58X_DMA_BWMCNT_CUR    0x4C        // DMA0 Bandwidth Monitor Count Current Register

/*
 * RCU
 */
#define SC58X_RCU0_BASE             0x3108B000
    #define SC58X_RCU0_CTL          0x00
    #define SC58X_RCU0_STAT         0x04
    #define SC58X_RCU0_CRCTL        0x08
    #define SC58X_RCU0_CRSTAT       0x0C
    #define SC58X_RCU0_SIDIS        0x10
    #define SC58X_RCU0_SISTAT       0x14
    #define SC58X_RCU0_SVECT_LCK    0x18
    #define SC58X_RCU0_BCODE        0x1C
    #define SC58X_RCU0_SVECT0       0x20
    #define SC58X_RCU0_SVECT1       0x24
    #define SC58X_RCU0_SVECT2       0x28
    #define SC58X_RCU0_MSG          0x60
    #define SC58X_RCU0_MSG_SET      0x64
    #define SC58X_RCU0_MSG_CLR      0x68
    #define SC58X_RCU0_REVID        0x70

/*
 * GPTimer
 */
#define SC58X_TIMER0_BASE           0x31001000
    #define SC58X_TIMER0_RUN        0x04
    #define SC58X_TIMER0_RUN_SET    0x08
    #define SC58X_TIMER0_RUN_CLR    0x0C
    #define SC58X_TIMER0_STOP_CFG   0x10
    #define SC58X_TIMER0_STOP_SET   0x14
    #define SC58X_TIMER0_STOP_CLR   0x18
    #define SC58X_TIMER0_DATA_IMSK  0x1C
    #define SC58X_TIMER0_STAT_IMSK  0x20
    #define SC58X_TIMER0_TRG_MSK    0x24
    #define SC58X_TIMER0_TRG_IE     0x28
    #define SC58X_TIMER0_DATA_ILAT  0x2C
    #define SC58X_TIMER0_STAT_ILAT  0x30
    #define SC58X_TIMER0_ERR_TYPE   0x34
    #define SC58X_TIMER0_BCAST_PER  0x38
    #define SC58X_TIMER0_BCAST_WID  0x3C
    #define SC58X_TIMER0_BCAST_DLY  0x40
    #define SC58X_TIMER0_TMR_CFG(x) (0x60 + 0x20 * (x))
    #define SC58X_TIMER0_TMR_CNT(x) (0x64 + 0x20 * (x))
    #define SC58X_TIMER0_TMR_PER(x) (0x68 + 0x20 * (x))
    #define SC58X_TIMER0_TMR_WID(x) (0x6C + 0x20 * (x))
    #define SC58X_TIMER0_TMR_DLY(x) (0x70 + 0x20 * (x))

        // SC58X_TIMER0_TMR_CFG
        #define TMR_CFG_EMURUN      (1 << 15)
        #define TMR_CFG_BPEREN      (1 << 14)
        #define TMR_CFG_BWIDEN      (1 << 13)
        #define TMR_CFG_BDLYEN      (1 << 12)
        #define TMR_CFG_OUTDIS      (1 << 11)
        #define TMR_CFG_TINSEL      (1 << 10)
        #define TMR_CFG_CLK_SCL     (0 << 8)
        #define TMR_CFG_CLK_ALT0    (1 << 8)
        #define TMR_CFG_CLK_ALT1    (3 << 8)
        #define TMR_CFG_PULSEHI     (1 << 7)
        #define TMR_CFG_IRQ_AE      (0 << 4)
        #define TMR_CFG_IRQ_DE      (1 << 4)
        #define TMR_CFG_IRQ_WPDE    (2 << 4)
        #define TMR_CFG_IRQ_PE      (3 << 4)
        #define TMR_CFG_MODE_WDOG_P (8 << 0)
        #define TMR_CFG_MODE_WDOG_W (9 << 0)
        #define TMR_CFG_MODE_PWM_C  (12 << 0)
        #define TMR_CFG_MODE_PWM    (13 << 0)

/*
 * TWI
 */
#define SC58X_TWI0_BASE             0x31001400
#define SC58X_TWI1_BASE             0x31001500
#define SC58X_TWI2_BASE             0x31001600
#define SC58X_TWI_SIZE              0x100
    #define SC58X_TWI_CLKDIV        0x00            // SCL Clock Divider Register
    #define SC58X_TWI_CTL           0x04            // Control Register
        #define TWI_CTL_EN              (1 << 7)    // Module enable
    #define SC58X_TWI_SLVCTL        0x08            // Slave Mode Control Register
    #define SC58X_TWI_SLVSTAT       0x0C            // Slave Mode Status Register
    #define SC58X_TWI_SLVADDR       0x10            // Slave Mode Address Register
    #define SC58X_TWI_MSTRCTL       0x14            // Master Mode Control Registers
        #define TWI_MSTRCTL_SCLOVR      (1 << 16)
        #define TWI_MSTRCTL_SDAOVR      (1 << 15)
        #define TWI_MSTRCTL_DCNT(x)     ((x) << 6)  // Data count
        #define TWI_MSTRCTL_RSTART      (1 << 5)    // Repeat Start
        #define TWI_MSTRCTL_STOP        (1 << 4)    // Issue Stop Condition
        #define TWI_MSTRCTL_FAST        (1 << 3)    // Fast Mode
        #define TWI_MSTRCTL_READ        (1 << 2)    // Mster Read
        #define TWI_MSTRCTL_WRITE       (0 << 2)    // Mster Write
        #define TWI_MSTRCTL_EN          (1 << 0)    // Mster Enable
    #define SC58X_TWI_MSTRSTAT      0x18            // Master Mode Status Register
        #define TWI_MSTRSTAT_BUSBUSY    (1 << 8)    // Bus Busy
        #define TWI_MSTRSTAT_SCLSEN     (1 << 7)    // Serial Clock Sense
        #define TWI_MSTRSTAT_SDASEN     (1 << 6)    // Serial Data Sense
        #define TWI_MSTRSTAT_BUFWRERR   (1 << 5)    // Buffer Write Error
        #define TWI_MSTRSTAT_BUFRDERR   (1 << 4)    // Buffer Read Error
        #define TWI_MSTRSTAT_DNAK       (1 << 3)    // Data Not Acknowledged
        #define TWI_MSTRSTAT_ANAK       (1 << 2)    // Address Not Acknowledged
        #define TWI_MSTRSTAT_LOSTARB    (1 << 1)    // Lost Arbitration
        #define TWI_MSTRSTAT_MPROG      (1 << 0)    // Master Transfer in Progress
    #define SC58X_TWI_MSTRADDR      0x1C            // Master Mode Address Register
    #define SC58X_TWI_ISTAT         0x20            // Interrupt Status Register
    #define SC58X_TWI_IMSK          0x24            // Interrupt Mask Register
        #define TWI_INTR_SCLI           (1 << 15)   // Serial Clock Interrupt
        #define TWI_INTR_SDAI           (1 << 14)   // Serial Data Interrupt
        #define TWI_INTR_RXSERV         (1 <<  7)   // Rx FIFO Service Interrupt
        #define TWI_INTR_TXSERV         (1 <<  6)   // Tx FIFO Service Interrupt
        #define TWI_INTR_MERR           (1 <<  5)   // Master Transfer Error Interrupt
        #define TWI_INTR_MCOMP          (1 <<  4)   // Master Transfer Complete Interrupt
    #define SC58X_TWI_FIFOCTL       0x28            // FIFO Control Register
        #define TWI_FIFOCTL_RXFLUSH     (1 << 1)    // Rx Buffer Flush
        #define TWI_FIFOCTL_TXFLUSH     (1 << 0)    // Tx Buffer Flush
    #define SC58X_TWI_FIFOSTAT      0x2C            // FIFO Status Register
    #define SC58X_TWI_TXDATA8       0x80            // Tx Data Single-Byte Register
    #define SC58X_TWI_TXDATA16      0x84            // Tx Data Double-Byte Register
    #define SC58X_TWI_RXDATA8       0x88            // Rx Data Single-Byte Register
    #define SC58X_TWI_RXDATA16      0x8C            // Rx Data Double-Byte Register

/*
 * UART
 */
#define SC58X_UART0_BASE            0x31003000
#define SC58X_UART1_BASE            0x31003400
#define SC58X_UART2_BASE            0x31003800
#define SC58X_UART_SIZE             0x100

    #define SC58X_UART_CTRL         0x04            // UART Control Register
        #define UART_CTRL_RFRT      (1 << 30)       // Receive FIFO RTS Threshold
        #define UART_CTRL_RFIT      (1 << 29)       // Receive FIFO IRQ Threshold
        #define UART_CTRL_ACTS      (1 << 28)       // Automatic CTS
        #define UART_CTRL_ARTS      (1 << 27)       // Automatic RTS
        #define UART_CTRL_XOFF      (1 << 26)       // Transmitter OFF
        #define UART_CTRL_MRTS      (1 << 25)       // Manual RTS
        #define UART_CTRL_SB        (1 << 19)       // Set Break
        #define UART_CTRL_STP       (1 << 16)       // Sticky Parity
        #define UART_CTRL_EPS       (1 << 15)       // Even Parity Select
        #define UART_CTRL_PEN       (1 << 14)       // Parity Enable
        #define UART_CTRL_STBH      (1 << 13)       // Stop Bit Half Bit Time
        #define UART_CTRL_STB       (1 << 12)       // Stop Bit
        #define UART_CTRL_WLS8      (3 <<  8)       // 8-bit Word
        #define UART_CTRL_WLS7      (2 <<  8)       // 7-bit Word
        #define UART_CTRL_WLS6      (1 <<  8)       // 6-bit Word
        #define UART_CTRL_WLS5      (0 <<  8)       // 5-bit Word
        #define UART_CTRL_MOD_UART  (0 <<  4)       // UART Mode
        #define UART_CTRL_LOOP_EN   (1 <<  1)       // Loopback Enable
        #define UART_CTRL_EN        (1 <<  0)       // UART Enable
    #define SC58X_UART_STAT         0x08            // UART Status Register
        #define UART_STAT_RFCS      (1 << 17)       // Receive FIFO Count Status
        #define UART_STAT_SCTS      (1 << 12)       // Sticky CTS
        #define UART_STAT_RO        (1 << 11)       // Reception On-going
        #define UART_STAT_TFI       (1 <<  8)       // Transmission Finished Indicator
        #define UART_STAT_TEMT      (1 <<  7)       // TSR and THR Empty
        #define UART_STAT_THRE      (1 <<  5)       // Transmit Hold Register Empty
        #define UART_STAT_BI        (1 <<  4)       // Break Indicator
        #define UART_STAT_FE        (1 <<  3)       // Framing Error
        #define UART_STAT_PE        (1 <<  2)       // Parity Error
        #define UART_STAT_OE        (1 <<  1)       // Overrun Error
        #define UART_STAT_DR        (1 <<  0)       // Data Ready
    #define SC58X_UART_SCR          0x0C            // UART Scratch Register
    #define SC58X_UART_CLK          0x10            // UART Clock Rate Register
    #define SC58X_UART_IMSK         0x14            // UART Interrupt Mask Register
    #define SC58X_UART_IMSK_SET     0x18            // UART Interrupt Mask Set Register
    #define SC58X_UART_IMSK_CLR     0x1C            // UART Interrupt Mask Clear Register
        #define UART_IMSK_ETXS      (1 << 9)        // TX to Status Interrupt
        #define UART_IMSK_ERXS      (1 << 8)        // RX to Status Interrupt
        #define UART_IMSK_EAWI      (1 << 7)        // Address Word Interrupt
        #define UART_IMSK_ERFCI     (1 << 6)        // Receive FIFO Count Interrupt
        #define UART_IMSK_ETFI      (1 << 5)        // Transmission Finished Interrupt
        #define UART_IMSK_EDTPTI    (1 << 4)        // DMA TX Peripheral Trigerred Interrupt
        #define UART_IMSK_EDSSI     (1 << 3)        // Modem Status Interrupt
        #define UART_IMSK_ELSI      (1 << 2)        // Line Status Interrupt
        #define UART_IMSK_ETBEI     (1 << 1)        // Transmit Buffer Empty Interrupt
        #define UART_IMSK_ERBFI     (1 << 0)        // Receive Buffer Full Interrupt
    #define SC58X_UART_RBR          0x20            // UART Receive Buffer Register
    #define SC58X_UART_THR          0x24            // UART Transmit Hold Register
    #define SC58X_UART_TAIP         0x28            // UART Transmit Address/Insert Pulse Register
    #define SC58X_UART_TSR          0x2C            // UART Transmit Shift Register
    #define SC58X_UART_RSR          0x30            // UART Receive Shift Register
    #define SC58X_UART_TXCNT        0x34            // UART Transmit Counter Register
    #define SC58X_UART_RXCNT        0x38            // UART Receive Counter Register

/*
 * GPIO
 */
#define SC58X_PORT0_BASE            0x31004000
#define SC58X_PORT(x)               ((x) * 0x80)
    #define SC58X_PORTA             0
    #define SC58X_PORTB             1
    #define SC58X_PORTC             2
    #define SC58X_PORTD             3
    #define SC58X_PORTE             4
    #define SC58X_PORTF             5
    #define SC58X_PORTG             6

    #define SC58X_PORT_FER          0x00            // Function Enable Register
    #define SC58X_PORT_FER_SET      0x04            // Function Enable Set Register
    #define SC58X_PORT_FER_CLR      0x08            // Function Enable Clear Register
    #define SC58X_PORT_DATA         0x0C            // GPIO Data Register
    #define SC58X_PORT_DATA_SET     0x10            // GPIO Data Set Register
    #define SC58X_PORT_DATA_CLR     0x14            // GPIO Data Clear Register
    #define SC58X_PORT_DIR          0x18            // GPIO Direction Register
    #define SC58X_PORT_DIR_SET      0x1C            // GPIO Direction Set Register
    #define SC58X_PORT_DIR_CLR      0x20            // GPIO Direction Clear Register
    #define SC58X_PORT_INEN         0x24            // GPIO Input Enable Register
    #define SC58X_PORT_INEN_SET     0x28            // GPIO Input Enable Set Register
    #define SC58X_PORT_INEN_CLR     0x2C            // GPIO Input Enable Clear Register
    #define SC58X_PORT_MUX          0x30            // Multiplexer Control Register
        #define SC58X_PORTMUX(p, f) ((f) << (2 * (p)))
    #define SC58X_PORT_DATA_TGL     0x34            // GPIO Output Toggle Register
    #define SC58X_PORT_POL          0x38            // GPIO Polarity Invert Register
    #define SC58X_PORT_POL_SET      0x3C            // GPIO Polarity Invert Set Register
    #define SC58X_PORT_POL_CLR      0x40            // GPIO Polarity Invert Clear  Register
    #define SC58X_PORT_LOCK         0x44            // GPIO Lock Register

/*
 * EMAC
 */
#define SC58X_EMAC0_BASE            0x3100C000      // EMAC0 Base
#define SC58X_EMAC1_BASE            0x3100E000      // EMAC0 Base

/*
 * SPI
 */
#define SC58X_SPI0_BASE             0x31042000
#define SC58X_SPI1_BASE             0x31043000
#define SC58X_SPI2_BASE             0x31044000
#define SC58X_SPI_SIZE              0x100

    #define SC58X_SPI_CTL           0x04            // Control Register
        #define SPI_CTL_MMSE        (1 << 31)       // Memory-Mapped SPI Enable
        #define SPI_CTL_MMWEM       (1 << 30)       // Memory Mapped Write Error Mask
        #define SPI_CTL_SOSI        (1 << 22)       // Start on MOSI
        #define SPI_CTL_DIOM        (1 << 20)       // DIO Mode
        #define SPI_CTL_QIOM        (2 << 20)       // QIO Mode
        #define SPI_CTL_FMODE       (1 << 18)       // Fast-Mode Enable
        #define SPI_CTL_FCWM75      (1 << 16)       // Flow Control Watermark, TFIFO 75% or more empty, or RFIFO 75% or more full
        #define SPI_CTL_FCWM50      (2 << 16)       // Flow Control Watermark, TFIFO 50% or more empty, or RFIFO 50% or more full
        #define SPI_CTL_FCPL        (1 << 15)       // Flow Control Polarity, Active-high RDY
        #define SPI_CTL_FCCH_TX     (1 << 14)       // Flow Control Channel Selection, Flow control on TX buffer
        #define SPI_CTL_FCEN        (1 << 13)       // Flow Control Enable
        #define SPI_CTL_LSBF        (1 << 12)       // Least Significant Bit First
        #define SPI_CTL_WTS8        (0 <<  9)       // Word Transfer Size, 8 bit
        #define SPI_CTL_WTS16       (1 <<  9)       // Word Transfer Size, 16 bit
        #define SPI_CTL_WTS32       (2 <<  9)       // Word Transfer Size, 32 bit
        #define SPI_CTL_SELST       (1 <<  7)       // Slave Select Polarity Between Transfers
        #define SPI_CTL_ASSEL       (1 <<  6)       // Slave Select Pin Control
        #define SPI_CTL_CPOLH       (0 <<  5)       // Clock Polarity, Active-high SPI CLK
        #define SPI_CTL_CPOLL       (1 <<  5)       // Clock Polarity, Active-low SPI CLK
        #define SPI_CTL_CPHA        (1 <<  4)       // Clock Phase, SPI CLK toggles from start
        #define SPI_CTL_MSTR        (1 <<  1)       // Master
        #define SPI_CTL_EN          (1 <<  0)       // Enable
    #define SC58X_SPI_RXCTL         0x08            // Receive Control Register
        #define SPI_RXCTL_RDR_NE    (1 <<  4)       // Receive Data Request, not empty RFIFO
        #define SPI_RXCTL_RDR_QF    (2 <<  4)       // Receive Data Request, 25% full RFIFO
        #define SPI_RXCTL_RDR_HF    (3 <<  4)       // Receive Data Request, 50% full RFIFO
        #define SPI_RXCTL_RDR_QE    (4 <<  4)       // Receive Data Request, 75% full RFIFO
        #define SPI_RXCTL_RDR_F     (5 <<  4)       // Receive Data Request, full RFIFO
        #define SPI_RXCTL_RWCEN     (1 <<  3)       // Receive Word Counter Enable
        #define SPI_RXCTL_RTI       (1 <<  2)       // Receive Transfer Initiate
        #define SPI_RXCTL_REN       (1 <<  0)       // Receive Enable
    #define SC58X_SPI_TXCTL         0x0C            // Transmit Control Register
        #define SPI_TXCTL_TDR_NF    (1 <<  4)       // Transmit Data Request, not full RFIFO
        #define SPI_TXCTL_TDR_QE    (2 <<  4)       // Transmit Data Request, 25% empty RFIFO
        #define SPI_TXCTL_TDR_HE    (3 <<  4)       // Transmit Data Request, 50% empty RFIFO
        #define SPI_TXCTL_TDR_QF    (3 <<  4)       // Transmit Data Request, 75% empty RFIFO
        #define SPI_TXCTL_TWCEN     (1 <<  3)       // Transmit Word Counter Enable
        #define SPI_TXCTL_TTI       (1 <<  2)       // Transmit Transfer Initiate
        #define SPI_TXCTL_TEN       (1 <<  0)       // Transmit Enable
    #define SC58X_SPI_CLK           0x10            // Clock Rate Register
    #define SC58X_SPI_DLY           0x14            // Delay Register
        #define SPI_DLY_LAGX        (1 <<  9)       // Extended SPI Clock Lag Enable
        #define SPI_DLY_LEADX       (1 <<  8)       // Extended SPI Clock Lead Enable
        #define SPI_DLY_STOP(x)     (x)             // Transfer delay time in multiples of SPI clock period
    #define SC58X_SPI_SLVSEL        0x18            // Slave Select Register
    #define SC58X_SPI_RWC           0x1C            // Received Word Count Register
    #define SC58X_SPI_RWCR          0x20            // Received Word Count Reload Register
    #define SC58X_SPI_TWC           0x24            // Transmitted Word Count Register
    #define SC58X_SPI_TWCR          0x28            // Transmitted Word Count Reload Register
    #define SC58X_SPI_IMSK          0x30            // Interrupt Mask Register
    #define SC58X_SPI_IMSK_CLR      0x34            // Interrupt Mask Clear Register
    #define SC58X_SPI_IMSK_SET      0x38            // Interrupt Mask Set Register
        #define SPI_IMSK_TF         (1 << 11)       // Transmit Finish
        #define SPI_IMSK_RF         (1 << 10)       // Receive Finish
        #define SPI_IMSK_TS         (1 <<  9)       // Transmit Start
        #define SPI_IMSK_RS         (1 <<  8)       // Receive Start
        #define SPI_IMSK_MF         (1 <<  7)       // Mode Fault
        #define SPI_IMSK_TC         (1 <<  6)       // Transmit Collision
        #define SPI_IMSK_TUR        (1 <<  5)       // Transmit Underrun
        #define SPI_IMSK_ROR        (1 <<  4)       // Receive Overrun
        #define SPI_IMSK_TUWM       (1 <<  2)       // Transmit Urgent Watermark
        #define SPI_IMSK_RUWM       (1 <<  1)       // Receive Urgent Watermark
    #define SC58X_SPI_STAT          0x40            // Status Register
        #define SPI_STAT_MMAE       (1 << 31)       // Memory Mapped Access Error
        #define SPI_STAT_MMRE       (1 << 29)       // Memory Mapped Read Error
        #define SPI_STAT_MMWE       (1 << 28)       // Memory Mapped Write Error
        #define SPI_STAT_TFF        (1 << 23)       // TFIFO Full
        #define SPI_STAT_RFE        (1 << 22)       // RFIFO Empty
        #define SPI_STAT_FCS        (1 << 20)       // Flow Control Stall Indication
        #define SPI_STAT_TF         (1 << 11)       // Transmit Finish Indication
        #define SPI_STAT_RF         (1 << 10)       // Receive Finish Indication
        #define SPI_STAT_TS         (1 <<  9)       // Transmit Start
        #define SPI_STAT_RS         (1 <<  8)       // Receive Start
        #define SPI_STAT_MF         (1 <<  7)       // Mode Fault
        #define SPI_STAT_TC         (1 <<  6)       // Transmit Collision
        #define SPI_STAT_TUR        (1 <<  5)       // Transmit Underrun
        #define SPI_STAT_ROR        (1 <<  4)       // Receive Overrun
        #define SPI_STAT_TUWM       (1 <<  2)       // Transmit Urgent Watermark
        #define SPI_STAT_RUWM       (1 <<  1)       // Receive Urgent Watermark
        #define SPI_STAT_SPIF       (1 <<  0)       // SPI Finished
    #define SC58X_SPI_ILAT          0x44            // Masked Interrupt Condition Register
    #define SC58X_SPI_ILAT_CLR      0x48            // Masked Interrupt Clear Register
    #define SC58X_SPI_RFIFO         0x50            // Receive FIFO Data Register
    #define SC58X_SPI_TFIFO         0x58            // Transmit FIFO Data Register
    #define SC58X_SPI_MMRDH         0x60            // Memory Mapped Read Header
    #define SC58X_SPI_MMTOP         0x64            // SPI Memory Top Address


/*
 * DMC
 */
#define SC58X_DMC0_BASE             0x31070000
#define SC58X_DMC1_BASE             0x31073000

    #define SC58X_DMC_CTL           0x04
    #define SC58X_DMC_STAT          0x08
    #define SC58X_DMC_EFFCTL        0x0C
    #define SC58X_DMC_PRIO          0x10
    #define SC58X_DMC_PRIOMASK      0x14
    #define SC58X_DMC_PRIO2         0x18
    #define SC58X_DMC_PRIOMASK2     0x1C
    #define SC58X_DMC_CFG           0x40
    #define SC58X_DMC_TR0           0x44
    #define SC58X_DMC_TR1           0x48
    #define SC58X_DMC_TR2           0x4C
    #define SC58X_DMC_MSK           0x5C
    #define SC58X_DMC_MR            0x60
    #define SC58X_DMC_MR1           0x64
    #define SC58X_DMC_EMR1          0x64
    #define SC58X_DMC_MR2           0x68
    #define SC58X_DMC_EMR2          0x68
    #define SC58X_DMC_DLLCTL        0x80
    #define SC58X_DMC_DT_CALIB_ADDR 0x90
    #define SC58X_DMC_DT_DATA_CALIB_DATA0  0x94
    #define SC58X_DMC_DT_DATA_CALIB_DATA1  0x98
    #define SC58X_DMC_RDDATABUFID1  0x100
    #define SC58X_DMC_RDDATABUFMSK1 0x104
    #define SC58X_DMC_RDDATABUFID2  0x104
    #define SC58X_DMC_RDDATABUFMSK2 0x108
    #define SC58X_DMC_PHY_CTL0      0x1000
    #define SC58X_DMC_PHY_CTL1      0x1004
    #define SC58X_DMC_PHY_CTL4      0x1010
    #define SC58X_DMC_CAL_PADCTL0   0x1034
    #define SC58X_DMC_CAL_PADCTL2   0x103C

/*
 * CGU
 */
#define SC58X_CGU_BASE              0x3108D000

    #define SC58X_CGU_CTRL          0x00        // CGU0 Control Register
        #define CGU_CTRL_LOCK       (1 << 31)
        #define CGU_CTRL_MSEL(x)    (((x) & 0x7F) << 8)
        #define CGU_CTRL_DF         (1 << 0)
    #define SC58X_CGU_PLLCTL        0x04        // CGU0 PLL Control Register
        #define CGU_PLLCTL_PLLBPST  (1 << 0)    // PLL Bypass Set
        #define CGU_PLLCTL_PLLBPCL  (1 << 1)    // PLL Bypass Clear
        #define CGU_PLLCTL_PLLDIS   (1 << 2)    // PLL Disable
        #define CGU_PLLCTL_PLLEN    (1 << 3)    // PLL Enable
        #define CGU_PLLCTL_LOCK     (1 << 31)   // PLL Lock
    #define SC58X_CGU_STAT          0x08        // CGU0 Status Register
        #define CGU_STAT_PLLEN      (1 << 0)    // PLL Enable
        #define CGU_STAT_PLLBP      (1 << 1)    // PLL Bypass
        #define CGU_STAT_PLOCK      (1 << 2)    // PLL Lock
        #define CGU_STAT_CLKSALGN   (1 << 3)    // PLL Alignment
    #define SC58X_CGU_DIV           0x0C        // CGU0 Clocks Divisor Register
        #define CGU_DIV_CSEL(x)     ((x) << 0)  // CCLK Divisor
        #define CGU_DIV_S0SEL(x)    ((x) << 5)  // SCLK 0 Divisor
        #define CGU_DIV_SYSSEL(x)   ((x) << 8)  // SYSCLK Divisor
        #define CGU_DIV_S1SEL(x)    ((x) << 13) // SCLK 1 Divisor
        #define CGU_DIV_DSEL(x)     ((x) << 16) // DCLK Divisor
        #define CGU_DIV_OSEL(x)     ((x) << 22) // OUTCLK Divisor
        #define CGU_DIV_ALGN        (1 << 29)   // Align
        #define CGU_DIV_UPDT        (1 << 30)   // Update Clock Divisors
        #define CGU_DIV_LOCK        (1 << 31)   // Lock
    #define SC58X_CGU_CLKOUTSEL     0x10        // CGU0 CLKOUT Select Register
    #define SC58X_CGU_OSCWDCTL      0x14        // CGU0 Oscillator Watchdog Register
    #define SC58X_CGU_TSCTL         0x18        // CGU0 Timestamp Control Register
    #define SC58X_CGU_TSVALUE0      0x1C        // CGU0 Timestamp Counter Initial 32 LSB Value Register
    #define SC58X_CGU_TSVALUE1      0x20        // CGU0 TimeStamp Counter Initial MSB Value Register
    #define SC58X_CGU_TSCOUNT0      0x24        // CGU0 Time Stamp Counter 32 LSB Register
    #define SC58X_CGU_TSCOUNT1      0x28        // CGU0 Time Stamp Counter 32 MSB Register
    #define SC58X_CGU_CCBF_DIS      0x2C        // CGU0 Core Clock Buffer Disable Register
    #define SC58X_CGU_CCBF_STAT     0x30        // CGU0 Core Clock Buffer Status Register
    #define SC58X_CGU_SCBF_DIS      0x38        // CGU0 System Clock Buffer Disable Register
    #define SC58X_CGU_SCBF_STAT     0x3C        // CGU0 System Clock Buffer Status Register
    #define SC58X_CGU_REVID         0x48        // CGU0 Revision ID Register

/*
 * GIC
 */
#define SC58X_GICDST_BASE           0x310B2000  // GIC Distributor Port Base Address
#define SC58X_GICCPU_BASE           0x310B4000  // GIC CPU Port Base Address

/*
 * IRQ list
 */
#define SC58X_IRQ_SEC0_ERR          (32 +   0)  // SEC0Error
#define SC58X_IRQ_CGU0_EVT          (32 +   1)  // CGU0Event
#define SC58X_IRQ_CGU1_EVT          (32 +   2)  // CGU1Event
#define SC58X_IRQ_WDOG0_EXP         (32 +   3)  // WDOG0Expiration
#define SC58X_IRQ_WDOG1_EXP         (32 +   4)  // WDOG1Expiration Level
#define SC58X_IRQ_OTPC0_ERR         (32 +   5)  // OTPC0Dual-bit error
#define SC58X_IRQ_TMU0_FAULT        (32 +   6)  // TMU0Fault
#define SC58X_IRQ_TAPC_KEYFAIL      (32 +   7)  // TAPCTest/User Key Fail Interrupt
#define SC58X_IRQ_L2CTL0_ECC_ERR    (32 +   8)  // L2CTL0ECC Error
#define SC58X_IRQ_C1_DATA_READ_ERR  (32 +  14)  // Core 1 Data Read Interrupt
#define SC58X_IRQ_C1_DATA_WRITE_ERR (32 +  15)  // Core 1 Data Write Interrupt
#define SC58X_IRQ_C1_INST_READ_ERR  (32 +  16)  // Core 1 Instruction Read Interrupt
#define SC58X_IRQ_C1_IDLE           (32 +  17)  // Core 1 Idle Interrupt
#define SC58X_IRQ_C2_DATA_READ_ERR  (32 +  18)  // Core 2 Data Read Interrupt
#define SC58X_IRQ_C2_DATA_WRITE_ERR (32 +  19)  // Core 2 Data Write Interrupt
#define SC58X_IRQ_C2_INST_READ_ERR  (32 +  20)  // Core 2 Instruction Read Interrupt
#define SC58X_IRQ_C2_IDLE           (32 +  21)  // Core 2 Idle Interrupt
#define SC58X_IRQ_C0_L2CC           (32 +  22)  // Core 0 L2CC Interrupt
#define SC58X_IRQ_C0_L1_PERR        (32 +  23)  // Core 0 L1 Parity Interrupt
#define SC58X_IRQ_DAI0_IRQH         (32 +  24)  // DAI0High Priority Interrupt
#define SC58X_IRQ_DAI1_IRQH         (32 +  25)  // DAI1High Priority Interrupt
#define SC58X_IRQ_TIMER0_TMR0       (32 +  26)  // TIMER0Timer 0
#define SC58X_IRQ_TIMER0_TMR1       (32 +  27)  // TIMER0Timer 1
#define SC58X_IRQ_TIMER0_TMR2       (32 +  28)  // TIMER0Timer 2
#define SC58X_IRQ_TIMER0_TMR3       (32 +  29)  // TIMER0Timer 3
#define SC58X_IRQ_PWM0_SYNC         (32 +  30)  // PWM0PWMTMR Grouped
#define SC58X_IRQ_PWM0_TRIP         (32 +  31)  // PWM0Trip
#define SC58X_IRQ_PWM1_SYNC         (32 +  32)  // PWM1PWMTMR Grouped
#define SC58X_IRQ_PWM1_TRIP         (32 +  33)  // PWM1Trip
#define SC58X_IRQ_PWM2_SYNC         (32 +  34)  // PWM2PWMTMR Grouped
#define SC58X_IRQ_PWM2_TRIP         (32 +  35)  // PWM2Trip
#define SC58X_IRQ_ACM0_EVT_MISS     (32 +  36)  // ACM0Event Miss
#define SC58X_IRQ_ACM0_EVT_COMPLETE (32 +  37)  // ACM0Event Complete
#define SC58X_IRQ_PINT0_BLOCK       (32 +  38)  // PINT0Pin Interrupt Block 0
#define SC58X_IRQ_PINT1_BLOCK       (32 +  39)  // PINT1Pin Interrupt Block 1
#define SC58X_IRQ_PINT2_BLOCK       (32 +  40)  // PINT2Pin Interrupt Block 2
#define SC58X_IRQ_PINT3_BLOCK       (32 +  41)  // PINT3Pin Interrupt Block 3
#define SC58X_IRQ_PINT4_BLOCK       (32 +  42)  // PINT4Pin Interrupt Block 4
#define SC58X_IRQ_PINT5_BLOCK       (32 +  43)  // PINT5Pin Interrupt Block 5
#define SC58X_IRQ_SOFT0_INT         (32 +  44)  // Software Interrupt 0
#define SC58X_IRQ_SOFT1_INT         (32 +  45)  // Software Interrupt 1
#define SC58X_IRQ_SOFT2_INT         (32 +  46)  // Software Interrupt 2
#define SC58X_IRQ_SOFT3_INT         (32 +  47)  // Software Interrupt 3
#define SC58X_IRQ_SOFT4_INT         (32 +  48)  // Software Interrupt 4
#define SC58X_IRQ_SOFT5_INT         (32 +  49)  // Software Interrupt 5
#define SC58X_IRQ_SOFT6_INT         (32 +  50)  // Software Interrupt 6
#define SC58X_IRQ_SOFT7_INT         (32 +  51)  // Software Interrupt 7
#define SC58X_IRQ_SPORT0_A_DMA      (32 +  52)  // SPORT0Channel A DMA 0
#define SC58X_IRQ_SPORT0_A_STAT     (32 +  53)  // SPORT0Channel A Status
#define SC58X_IRQ_SPORT0_B_DMA      (32 +  54)  // SPORT0Channel B DMA 1
#define SC58X_IRQ_SPORT0_B_STAT     (32 +  55)  // SPORT0Channel B Status
#define SC58X_IRQ_SPORT1_A_DMA      (32 +  56)  // SPORT1Channel A DMA 2
#define SC58X_IRQ_SPORT1_A_STAT     (32 +  57)  // SPORT1Channel A Status
#define SC58X_IRQ_SPORT1_B_DMA      (32 +  58)  // SPORT1Channel B DMA 3
#define SC58X_IRQ_SPORT1_B_STAT     (32 +  59)  // SPORT1Channel B Status
#define SC58X_IRQ_SPORT4_A_DMA      (32 +  60)  // SPORT4Channel A DMA 10
#define SC58X_IRQ_SPORT4_A_STAT     (32 +  61)  // SPORT4Channel A Status
#define SC58X_IRQ_SPORT4_B_DMA      (32 +  62)  // SPORT4Channel B DMA 11
#define SC58X_IRQ_SPORT4_B_STAT     (32 +  63)  // SPORT4Channel B Status
#define SC58X_IRQ_SPORT5_A_DMA      (32 +  64)  // SPORT5Channel A DMA 12
#define SC58X_IRQ_SPORT5_A_STAT     (32 +  65)  // SPORT5Channel A Status
#define SC58X_IRQ_SPORT5_B_DMA      (32 +  66)  // SPORT5Channel B DMA 13
#define SC58X_IRQ_SPORT5_B_STAT     (32 +  67)  // SPORT5Channel B Status
#define SC58X_IRQ_SPI2_TXDMA        (32 +  68)  // SPI2TX DMA Channel 26
#define SC58X_IRQ_SPI2_RXDMA        (32 +  69)  // SPI2RX DMA Channel 27
#define SC58X_IRQ_SPI2_STAT         (32 +  70)  // SPI2Status
#define SC58X_IRQ_SPI2_ERR          (32 +  71)  // SPI2Error
#define SC58X_IRQ_TIMER0_TMR4       (32 +  72)  // TIMER0Timer 4
#define SC58X_IRQ_TIMER0_TMR5       (32 +  73)  // TIMER0Timer 5
#define SC58X_IRQ_TIMER0_TMR6       (32 +  74)  // TIMER0Timer 6
#define SC58X_IRQ_TIMER0_TMR7       (32 +  75)  // TIMER0Timer 7
#define SC58X_IRQ_TIMER0_STAT       (32 +  76)  // TIMER0Status
#define SC58X_IRQ_LP0_DMA           (32 +  77)  // LP0DMA Channel 30
#define SC58X_IRQ_LP0_STAT          (32 +  78)  // LP0Status
#define SC58X_IRQ_LP1_DMA           (32 +  79)  // LP1DMA Channel 36
#define SC58X_IRQ_LP1_STAT          (32 +  80)  // LP1Status
#define SC58X_IRQ_EPPI0_CH0_DMA     (32 +  81)  // EPPI0Channel 0 DMA 28
#define SC58X_IRQ_EPPI0_CH1_DMA     (32 +  82)  // EPPI0Channel 1 DMA 29
#define SC58X_IRQ_EPPI0_STAT        (32 +  83)  // EPPI0Status
#define SC58X_IRQ_CAN0_RX           (32 +  84)  // CAN0Receive
#define SC58X_IRQ_CAN0_TX           (32 +  85)  // CAN0Transmit
#define SC58X_IRQ_CAN0_STAT         (32 +  86)  // CAN0Status
#define SC58X_IRQ_CAN1_RX           (32 +  87)  // CAN1Receive
#define SC58X_IRQ_CAN1_TX           (32 +  88)  // CAN1Transmit
#define SC58X_IRQ_CAN1_STAT         (32 +  89)  // CAN1Status
#define SC58X_IRQ_SPORT2_A_DMA      (32 +  90)  // SPORT2Channel A DMA 4
#define SC58X_IRQ_SPORT2_A_STAT     (32 +  91)  // SPORT2Channel A Status
#define SC58X_IRQ_SPORT2_B_DMA      (32 +  92)  // SPORT2Channel B DMA 5
#define SC58X_IRQ_SPORT2_B_STAT     (32 +  93)  // SPORT2Channel B Status
#define SC58X_IRQ_SPORT3_A_DMA      (32 +  94)  // SPORT3Channel A DMA 6
#define SC58X_IRQ_SPORT3_A_STAT     (32 +  95)  // SPORT3Channel A Status
#define SC58X_IRQ_SPORT3_B_DMA      (32 +  96)  // SPORT3Channel B DMA 7
#define SC58X_IRQ_SPORT3_B_STAT     (32 +  97)  // SPORT3Channel B Status
#define SC58X_IRQ_SPORT6_A_DMA      (32 +  98)  // SPORT6Channel A DMA 14
#define SC58X_IRQ_SPORT6_A_STAT     (32 +  99)  // SPORT6Channel A Status
#define SC58X_IRQ_SPORT6_B_DMA      (32 + 100)  // SPORT6Channel B DMA 15
#define SC58X_IRQ_SPORT6_B_STAT     (32 + 101)  // SPORT6Channel B Status
#define SC58X_IRQ_SPORT7_A_DMA      (32 + 102)  // SPORT7Channel A DMA 16
#define SC58X_IRQ_SPORT7_A_STAT     (32 + 103)  // SPORT7Channel A Status
#define SC58X_IRQ_SPORT7_B_DMA      (32 + 104)  // SPORT7Channel B DMA 17
#define SC58X_IRQ_SPORT7_B_STAT     (32 + 105)  // SPORT7Channel B Status
#define SC58X_IRQ_SPI0_TXDMA        (32 + 106)  // SPI0TX DMA Channel 22
#define SC58X_IRQ_SPI0_RXDMA        (32 + 107)  // SPI0RX DMA Channel 23
#define SC58X_IRQ_SPI0_STAT         (32 + 108)  // SPI0Status
#define SC58X_IRQ_SPI0_ERR          (32 + 109)  // SPI0Error
#define SC58X_IRQ_SPI1_TXDMA        (32 + 110)  // SPI1TX DMA Channel 24
#define SC58X_IRQ_SPI1_RXDMA        (32 + 111)  // SPI1RX DMA Channel 25
#define SC58X_IRQ_SPI1_STAT         (32 + 112)  // SPI1Status
#define SC58X_IRQ_SPI1_ERR          (32 + 113)  // SPI1Error
#define SC58X_IRQ_UART0_TXDMA       (32 + 114)  // UART0Transmit DMA 20
#define SC58X_IRQ_UART0_RXDMA       (32 + 115)  // UART0Receive DMA 21
#define SC58X_IRQ_UART0_STAT        (32 + 116)  // UART0Status
#define SC58X_IRQ_UART1_TXDMA       (32 + 117)  // UART1Transmit DMA 34
#define SC58X_IRQ_UART1_RXDMA       (32 + 118)  // UART1Receive DMA 35
#define SC58X_IRQ_UART1_STAT        (32 + 119)  // UART1Status
#define SC58X_IRQ_UART2_TXDMA       (32 + 120)  // UART2Transmit DMA 37
#define SC58X_IRQ_UART2_RXDMA       (32 + 121)  // UART2Receive DMA 38
#define SC58X_IRQ_UART2_STAT        (32 + 122)  // UART2Status
#define SC58X_IRQ_TWI0_DATA         (32 + 123)  // TWI0Data Interrupt
#define SC58X_IRQ_TWI1_DATA         (32 + 124)  // TWI1Data Interrupt
#define SC58X_IRQ_TWI2_DATA         (32 + 125)  // TWI2Data Interrupt
#define SC58X_IRQ_CNT0_STAT         (32 + 126)  // CNT0Status
#define SC58X_IRQ_CTI1_EVT0         (32 + 127)  // CTI1Core 1 CTI Event
#define SC58X_IRQ_CTI2_EVT0         (32 + 128)  // CTI2Core 2 CTI Event
#define SC58X_IRQ_PKIC0_IRQ         (32 + 129)  // PKIC0Interrupt
#define SC58X_IRQ_PKTE0_IRQ         (32 + 130)  // PKTE0Interrupt
#define SC58X_IRQ_MSI0_STAT         (32 + 131)  // MSI0Status
#define SC58X_IRQ_USB0_STAT         (32 + 132)  // USB0Status/FIFO Data Ready
#define SC58X_IRQ_USB0_DATA         (32 + 133)  // USB0DMA Status/Transfer Complete
#define SC58X_IRQ_USB1_STAT         (32 + 134)  // USB1Status/FIFO Data Ready
#define SC58X_IRQ_USB1_DATA         (32 + 135)  // USB1DMA Status/Transfer Complete
#define SC58X_IRQ_TRU0_SLV4         (32 + 136)  // TRU0Interrupt 4 -- Core 1 only
#define SC58X_IRQ_TRU0_SLV5         (32 + 137)  // TRU0Interrupt 5 -- Core 1 only
#define SC58X_IRQ_TRU0_SLV6         (32 + 138)  // TRU0Interrupt 6 -- Core 1 only
#define SC58X_IRQ_TRU0_SLV7         (32 + 139)  // TRU0Interrupt 7 -- Core 1 only
#define SC58X_IRQ_TRU0_SLV8         (32 + 140)  // TRU0Interrupt 8 -- Core 2 only
#define SC58X_IRQ_TRU0_SLV9         (32 + 141)  // TRU0Interrupt 9 -- Core 2 only
#define SC58X_IRQ_TRU0_SLV10        (32 + 142)  // TRU0Interrupt 10 -- Core 2 only
#define SC58X_IRQ_TRU0_SLV11        (32 + 143)  // TRU0Interrupt 11 -- Core 2 only
#define SC58X_IRQ_SINC0_STAT        (32 + 144)  // SINC0Status
#define SC58X_IRQ_DAI0_IRQL         (32 + 145)  // DAI0Low Priority Interrupt
#define SC58X_IRQ_DAI1_IRQL         (32 + 146)  // DAI1Low Priority Interrupt
#define SC58X_IRQ_M6CPH0_IRQ        (32 + 147)  // M6CPH0M6 Cipher Interrupt
#define SC58X_IRQ_EMAC0_STAT        (32 + 148)  // EMAC0Status
#define SC58X_IRQ_EMAC1_STAT        (32 + 149)  // EMAC1Status
#define SC58X_IRQ_FFTA0_TXDMA       (32 + 150)  // FFTA0Transmit DMA 41
#define SC58X_IRQ_FFTA0_RXDMA       (32 + 151)  // FFTA0Receive DMA 42
#define SC58X_IRQ_FFTA0_STAT        (32 + 152)  // FFTA0Status
#define SC58X_IRQ_FIR0_DMA          (32 + 153)  // FIR0DMA
#define SC58X_IRQ_FIR0_STAT         (32 + 154)  // FIR0Status
#define SC58X_IRQ_IIR0_DMA          (32 + 155)  // IIR0DMA
#define SC58X_IRQ_IIR0_STAT         (32 + 156)  // IIR0Status
#define SC58X_IRQ_HADC0_EVT         (32 + 157)  // HADC0Event
#define SC58X_IRQ_HAE0_RXDMA_CH0    (32 + 158)  // HAE0RX DMA Channel 0
#define SC58X_IRQ_HAE0_RXDMA_CH1    (32 + 159)  // HAE0RX DMA Channel 1
#define SC58X_IRQ_HAE0_TXDMA        (32 + 160)  // HAE0TX DMA Channel 0
#define SC58X_IRQ_HAE0_STAT         (32 + 161)  // HAE0Status
#define SC58X_IRQ_MLB0_INT0         (32 + 162)  // MLB0Interrupt 0
#define SC58X_IRQ_MLB0_INT1         (32 + 163)  // MLB0Interrupt 1
#define SC58X_IRQ_MLB0_STAT         (32 + 164)  // MLB0Status
#define SC58X_IRQ_RTC0_EVT          (32 + 165)  // RTC0Event
#define SC58X_IRQ_MDMA3_SRC         (32 + 166)  // Memory DMA Stream 3 Source Channel
#define SC58X_IRQ_MDMA3_DST         (32 + 167)  // Memory DMA Stream 3 Destination Channel
#define SC58X_IRQ_MDMA2_SRC         (32 + 168)  // Memory DMA Stream 2 Source Channel
#define SC58X_IRQ_MDMA2_DST         (32 + 169)  // Memory DMA Stream 2 Destination Channel
#define SC58X_IRQ_EMDMA0_DONE       (32 + 170)  // EMDMA0Transfer Done
#define SC58X_IRQ_EMDMA1_DONE       (32 + 171)  // EMDMA1Transfer Done
#define SC58X_IRQ_MDMA0_SRC         (32 + 172)  // Memory DMA Stream 0 Source Channel
#define SC58X_IRQ_MDMA0_DST         (32 + 173)  // Memory DMA Stream 0 Destination Channel
#define SC58X_IRQ_MDMA1_SRC         (32 + 174)  // Memory DMA Stream 1 Source Channel
#define SC58X_IRQ_MDMA1_DST         (32 + 175)  // Memory DMA Stream 1 Destination Channel
#define SC58X_IRQ_CRC0_DCNTEXP      (32 + 176)  // CRC0Datacount expiration
#define SC58X_IRQ_CRC1_DCNTEXP      (32 + 177)  // CRC1Datacount expiration
#define SC58X_IRQ_CRC0_ERR          (32 + 178)  // CRC0Error
#define SC58X_IRQ_CRC1_ERR          (32 + 179)  // CRC1Error
#define SC58X_IRQ_SPORT0_A_DMA_ERR  (32 + 180)  // SPORT0Channel A DMA Error
#define SC58X_IRQ_SPORT0_B_DMA_ERR  (32 + 181)  // SPORT0Channel B DMA Error
#define SC58X_IRQ_SPORT1_A_DMA_ERR  (32 + 182)  // SPORT1Channel A DMA Error
#define SC58X_IRQ_SPORT1_B_DMA_ERR  (32 + 183)  // SPORT1Channel B DMA Error
#define SC58X_IRQ_SPORT4_A_DMA_ERR  (32 + 184)  // SPORT4Channel A DMA Error
#define SC58X_IRQ_SPORT4_B_DMA_ERR  (32 + 185)  // SPORT4Channel B DMA Error
#define SC58X_IRQ_SPORT5_A_DMA_ERR  (32 + 186)  // SPORT5Channel A DMA Error
#define SC58X_IRQ_SPORT5_B_DMA_ERR  (32 + 187)  // SPORT5Channel B DMA Error
#define SC58X_IRQ_SPI2_TXDMA_ERR    (32 + 188)  // SPI2TX DMA Channel Error
#define SC58X_IRQ_SPI2_RXDMA_ERR    (32 + 189)  // SPI2RX DMA Channel Error
#define SC58X_IRQ_SPORT2_A_DMA_ERR  (32 + 190)  // SPORT2Channel A DMA Error
#define SC58X_IRQ_SPORT2_B_DMA_ERR  (32 + 191)  // SPORT2Channel B DMA Error
#define SC58X_IRQ_SPORT3_A_DMA_ERR  (32 + 192)  // SPORT3Channel A DMA Error
#define SC58X_IRQ_SPORT3_B_DMA_ERR  (32 + 193)  // SPORT3Channel B DMA Error
#define SC58X_IRQ_SPORT6_A_DMA_ERR  (32 + 194)  // SPORT6Channel A DMA Error
#define SC58X_IRQ_SPORT6_B_DMA_ERR  (32 + 195)  // SPORT6Channel B DMA Error
#define SC58X_IRQ_SPORT7_A_DMA_ERR  (32 + 196)  // SPORT7Channel A DMA Error
#define SC58X_IRQ_SPORT7_B_DMA_ERR  (32 + 197)  // SPORT7Channel B DMA Error
#define SC58X_IRQ_SPI0_TXDMA_ERR    (32 + 198)  // SPI0TX DMA Channel Error
#define SC58X_IRQ_SPI0_RXDMA_ERR    (32 + 199)  // SPI0RX DMA Channel Error
#define SC58X_IRQ_SPI1_TXDMA_ERR    (32 + 200)  // SPI1TX DMA Channel Error
#define SC58X_IRQ_SPI1_RXDMA_ERR    (32 + 201)  // SPI1RX DMA Channel Error
#define SC58X_IRQ_UART0_TXDMA_ERR   (32 + 202)  // UART0Transmit DMA Error
#define SC58X_IRQ_UART0_RXDMA_ERR   (32 + 203)  // UART0Receive DMA Error
#define SC58X_IRQ_UART1_TXDMA_ERR   (32 + 204)  // UART1Transmit DMA Error
#define SC58X_IRQ_UART1_RXDMA_ERR   (32 + 205)  // UART1Receive DMA Error
#define SC58X_IRQ_UART2_TXDMA_ERR   (32 + 206)  // UART2Transmit DMA Error
#define SC58X_IRQ_UART2_RXDMA_ERR   (32 + 207)  // UART2Receive DMA Error
#define SC58X_IRQ_LP0_DMA_ERR       (32 + 208)  // LP0DMA Data Error
#define SC58X_IRQ_LP1_DMA_ERR       (32 + 209)  // LP1DMA Data Error
#define SC58X_IRQ_FFTA0_TXDMA_ERR   (32 + 210)  // FFTA0Transmit DMA Error
#define SC58X_IRQ_FFTA0_RXDMA_ERR   (32 + 211)  // FFTA0Receive DMA Error
#define SC58X_IRQ_HAE0_RXDMA_CH0_ERR (32+ 212)  // HAE0RX DMA Channel 0 Error
#define SC58X_IRQ_HAE0_RXDMA_CH1_ERR (32+ 213)  // HAE0RX DMA Channel 1 Error
#define SC58X_IRQ_HAE0_TXDMA_ERR    (32 + 214)  // HAE0TX DMA Channel Error
#define SC58X_IRQ_EPPI0_CH0_DMA_ERR (32 + 215)  // EPPI0DMA Channel 0 Error
#define SC58X_IRQ_EPPI0_CH1_DMA_ERR (32 + 216)  // EPPI0DMA Channel 1 Error
#define SC58X_IRQ_MDMA0_SRC_ERR     (32 + 217)  // Standard BW MDMA0 Channel 0 (Source) Error
#define SC58X_IRQ_MDMA0_DST_ERR     (32 + 218)  // Standard BW MDMA0 Channel 1 (Destination) Error
#define SC58X_IRQ_MDMA1_SRC_ERR     (32 + 219)  // Standard BW MDMA1 Channel 0 (Source) Error
#define SC58X_IRQ_MDMA1_DST_ERR     (32 + 220)  // Standard BW MDMA1 Channel 1 (Destination) Error
#define SC58X_IRQ_MDMA2_SRC_ERR     (32 + 221)  // Enhanced BW MDMA2 Channel 0 (Source) Error
#define SC58X_IRQ_MDMA2_DST_ERR     (32 + 222)  // Enhanced BW MDMA2 Channel 1 (Destination) Error
#define SC58X_IRQ_MDMA3_SRC_ERR     (32 + 223)  // Max BW MDMA3 Channel 0 (Source) Error
#define SC58X_IRQ_MDMA3_DST_ERR     (32 + 224)  // Max BW MDMA3 Channel 1 (Destination) Error
#define SC58X_IRQ_SWU0_EVT          (32 + 225)  // SWU0Event
#define SC58X_IRQ_SWU2_EVT          (32 + 226)  // SWU2Event
#define SC58X_IRQ_SWU1_EVT          (32 + 227)  // SWU1Event
#define SC58X_IRQ_SWU4_EVT          (32 + 228)  // SWU4Event
#define SC58X_IRQ_SWU3_EVT          (32 + 229)  // SWU3Event
#define SC58X_IRQ_SWU6_EVT          (32 + 230)  // SWU6Event
#define SC58X_IRQ_SWU5_EVT          (32 + 231)  // SWU5Event
#define SC58X_IRQ_SWU7_EVT          (32 + 232)  // SWU7Event
#define SC58X_IRQ_SWU8_EVT          (32 + 233)  // SWU8Event
#define SC58X_IRQ_SWU9_EVT          (32 + 234)  // SWU9Event
#define SC58X_IRQ_SWU10_EVT         (32 + 235)  // SWU10Event
#define SC58X_IRQ_SWU11_EVT         (32 + 236)  // SWU11Event
#define SC58X_IRQ_SWU12_EVT         (32 + 237)  // SWU12Event
#define SC58X_IRQ_SWU13_EVT         (32 + 238)  // SWU13Event
#define SC58X_IRQ_SWU14_EVT         (32 + 239)  // SWU14Event
#define SC58X_IRQ_SWU15_EVT         (32 + 240)  // SWU15Event
#define SC58X_IRQ_SPU0_INT          (32 + 241)  // SPU0Interrupt
#define SC58X_IRQ_SMPU_AGGR_INT     (32 + 242)  // SMPU Aggregated Interrupt/Event
#define SC58X_IRQ_PCIE0_RESET       (32 + 243)  // PCIE0Reset
#define SC58X_IRQ_PCIE0_STAT        (32 + 244)  // PCIE0Status
#define SC58X_IRQ_PCIE0_DMA         (32 + 245)  // PCIE0DMA Completion
#define SC58X_IRQ_EMAC0_PWR         (32 + 246)  // EMAC0Power
#define SC58X_IRQ_EMAC1_PWR         (32 + 247)  // EMAC1Power
#define SC58X_IRQ_TRU0_SLV0         (32 + 248)  // TRU0Interrupt 0 -- Core 0 only
#define SC58X_IRQ_TRU0_SLV1         (32 + 249)  // TRU0Interrupt 1 -- Core 0 only
#define SC58X_IRQ_TRU0_SLV2         (32 + 250)  // TRU0Interrupt 2 -- Core 0 only
#define SC58X_IRQ_TRU0_SLV3         (32 + 251)  // TRU0Interrupt 3 -- Core 0 only
#define SC58X_IRQ_C0_PMUIRQ         (32 + 253)  // Performance Monitoring Unit Interrupt (Core 0 only)

#endif /* __ARM_SC58X_H_INCLUDED */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/lib/public/arm/sc58x.h $ $Rev: 802303 $")
#endif
