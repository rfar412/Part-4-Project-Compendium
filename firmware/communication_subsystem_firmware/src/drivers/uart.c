
#include "uart.h"

#define UART_MAX_MSG_LEN 64

static volatile char uart_rx_buffer[UART_BUFFER_SIZE];
static volatile unsigned int uart_rx_index = 0;
volatile uint8_t uart_message_ready = 0;

// UART ISR
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    switch (__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG)) {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG: {
            char c = UCA0RXBUF;
            if (c == '\r' || c == '\n') {
                uart_rx_buffer[uart_rx_index] = '\0';
                send_message((char *)uart_rx_buffer);  // Call your existing function
                uart_send_string("\r\nMessage sent: ");
                uart_send_string((char *)uart_rx_buffer);
                uart_send_string("\r\n");

                // Binary form
                uart_send_string("Binary form: ");
                unsigned int i;
                for (i = 0; i < uart_rx_index; i++) {
                    uart_send_binary_char(uart_rx_buffer[i]);
                    uart_send_char(' '); // spacing between bytes
                }
                uart_send_string("\r\n");

                uart_rx_index = 0;  // reset buffer
            } else if (c == '\b' || c == 0x7F) {
                // Handle backspace/delete
                if (uart_rx_index > 0) {
                    uart_rx_index--;
                    // Echo backspace sequence to erase on terminal
                    uart_send_string("\b \b");
                }
            } else {
                if (uart_rx_index < UART_BUFFER_SIZE - 1) {
                    uart_rx_buffer[uart_rx_index++] = c;
                    uart_send_char(c);
                }
            }
            break;
        }
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
    }
}

void uart_init(void) {
    UCA0CTLW0 |= UCSWRST;          // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;    // SMCLK source

    // Default to 115200 baud @16MHz
    UCA0BRW = 8;
    UCA0MCTLW = 0xD600;

    // Configure pins P1.4 (TX), P1.5 (RX)
    P1SEL1 &= ~(BIT4 | BIT5);
    P1SEL0 |= (BIT4 | BIT5);

    UCA0CTLW0 &= ~UCSWRST;         // Release eUSCI for operation
    UCA0IE |= UCRXIE;              // Enable RX interrupt
}

void uart_send_char(char c) {
    while (!(UCA0IFG & UCTXIFG));  // Wait for TX buffer ready
    UCA0TXBUF = c;
}

void uart_send_string(const char *str) {
    while (*str) {
        uart_send_char(*str++);
    }
} 

void uart_send_num(unsigned int value) {
    char buf[8];
    sprintf(buf, "%d\r\n", value);
    char *p;
    for (*p = buf; *p; p++) {
        while (!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = *p;
    }
}

static void uart_send_binary_char(char c) {
    int i;
    for (i = 6; i >= 0; i--) {
        uart_send_char((c & (1 << i)) ? '1' : '0');
    }
} 

void uint_to_str(unsigned int num, char *buf) {
    char temp[6];
    int i = 0;
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while (num > 0 && i < 5) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }
    int j = 0;
    while (i > 0) buf[j++] = temp[--i];
    buf[j] = '\0';
}
