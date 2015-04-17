/*
 * rpiGPIOserial.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <CIGI-serial-packets.h>
#include <rpi-mysql.h>
#include <rpi-uart.h>
#define version "0.1"

#define true 1
#define false 0

#define BUFFERSIZE 1024

extern char MODEMDEVICE[BUFFERSIZE];

// global debug flag
// can be set with -d cmd line parameter option
int DEBUG = 0; // set to 0 to disable debugging printf()s

int main(int argc, char ** argv) {

	CILOG packet_buffer;
	int uart0_filestream = -1;
	int exit_program = false;

	//const int BUFFERSIZE = 1024;

	char buffer[BUFFERSIZE];
	char rx_buffer[BUFFERSIZE];

	// sort through the options and parameters received from the command line
	int c = 0;
	opterr = 0;

	while ((c = getopt(argc, argv, "d")) != -1)
		switch (c) {
		case 'd':
			DEBUG = 1;
			break;
		case '?':
			if (isprint(optopt)) {
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			} else {
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			}
			fprintf(stderr, "available option(s):\n");
			fprintf(stderr, "-d to display debug info\n\n");
			return 1;
		default:
			DEBUG = 0;
		}

	if (DEBUG) {
		printf("debug flag = %d\n", DEBUG);
	}

	if (optind == argc) {
		fprintf(stderr,
				"missing UART port. example usage: rpiGPIOserial /dev/ttyAMA0\n");
		fprintf(stderr,
				"to see more info, run with -d option.\nexample usage: rpiGPIOserial -d /dev/ttyAMA0\n\n");
		return -1;
	}

	/*
	 //display the command line arguments as received
	 if(DEBUG) {
	 for (index = optind; index < argc; index++) {
	 printf("Non-option argument %i = %s\n", index, argv[index]);
	 }
	 optind = 0;
	 printf("length = %i\n", (int) strlen(argv[argc-1]));
	 }
	 */

	printf("CIGI serial packet translator, version ");
	printf(version);
	printf("\n\n");

	debugTestSQL();

	memset(MODEMDEVICE, '\0', sizeof(MODEMDEVICE));
	strcpy(MODEMDEVICE, argv[argc - 1]);

	if (DEBUG) {
		printf("port to open = %s\n", MODEMDEVICE);
	}

	// setup the serial port and get filedescriptor for accessing the UART
	if (rpi_uart_setup(&uart0_filestream) == -1) {
		//ERROR - CAN'T OPEN SERIAL PORT
		perror(
				"Error - Unable to open UART.  Ensure it is not in use by another application. ");
	}

	if (DEBUG) {
		// send the caps alphabet A thru Z
		char testbyte = 'A';

		int n = 0;
		int i = 0;

		for (i = 0; i < 26; i++) {
			n = rpi_uart_send_byte(&uart0_filestream, &testbyte);
			testbyte++;
			if (n < 0) {
				perror("Write failed - ");
				return n;
			}
		}

		rpi_uart_send_newline(&uart0_filestream);

		// example of a valid packet:
		char * test_packet =
				"$CILOG,C3766,CurrentRegulator,Anode,1.90mA,Cathode,1.11mA,Setpoint,7.79V,Locked?,No,Error#,0,v.,3.0,Pg,0.01000,Ig,0.00000,Dg,0.00000,*45";

		n = rpi_uart_send_string(&uart0_filestream, &test_packet[0]);
		if (n < 0) {
			perror("Write failed - ");
			return n;
		}

		rpi_uart_send_newline(&uart0_filestream);

	}

	if (uart0_filestream > 0) {
		printf("waiting to receive data via UART.\n");

		int buffer_position = 0;
		int rx_byte_count = 0;
		int i = 0;

		while (exit_program == false) {

			rx_byte_count = rpi_uart_receive_bytes(&uart0_filestream,
					&rx_byte_count, rx_buffer, BUFFERSIZE);

			if (rx_byte_count > 0) {

				if ((buffer_position + rx_byte_count) > BUFFERSIZE) {
					buffer_position = 0; // reset to start writing at the beginning of the buffer again
					printf("buffer position set to zero");
				}

				for (i = 0; i < rx_byte_count; i++) {
					buffer[buffer_position++] = rx_buffer[i];
				}

				for (i = 0; i < (buffer_position + rx_byte_count); i++) {
					if (DEBUG) {
						printf("buffer[%i] = %c\n", i, buffer[i]);
					}
					if (processIncomingPacket(buffer[i], &packet_buffer) == 1) {
						printf("\n\nincoming packet validated OK !\n\n");
						debugPrintCIGIPacket(&packet_buffer, numberOfValues, valueNameLength, valueLength);
						for (i = 0; i <= BUFFERSIZE; i++) {
							buffer[i] = 0;
						}
						buffer_position = 0;
						if(DEBUG) {
							exit_program = true;
						}
					}
				}
			}
		}

		if (DEBUG) {
			printf("finished.  The party's over!\n\n");
		}

		// Don't forget to clean up
		rpi_uart_close(&uart0_filestream);
	}
	return 0;
}


