#ifndef ___BSAE64___H___
#define ___BSAE64___H___

void splitToBase64( unsigned int val, char *c1, char *c2,  char *c3, char *c4 );
char getBase64ConvChar ( char c );
char getBinaryConvChar ( char c );
void convertToBase64(unsigned char *src, int len, char *dest);
void convertToBinary(char *src, int len, unsigned char *dest);
	
#endif
