/*
 * GPIO-serial.c
 *
 *  Created on: AApril 02, 2015
 *      Author: monk
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define valueLength 9
#define numberOfValues 9  // was 9 in standards control program
#define valueNameLength 12

struct _CILOG {
	char messageType[6]; //up to 5 characters long, additional for null character
	char CIGI_part[10];
	char deviceName[19]; //up to 18 characters long, additional for null character
	char valueName[numberOfValues][valueNameLength]; //8 names, each up to 12 characters long, additional for null character
	char valueData[numberOfValues][valueLength]; //8 values, up to 8 characters in length, additional for null character
	char checksum[3]; //two character checksum, represented in hex. Third character to be used as null character
	int isValid;
};

//A typedef is required so I don't need to repeat the "struct" keyword for C:
typedef struct _CILOG CILOG;

// raspberry pi uart functions
int rpi_uart_setup(void);
int rpi_uart_send_bytes(int data);
int rpi_uart_receive_bytes(int data) ;
int rpi_uart_close(void);

int calculateChecksum(char*);
int checksum(char*);
char lowerNibbleHex(int);
char upperNibbleHex(int);
char returnNibbleHex(int byteValue);
int processIncomingPacket(char, CILOG *);

int main(int argc, char ** argv) {
	CILOG packet_buffer;
	int fd;
	//speed_t baud;
	// Open the Port. We want read/write, no "controlling tty" status, and open it no matter what state DCD is in
	fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		perror("open_port: Unable to open /dev/ttyAMA0 - ");
		return (-1);
	}

	struct termios tc;
	//tcgetattr(fd, &tc);
	//cfsetispeed(&tc, B115200);
	//cfsetospeed(&tc, B115200);
	//tcsetattr(fd, TCSANOW, &tc);

	if (tcgetattr(fd, &tc)) {
		return -1;
	}

	/* input flags */
	tc.c_iflag &= ~ IGNBRK; /* enable ignoring break */
	tc.c_iflag &= ~(IGNPAR | PARMRK); /* disable parity checks */
	tc.c_iflag &= ~ INPCK; /* disable parity checking */
	tc.c_iflag &= ~ ISTRIP; /* disable stripping 8th bit */
	tc.c_iflag &= ~(INLCR | ICRNL); /* disable translating NL <-> CR */
	tc.c_iflag &= ~ IGNCR; /* disable ignoring CR */
	tc.c_iflag &= ~(IXON | IXOFF); /* disable XON/XOFF flow control */
	/* output flags */
	tc.c_oflag &= ~ OPOST; /* disable output processing */
	tc.c_oflag &= ~(ONLCR | OCRNL); /* disable translating NL <-> CR */
	/* not for FreeBSD */
	tc.c_oflag &= ~ OFILL; /* disable fill characters */
	/* control flags */
	tc.c_cflag |= CLOCAL; /* prevent changing ownership */
	tc.c_cflag |= CREAD; /* enable reciever */
	tc.c_cflag &= ~ PARENB; /* disable parity */
	tc.c_cflag &= ~ CSTOPB; /* disable 2 stop bits */
	tc.c_cflag &= ~ CSIZE; /* remove size flag... */
	tc.c_cflag |= CS8; /* ...enable 8 bit characters */
	tc.c_cflag |= HUPCL; /* enable lower control lines on close - hang up */
	/*
	 if (fRtsCts) {
	 tc.c_cflag |=   CRTSCTS;                  // enable hardware CTS/RTS flow control
	 } else {
	 tc.c_cflag &= ~ CRTSCTS;                  // disable hardware CTS/RTS flow control
	 } */

	/* local flags */
	tc.c_lflag &= ~ ISIG; /* disable generating signals */
	tc.c_lflag &= ~ ICANON; /* disable canonical mode - line by line */
	tc.c_lflag &= ~ ECHO; /* disable echoing characters */
	tc.c_lflag &= ~ ECHONL; /* ??? */
	tc.c_lflag &= ~ NOFLSH; /* disable flushing on SIGINT */
	tc.c_lflag &= ~ IEXTEN; /* disable input processing */

	/* control characters */
	memset(tc.c_cc, 0, sizeof(tc.c_cc));

	/* set i/o baud rate */
	//cfsetspeed(&tc, baud);
	//tcgetattr(fd, &tc);
	cfsetispeed(&tc, B115200);
	cfsetospeed(&tc, B115200);
	//tcsetattr(fd, TCSANOW, &tc);

	tcsetattr(fd, TCSAFLUSH, &tc);

	/* enable input & output transmission */
	tcflow(fd, TCOON | TCION);

	// Turn off blocking for reads, use (fd, F_SETFL, FNDELAY) if you want that
	fcntl(fd, F_SETFL, 0);

	int index = 0;
	for (index = optind; index < argc; index++) {
		printf("Non-option argument %s\n", argv[index]);
	}

	//system("clear");

	// Write to the port
	int n = write(fd, "Still going!\r\n", 14); // int write(int fd, char *Buff, int NumBytes);
	if (n < 0) {
		perror("Write failed - ");
		return -1;
	}

	n = -1;

	char single_char_buffer[1];
	const int BUFFERSIZE = 256;
    int buffer_position = 0;
	char buffer[BUFFERSIZE];
	int i = 0;
	int finished = 0;

	// example of a valid packet:
	// "$$CILOG,C3766,CurrentRegulator,Anode,1.90mA,Cathode,1.11mA,Setpoint,7.79V,Locked?,No,Error#,0,v.,3.0,Pg,0.01000,Ig,0.00000,Dg,0.00000,*45"

	while (finished == 0) {

		n = read(fd, (void*)single_char_buffer, 1); // returns
		if (n > 0) {
			buffer[buffer_position++] = single_char_buffer[0];
			//system("clear");
			printf("%c\r\n", single_char_buffer[0]);

		}

		if(buffer_position >= BUFFERSIZE) {
			buffer_position = 0;
			//finished = 1;
		}
		/*for (i = 0; i < BUFFERSIZE; i++) {
			if (processIncomingPacket(buffer[i], &packet_buffer) == 1) {
				printf("incoming packet validated OK !\r\n");
				finished = 1;
			}
		}*/

	}

	if (n < 0) {
		perror("Read failed - ");
		return -1;
	} else if (n == 0)
		printf("No data on port\n");
	else {
		single_char_buffer[0] = '\0';
		printf("%i bytes read : %s", n, single_char_buffer);
	}

	// Don't forget to clean up
	close(fd);

	return 0;
}

