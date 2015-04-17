#ifndef CIGI_SERIAL_PACKETS_H
#define CIGI_SERIAL_PACKETS_H

#define valueLength 9
#define numberOfValues 9  // was 9 in standards control program
#define valueNameLength 12

typedef struct _CILOG {
	char messageType[6]; //up to 5 characters long, additional for null character
	char CIGI_part[10];
	char deviceName[19]; //up to 18 characters long, additional for null character
	char valueName[numberOfValues][valueNameLength]; //8 names, each up to 12 characters long, additional for null character
	char valueData[numberOfValues][valueLength]; //8 values, up to 8 characters in length, additional for null character
	char checksum[3]; //two character checksum, represented in hex. Third character to be used as null character
	int isValid;
} CILOG;

int calculateChecksum(char*);
int checksum(char*);
char lowerNibbleHex(int);
char upperNibbleHex(int);
char returnNibbleHex(int byteValue);
int processIncomingPacket(char, CILOG*);

void debugPrintCIGIPacket(CILOG *, int, int, int) ;

#endif // CIGI_SERIAL_PACKETS_H
