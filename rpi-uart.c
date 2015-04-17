#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

// TODO make this a commandline parameter, and default to 9600 instead of being forced to 9600
#define BAUDRATE B9600

extern int DEBUG;

#define BUFFERSIZE 1024
//#define MODEMDEVICE "/dev/ttyAMA0" // default file to access raspberry pi GPIO UART
char MODEMDEVICE[BUFFERSIZE];

// raspberry pi uart functions

int rpi_uart_setup(int * filestream) {
	//-------------------------
	//----- SETUP USART 0 -----
	//-------------------------
	//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
	//int uart0_filestream = -1;

	if (DEBUG) {
		printf("\nserial port settings BEFORE running setup function:\n");
		system("stty -F /dev/ttyAMA0");
		printf("\n\n");
	}

	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	*filestream = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NDELAY);//Open in non blocking read/write mode
	if (*filestream == -1) {
		//ERROR - CAN'T OPEN SERIAL PORT
		perror("Unable to open UART device. - ");
	}

	//CONFIGURE THE UART
	//The flags are defined in termios.h
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for binary comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;

	if (tcgetattr(*filestream, &options) == -1) {
		perror("reading the attributes of the UART port failed.");
		return -1;
	}

	/*
	 * an alternate/simplified method to setup the flags
	 options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;		//<Set baud rate
	 options.c_iflag = IGNPAR;
	 options.c_oflag = 0;
	 options.c_lflag = 0;
	 */

	/* input flags */
	options.c_iflag &= ~ IGNBRK; /* enable ignoring break */
	options.c_iflag &= ~(IGNPAR | PARMRK); /* disable parity checks */
	options.c_iflag &= ~ INPCK; /* disable parity checking */
	options.c_iflag &= ~ ISTRIP; /* disable stripping 8th bit */
	options.c_iflag &= ~(INLCR | ICRNL); /* disable translating NL <-> CR */
	options.c_iflag &= ~ IGNCR; /* disable ignoring CR */
	options.c_iflag &= ~(IXON | IXOFF); /* disable XON/XOFF flow control */
	/* output flags */
	options.c_oflag &= ~ OPOST; /* disable output processing */
	options.c_oflag &= ~(ONLCR | OCRNL); /* disable translating NL <-> CR */
	/* not for FreeBSD */
	options.c_oflag &= ~ OFILL; /* disable fill characters */
	/* control flags */
	options.c_cflag |= CLOCAL; /* prevent changing ownership */
	options.c_cflag |= CREAD; /* enable reciever */
	options.c_cflag &= ~ PARENB; /* disable parity */
	options.c_cflag &= ~ CSTOPB; /* disable 2 stop bits */
	options.c_cflag &= ~ CSIZE; /* remove size flag... */
	options.c_cflag |= CS8; /* ...enable 8 bit characters */
	options.c_cflag |= HUPCL; /* enable lower control lines on close - hang up */

	/* flow control enable / disable (pick one)*/
	//options.c_cflag |= CRTSCTS; /* enable hardware CTS/RTS flow control */
	options.c_cflag &= ~ CRTSCTS; /* disable hardware CTS/RTS flow control */

	/* local flags */
	options.c_lflag &= ~ ISIG; /* disable generating signals */
	options.c_lflag &= ~ ICANON; /* disable canonical mode - line by line */
	options.c_lflag &= ~ ECHO; /* disable echoing characters */
	options.c_lflag &= ~ ECHONL; /* ??? */
	options.c_lflag &= ~ NOFLSH; /* disable flushing on SIGINT */
	options.c_lflag &= ~ IEXTEN; /* disable input processing */

	/* control characters */
	memset(options.c_cc, 0, sizeof(options.c_cc));

	/* set i/o baud rate */
	if (cfsetspeed(&options, BAUDRATE) == -1) {
		perror("cfsetspeed on UART port failed. Verify baud rate selection.");
		return -1;
	}

	// write the attributes to the port
	if (tcsetattr(*filestream, TCSAFLUSH, &options) == -1) {
		perror("setting attributes on the UART port failed.");
		return -1;
	}

	/* enable input & output transmission */
	if (tcflow(*filestream, TCOON | TCION) == -1) {
		perror("enable the UART port failed.");
		return -1;
	}

	// flush input and output buffers
	if (tcflush(*filestream, TCIOFLUSH) == -1) {
		perror("flushing the UART buffers failed.");
		return -1;
	}

	if (DEBUG) {
		printf("serial port settings AFTER running setup function:\n");
		system("stty -F /dev/ttyAMA0");
		printf("\n\n");
	}

	return 0;
}

int rpi_uart_send_byte(int * filestream, char * byte) {
	// transmit one byte

	int count = -1;

	if (*filestream != -1) {
		count = write(*filestream, (void *) byte, (size_t) 1); //Filestream, bytes to write, number of bytes to write
		if (count < 0) {
			perror("UART transmit error - ");
		}
	} else {
		perror(
				"Unable to use port. filestream (filedes=-1) error when trying to send a byte ");
	}
	return count;
}

int rpi_uart_send_string(int * filestream, char * string_to_send) {
	// transmit an entire string

	int count = -1;

	if (*filestream != -1) {
		count = write(*filestream, (void *) string_to_send, strlen(string_to_send)); //Filestream, bytes to write, number of bytes to write
		if (count < 0) {
			perror("UART transmit error writing string to UART - ");
		}
	} else {
		perror(
				"Unable to use port. filestream (filedes=-1) error when trying to send a byte ");
	}
	return count;
}

int rpi_uart_send_newline(int * filestream) {
	// transmit just a newline character

	char newline = '\n';
	int retval = -1;
	retval = rpi_uart_send_byte(filestream, &newline);
	if (retval < 0) {
		perror("UART transmit error writing string to UART ");
	}
	return retval;
}

int rpi_uart_receive_bytes(int * filestream, int * rx_length, char * rx_buffer,
		const int BUFFERMAXSIZE) {

	// CHECK FOR ANY RX BYTES

	*rx_length = 0;

	if (*filestream != -1) {
		// Read up to 255 characters from the port if they are there
		//unsigned char rx_buffer[BUFFERMAXSIZE];
		int rx_byte_count = read(*filestream, (void*) rx_buffer,
				(BUFFERMAXSIZE - 1)); //Filestream, buffer to store in, number of bytes to read (max)
		if (rx_byte_count < 0) {
			//An error occured (will occur if there are no bytes)
		} else if (rx_byte_count == 0) {
			//No data waiting
		} else {
			//Bytes received
			*rx_length = rx_byte_count;
			if (DEBUG) {
				printf("%i RAW bytes read : %s\n", rx_byte_count, rx_buffer);
			}
		}
	}
	return *rx_length;
}

int rpi_uart_close(int * filestream) {
	//----- CLOSE THE UART -----
	return close(*filestream);
}