int rpi_uart_setup(void) {
	;
	//-------------------------
		//----- SETUP USART 0 -----
		//-------------------------
		//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
		int uart0_filestream = -1;

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
		uart0_filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
		if (uart0_filestream == -1) {
			//ERROR - CAN'T OPEN SERIAL PORT
			printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		}

		//CONFIGURE THE UART
		//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
		//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
		//	CSIZE:- CS5, CS6, CS7, CS8
		//	CLOCAL - Ignore modem status lines
		//	CREAD - Enable receiver
		//	IGNPAR = Ignore characters with parity errors
		//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
		//	PARENB - Parity enable
		//	PARODD - Odd parity (else even)
		struct termios options;
		tcgetattr(uart0_filestream, &options);
		options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
		options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag = 0;
		tcflush(uart0_filestream, TCIFLUSH);
		tcsetattr(uart0_filestream, TCSANOW, &options);
}

int rpi_uart_send_bytes(int data) {
	//----- TX BYTES -----
		unsigned char tx_buffer[20];
		unsigned char *p_tx_buffer;

		p_tx_buffer = &tx_buffer[0];
		*p_tx_buffer++ = 'H';
		*p_tx_buffer++ = 'e';
		*p_tx_buffer++ = 'l';
		*p_tx_buffer++ = 'l';
		*p_tx_buffer++ = 'o';

		if (filestream != -1) {
			int count = write(filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));		//Filestream, bytes to write, number of bytes to write
			if (count < 0)
			{
				printf("UART TX error\n");
			}
		}
		return 0;

}

