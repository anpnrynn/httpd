#include <stdio.h>
#include "base64.h"


void splitToBase64 ( unsigned int val, char *c1, char *c2,  char *c3, char *c4 ) {
    *c1 = ( val ) & 0x0000003F;
    *c2 = ( val >> 6 ) & 0x0000003F;
    *c3 = ( val >> 12 ) & 0x0000003F;
    *c4 = ( val >> 18  ) & 0x0000003F;
}

char getBase64ConvChar ( char c ) {
    if ( c < 26 )
    { return 'a' + c; }
    else if ( c < 52 )
    { return 'A' + c - 26; }
    else if ( c < 62 )
    { return '0' + c - 52; }
    else {
        if ( c == 62 )
        { return ','; }
        else
        { return '.'; }
    }
}

char getBinaryConvChar ( char c ) {
    if ( c >= 'a' && c <= 'z' )
    { return c - 'a'; }
    else if ( c >= 'A' && c <= 'Z' )
    { return c - 'A' + 26; }
    else if ( c >= '0' && c <= '9' )
    { return c - '0' + 52; }
    else if ( c == ',' )
    { return ( char ) 62; }
    else
    { return ( char ) 63; }
}

void convertToBase64 ( unsigned char *src, int len, char *dest ) {
    unsigned int *temp = NULL;
    unsigned int value;
    int i, j;
    int size = ( int ) ( ( len * 8 ) / 6 );

    if ( ( len * 8 ) % 6 > 0 )
    { size++; }

    for ( i = 0, j = 0; i < len; i += 3, j += 4 ) {
        temp  =  ( unsigned int * ) &src[i];
        value =  ( *temp ) & 0x00FFFFFF;
        splitToBase64 ( value, &dest[j], &dest[j + 1], &dest[j + 2], &dest[j + 3] );
        dest[j] = getBase64ConvChar ( dest[j] );
        dest[j + 1] = getBase64ConvChar ( dest[j + 1] );
        dest[j + 2] = getBase64ConvChar ( dest[j + 2] );
        dest[j + 3] = getBase64ConvChar ( dest[j + 3] );
    }

    dest[size] = 0;
}

void convertToBinary ( char *src, int len, unsigned char *dest ) {
    unsigned int *temp;
    int i, j;
    unsigned char c1, c2, c3, c4;
    unsigned int value;

    for ( i = 0, j = 0; i < len; i += 4, j += 3 ) {
        value = 0;
        temp  = ( unsigned int * ) &dest[j];

        c1 = getBinaryConvChar ( src[i + 3] );
        value = c1 << 18;
        c2 = getBinaryConvChar ( src[i + 2] );
        value |= ( 0x3F & c2 ) << 12;
        c3 = getBinaryConvChar ( src[i + 1] );
        value |= ( 0x3F & c3 ) << 6;
        c4 = getBinaryConvChar ( src[i] );
        value |= ( 0x3F & c4 );
        *temp  = value & 0x00FFFFFF;
    }
}

