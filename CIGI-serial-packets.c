#include <ctype.h>
#include <stdio.h>

#include <CIGI-serial-packets.h>

extern int DEBUG;

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

char upperNibbleHex(int byteValue) {
	unsigned char value = byteValue;
	value = value & 0xF0;
	value = value >> 4;
	return returnNibbleHex((int) value);
}

char lowerNibbleHex(int byteValue) {
	return returnNibbleHex(byteValue);
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
	static int currentVariable = 0;

	if (buffer == '$' || counter == 0) { // '$' Indicates Start of packet.
		if (buffer == '$') {
			counter = 1;
			logChecksumFlag = 0;
			checksumCounter = 0;
			currentVariable = 0;
			checksum = 0;

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
		if (checksumCounter == 0) { //Upper Nibble
			capturedMessage->checksum[0] = toupper(buffer);
			//printf("first received checksum digit=%i\n", capturedMessage->checksum[0]);
		}

		else if (checksumCounter == 1) { //low nibble
			capturedMessage->checksum[1] = toupper(buffer);
			//printf("second received checksum digit=%i\n", capturedMessage->checksum[1]);
		}

		else if (checksumCounter > 1) {
			checksumCounter = 0;
			logChecksumFlag = 0;
			counter = 0;

			if (capturedMessage->checksum[0] == checksumHighNibble
					&& capturedMessage->checksum[1] == checksumLowNibble) { //checksum is correct
				capturedMessage->isValid = 1; //Message is valid
				if (DEBUG) {
					printf("checksum IS VALID!");
				}
				return 1;
			} //Return Captured Message

			else { //checksum doesn't check out
				if (DEBUG) {
					printf("checksum IS NOT VALID!");
				}
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

		else if (currentVariable == 19 && j < valueNameLength)
			capturedMessage->valueName[8][j++] = buffer;

		else if (currentVariable == 20 && j < valueLength)
			capturedMessage->valueData[8][j++] = buffer;

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

void debugPrintCIGIPacket(CILOG *p, int number_of_values, int name_length, int value_length) {

	/*
	print out the contents of one of these data structures:

			typedef struct _CILOG {
				char messageType[6]; //up to 5 characters long, additional for null character
				char CIGI_part[10];
				char deviceName[19]; //up to 18 characters long, additional for null character
				char valueName[numberOfValues][valueNameLength]; //8 names, each up to 12 characters long, additional for null character
				char valueData[numberOfValues][valueLength]; //8 values, up to 8 characters in length, additional for null character
				char checksum[3]; //two character checksum, represented in hex. Third character to be used as null character
				int isValid;
			} CILOG;
	 */

	int i = 0;

	printf("\n===========================================\n");
	printf("Packet Contents:\n");
	printf("Message Type = "); printf(p->messageType); printf("\n");
	printf("CIGI Part Number = "); printf(p->CIGI_part); printf("\n");
	printf("device name = "); printf(p->deviceName); printf("\n");
	printf("%i Value and Data pairs to display:\n", number_of_values);
	for(i=0; i<number_of_values; i++) {
		printf("%3i)  ", (i+1)); printf(p->valueName[i]); printf(" = "); printf(p->valueData[i]); printf("\n");
	}
	printf("checksum = "); printf(p->checksum); printf("\n");
	printf("===========================================\n");

	return;
}