int rpi_uart_receive_bytes(int * filestream) {
	//----- CHECK FOR ANY RX BYTES -----
		if (filestream != -1) {
			// Read up to 255 characters from the port if they are there
			unsigned char rx_buffer[256];
			int rx_length = read(filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
			if (rx_length < 0)
			{
				//An error occured (will occur if there are no bytes)
			}
			else if (rx_length == 0)
			{
				//No data waiting
			}
			else
			{
				//Bytes received
				rx_buffer[rx_length] = '\0';
				printf("%i bytes read : %s\n", rx_length, rx_buffer);
			}
		}
return filestream;
}

int rpi_uart_close(void) {
	//----- CLOSE THE UART -----
		close(uart0_filestream);
		return 0;
}

int processIncomingPacket(char buffer, CILOG *capturedMessage) {

	static int i = 0;
	static int j = 0;
	static int counter = 0;
	static int logChecksumFlag = 0; // if 1, log next two bytes as checksum bytes
	static char checksum = 0;
	static char checksumHighNibble = 0;
	static char checksumLowNibble = 0;
	static int checksumCounter = 0;
	//static int currentValueCount = 0;
	static int currentVariable = 0;
	// static char messageType[5];

	if (buffer == '$' || counter == 0) { // '$' Indicates Start of packet.
		if (buffer == '$') {
			counter = 1;
			logChecksumFlag = 0;
			checksumCounter = 0;
			currentVariable = 0;
			checksum = 0;

			//   capturedMessage = initCILOG();

			for (i = 0; i < 6; i++)
				capturedMessage->messageType[i] = 0;

			capturedMessage->isValid = 0; // Will Return a nonvalid struct if data doesn't check out.
			for (i = 0; i < numberOfValues; i++) { //Initialize buffers
				for (j = 0; j < valueNameLength; j++) //11 name chars, one null
					capturedMessage->valueName[i][j] = 0;
				for (j = 0; j < valueLength; j++) //8 value chars, one null
					capturedMessage->valueData[i][j] = 0;
			}

			for (j = 0; j < 3; j++)
				capturedMessage->checksum[j] = 0;
			for (i = 0; i < 19; i++)
				capturedMessage->deviceName[i] = 0;
			for (i = 0; i < 10; i++)
				capturedMessage->CIGI_part[i] = 0;

			j = 0;

		} else
			counter = 0;
	} else if (logChecksumFlag == 1) { //checksum '*' asterisks has been recieved. Following two chars will contain checksum.
		if (checksumCounter == 0) //Upper Nibble
			capturedMessage->checksum[0] = toupper(buffer);

		else if (checksumCounter == 1) //low nibble
			capturedMessage->checksum[1] = toupper(buffer);

		else if (checksumCounter > 1) {
			checksumCounter = 0;
			logChecksumFlag = 0;
			counter = 0;

			if (capturedMessage->checksum[0] == checksumHighNibble
					&& capturedMessage->checksum[1] == checksumLowNibble) { //checksum is correct
				capturedMessage->isValid = 1; //Message is valid
				return 1;
			} //Return Captured Message

			else { //checksum doesn't check out
				return 0;
			} //Start over. This packet is invalid.
		}

		checksumCounter++;
	} else if (buffer == '*' && counter != 0) { //Checksum '*' asterisks signals the next two bytes will contain the checksum. logChecksumFlag indicates this
		logChecksumFlag = 1;
		checksumLowNibble = lowerNibbleHex((int) checksum);
		checksumHighNibble = upperNibbleHex((int) checksum);
	} else if (counter > 0) { //pull data from packet
		if (buffer == ',') {
			currentVariable++; //go to next variable
			j = 0;
		} //Reset Internal incumentor

		else if (currentVariable == 0 && j < 5) //variable 0 is messageType. Only record first 5 characters
			capturedMessage->messageType[j++] = buffer;

		else if (currentVariable == 1 && j < 10)
			capturedMessage->CIGI_part[j++] = buffer;

		else if (currentVariable == 2 && j < 16)
			capturedMessage->deviceName[j++] = buffer;

		else if (currentVariable == 3 && j < valueNameLength)
			capturedMessage->valueName[0][j++] = buffer;

		else if (currentVariable == 4 && j < valueLength)
			capturedMessage->valueData[0][j++] = buffer;

		else if (currentVariable == 5 && j < valueNameLength)
			capturedMessage->valueName[1][j++] = buffer;

		else if (currentVariable == 6 && j < valueLength)
			capturedMessage->valueData[1][j++] = buffer;

		else if (currentVariable == 7 && j < valueNameLength)
			capturedMessage->valueName[2][j++] = buffer;

		else if (currentVariable == 8 && j < valueLength)
			capturedMessage->valueData[2][j++] = buffer;

		else if (currentVariable == 9 && j < valueNameLength)
			capturedMessage->valueName[3][j++] = buffer;

		else if (currentVariable == 10 && j < valueLength)
			capturedMessage->valueData[3][j++] = buffer;

		else if (currentVariable == 11 && j < valueNameLength)
			capturedMessage->valueName[4][j++] = buffer;

		else if (currentVariable == 12 && j < valueLength)
			capturedMessage->valueData[4][j++] = buffer;

		else if (currentVariable == 13 && j < valueNameLength)
			capturedMessage->valueName[5][j++] = buffer;

		else if (currentVariable == 14 && j < valueLength)
			capturedMessage->valueData[5][j++] = buffer;

		else if (currentVariable == 15 && j < valueNameLength)
			capturedMessage->valueName[6][j++] = buffer;

		else if (currentVariable == 16 && j < valueLength)
			capturedMessage->valueData[6][j++] = buffer;

		else if (currentVariable == 17 && j < valueNameLength)
			capturedMessage->valueName[7][j++] = buffer;

		else if (currentVariable == 18 && j < valueLength)
			capturedMessage->valueData[7][j++] = buffer;

		checksum ^= buffer; //Update checksum
		counter++; // Increment counter
	}

	return 0; //Message is not yet valid
}

int checksum(char *s) {
	int c = 0;

	while (*s) {
		c ^= *s++;
	}
	return c;
}

int calculateChecksum(char* incomingString) {

	int i = 0;
	int checksum = 0;

	for (i = 0; incomingString[i] != 0; i++) {
		checksum ^= incomingString[i];
	}

	return checksum;

}

char upperNibbleHex(int byteValue) {
	unsigned char value = byteValue;
	value = value & 0xF0;
	value = value >> 4;
	return returnNibbleHex((int) value);
}

char lowerNibbleHex(int byteValue) {
	return returnNibbleHex(byteValue);
}

char returnNibbleHex(int byteValue) {
	unsigned char value = byteValue;
	value = value & 0x0F;

	char hexChar = '!';
	//Serial.print(value);
	switch (value) {
	case 0:
		hexChar = '0';
		break;
	case 1:
		hexChar = '1';
		break;
	case 2:
		hexChar = '2';
		break;
	case 3:
		hexChar = '3';
		break;
	case 4:
		hexChar = '4';
		break;
	case 5:
		hexChar = '5';
		break;
	case 6:
		hexChar = '6';
		break;
	case 7:
		hexChar = '7';
		break;
	case 8:
		hexChar = '8';
		break;
	case 9:
		hexChar = '9';
		break;
	case 10:
		hexChar = 'A';
		break;
	case 11:
		hexChar = 'B';
		break;
	case 12:
		hexChar = 'C';
		break;
	case 13:
		hexChar = 'D';
		break;
	case 14:
		hexChar = 'E';
		break;
	case 15:
		hexChar = 'F';
		break;
	}
	//Serial.println(hexChar);
	return hexChar;
}

