#ifndef MULTIPART_H__
#define MULTIPART_H__


#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <apptypes.h>
#include <mozhdr.h>

using namespace std;

class MultipartElement {
	private:
		bool   isFile;
		bool   isFormData;
		string name;
		string fileName;
		string value;	
		string contentType;

	public:

		MultipartElement ();
		MultipartElemtn  ( char *nameValue );
		~MultipartElement ();

		bool isElementFile();
		char * getName();
		void   setName(char *nameValue );
	
		char *getFileName ();
		char *getFileAbsPath ();
		void  setFileName ( char *fileNameValue);

		char *getValue ();
		void  setValue (char *elementValue );

		char *getContentType();
		void  setContentType();

		bool operator <( MultiPartElement & );
};


enum MpParseState{
	MP_START     = 0,
	MP_HEADER       ,
	MP_HEADEREND    ,
	MP_DATA         ,
	MP_EMPTYLINE    ,
	MP_END          ,
	MP_MAXSTATE
};


#define MAX_MPLINE_SIZE 256

class MultipartParser {

	private:

		char mpBoundary[128];
		unsigned char mpBuf [MAX_MPLINE_SIZE];
		MpParseState state;
		int  mpCount;
		int  mpLineCharCount;


	public:

		MultipartParser( PRFileDesc *mpFile, char *boundaryValue);
		~MultipartParser();

		
};


#endif
