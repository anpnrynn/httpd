#ifndef MULTIPART_H__
#define MULTIPART_H__

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <string.h>

using namespace std;


class MultipartObject {
	private:
	public:
		string name;
		string value;
		string filename;
		string mime;
		unsigned int length;
		unsigned char* data;

		~MultipartObject(){
			if( data )
				delete []data;
		}
};

typedef vector<MultipartObject*> VectorM;


class MultipartReader {
	private:
		FILE *postFile;
		//int postFile; //fd
		char *boundary;
		int eof;

	public:
		VectorM *vm;

		MultipartReader( FILE *fd, char *bound );
		~MultipartReader( );

		int readline( char *line );
		int quickread( unsigned char *data, int &offset, int size);
		int readTillBoundaryEnd(unsigned char *data , int *datasize );
		int processMultipartData(int postsize);
		int readContentType (char *line , char *mime);
		int readContentDisposition(char *line , char * name, char *filename);
		int checkBoundary (char *line, char *boundary);
};


#endif
