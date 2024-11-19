#include <unistd.h>
#include "chunkedencoding.h"

int main(){

	ChunkedEncoding *ce = new ChunkedEncoding;
	//char *data = "hello this is a";// new approach to solving issues in the chunked trnasfer encoding \n";

	char data[4096];
	ce->setData( data, 4096, false );
	printf("size= %d , %x\n", strlen(data), strlen(data) );
	printf(" %x, %x %x %x %x %x \n", ce->data[0], ce->data[1], ce->data[2], ce->data[3], ce->data[4], ce->data[5] ); 
	printf(" %d, %d %d %d %d %d \n", ce->data[0], ce->data[1], ce->data[2], ce->data[3], ce->data[4], ce->data[5] ); 
	printf(" %c, %c %c %c %c %c \n", ce->data[0], ce->data[1], ce->data[2], ce->data[3], ce->data[4], ce->data[5] ); 
}
