#ifndef RPI_UART_H
#define RPI_UART_H

int rpi_uart_setup(int*);
int rpi_uart_send_byte(int*, char*);
int rpi_uart_send_string(int*, char*);
int rpi_uart_send_newline(int*);
int rpi_uart_receive_bytes(int*, int*, char*, const int);
int rpi_uart_close(int*);

#endif // RPI_UART_H
