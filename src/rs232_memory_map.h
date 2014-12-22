#include <linux/kernel.h>		
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/errno.h>	
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/completion.h>
#include <linux/ioctl.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define TRUE 1
#define FALSE 0

// UART register addresses
#define	UART_RX_DATA	(UART_BASE_ADDRRESS + 0)    //R
#define	UART_TX_DATA	(UART_BASE_ADDRRESS + 0)    //W
#define	UART_DLATCH_LO	(UART_BASE_ADDRRESS + 0)    //R/W uticu na 
#define	UART_DLATCH_HI	(UART_BASE_ADDRRESS + 0x04) //R/W brzinu prenosa
#define	UART_INTR_EN	(UART_BASE_ADDRRESS + 0x04) //R/W disable-polling 
#define	UART_INTR_ID	(UART_BASE_ADDRRESS + 0x08) //R   id prekida, id uart chipa
#define	UART_FIFO_CTRL	(UART_BASE_ADDRRESS + 0x08) //W	  enable-polling
#define	UART_LINE_CTRL	(UART_BASE_ADDRRESS + 0x0C) //->
#define	UART_MODEM_CTRL	(UART_BASE_ADDRRESS + 0x10) //kontrolisanje modema -->
#define	UART_LINE_STAT	(UART_BASE_ADDRRESS + 0x14) //stanje UART-a --->
#define	UART_MODEM_STAT (UART_BASE_ADDRRESS + 0x18) //stanje modema ---->
#define	UART_SCRATCH	(UART_BASE_ADDRRESS + 0x1C) //

// line status register bits --->
#define DATA_READY_BIT                  0x01
#define OVERRUN_ERROR_BIT               0x02
#define PARITY_ERROR_BIT                0x04
#define FRAMING_ERROR_BIT               0x08
#define BREAK_INTERRUPT_BIT             0x10
#define TX_HOLDING_REGISTER_EMPTY_BIT   0x20
#define TRANSMITTER_EMPTY_BIT           0x40
#define RCVR_FIFO_ERROR_BIT             0x80

// line control register bits ->
#define WORD_LENGTH_5           0x00
#define WORD_LENGTH_6           0x01
#define WORD_LENGTH_7           0x02
#define WORD_LENGTH_8           0x03
#define STOP_BITS_SELECT_BIT    0x04
#define PARITY_ENABLE_BIT       0x08
#define EVEN_PARITY_SELECT_BIT  0x10
#define STICK_PARITY_BIT        0x20
#define SET_BREAK_BIT           0x40
#define DIVISOR_ACCESS_BIT      0x80/*omogucava da se preko iste mem. lok pristupi razl. reg*/

// modem control register bits -->
#define DATA_TERMINAL_READY_BIT 0x01 
#define REQUEST_TO_SEND_BIT     0x02
#define OUT1_BIT                0x04
#define OUT2_BIT                0x08
#define LOOP_BIT                0x10

// modem status register bits ---->
#define DCTS_BIT    0x01
#define DDSR_BIT    0x02
#define TERI_BIT    0x04
#define DDCD_BIT    0x08
#define CTS_BIT     0x10
#define DSR_BIT     0x20
#define RI_BIT      0x40
#define DCD_BIT     0x80

// FIFO control register bits
#define FIFO_ENABLE_BIT         0x01
#define RCVR_FIFO_RESET_BIT     0x02
#define XMIT_FIFO_RESET_BIT     0x04
#define DMA_MODE_SELECT_BIT     0x08
#define RCVR_TRIGGER_BIT1       0x40
#define RCVR_TRIGGER_BIT2       0x80

// Interrupt enable register
#define DISABLE_ALL                         0x00
#define ENABLE_ALL                          0x0F
#define ENABLE_RECEIVED_DATA_AVAILABLE      0x01
#define ENABLE_TX_HOLDING_REGISTER_EMPTY    0x02    
#define ENABLE_LINE_STATUS_ERROR            0x04
#define ENABLE_MODEM_STATUS_CHANGED         0x08

// Interrupt ID register
#define INTERRUPT_ID_BIT_MASK           0x0F
#define NO_INTERRUPT_PENDING            0x01
#define RECEIVER_LINE_STATUS_INT        0x06
#define RECEIVED_DATA_AVAILABLE_INT     0x04
#define CHARACTER_TIMEOUT_INT           0x0C
#define TX_HOLDING_REGISTER_EMPTY_INT   0x02
#define MODEM_STATUS_CHANGED_INT        0x00

#define UART_CLOCK 25000000
#define UART_BAUD 115200 
#define BAUD_DIVISOR ((UART_CLOCK/UART_BAUD+8)/16)

#define UART_IRQ	201	// UART's Interrupt Request Number
static unsigned long UART_BASE_ADDRRESS = 0xF7FCA000; // UART's base address
