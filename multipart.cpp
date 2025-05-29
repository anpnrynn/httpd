#include <multipart.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <log.h>
#include <unistd.h>

MultipartReader::MultipartReader( FILE *fd, char *bound ){
	postFile = fd;
	eof = 0;
	//if( postFile ) {
		fseek( postFile, 0L, SEEK_SET );
		vm = new VectorM();
		boundary = bound;
	//}
}

MultipartReader::~MultipartReader( ){
	fclose(postFile);
	VectorM::iterator i = vm->begin();
	while( i != vm->end() ){
		delete *i;
		i++;
	}
	delete vm;
}

int MultipartReader::readline( char *line ){
	int i = 0;
	while( 1 ){
		//if( feof( postFile ) ){
		//	break;
		//}
		//int  d = fgetc ( postFile );
		unsigned char c = 0;
		int d = fread(&c, 1, 1, postFile );
		if( d ==  0 || d < 0 ){
			if( d == 0) {
				eof = 1;
				break;
			} else if ( errno == EAGAIN ){
				continue;
			} else {
				break;
			}
		}
		//char c = ( char ) d;
		if( c == '\r' ){
			line[i++] = c;
		} else if ( c == '\n' ){
			if( line[i-1] == '\r' )
			{	line[i++] = c;
				line[i++] = 0;
				break;
			} else {
				line[i++] = c;
			}
		} else {
			line[i++] = c;
		}
	}
	return i;
}

int MultipartReader::quickread( unsigned char *data, int &offset, int size){
	int i = 0;
	char bound[256];
	bound[0] ='-';
	data[offset] = '-';
	for ( i=1; i<size; i++ ){
		//if( feof( postFile ) ){
		//	break;
		//}
		//int  d = fgetc ( postFile );
		unsigned char c = 0;
		int d = fread (&c , 1, 1 , postFile );
		if( d ==  0 || d < 0 ){
			if( d == 0) {
				eof = 1;
				break;
			} else if ( errno == EAGAIN ){
				continue;
			} else {
				break;
			}
		}
		bound[i] = (unsigned char ) c;
		data[offset+i]  = bound[i];
	}
	bound[i] = 0;
	offset += i;
	//fdebuglog( stderr, "%s is a boundary ? %s  \n", bound, boundary );
	return checkBoundary( bound, boundary );

}


int MultipartReader::readTillBoundaryEnd(unsigned char *data , int *datasize ){
	char bound[256];
	int m = strlen( boundary );
	int i = 0;
	memset( bound, 0, 256 );
	unsigned char c = 0;;
	unsigned char prevc = 0;
	while( 1 ){
		//if( feof( postFile ) ){
		//	break;
		//}
		//int  d = fgetc ( postFile );
		prevc = c;
		int d = fread( &c, 1, 1, postFile);
		if( d ==  0 || d < 0){
			if( d == 0) {
				eof = 1;
				break;
			} else if ( errno == EAGAIN ){
				continue;
			} else {
				break;
			}
		}
		//c = (unsigned char ) d;
		if( c == '-' && ( prevc == '\n' || prevc == '\r' )){
			int rc = quickread( data, i, m );
			if( rc == 0 ){
				//fdebuglog(stderr, "Not a bounday \n");
			} else {
				int k = 0;
				while( k < m ){
					data[i-m+k] = 0;
					k++;
				}
				data[i] = 0;
				*datasize = i-m;
				//if( data[*datasize-1] == '\n' ){
				//	fdebuglog(stderr, "Data contains \\n \n");
				//}
				data[*datasize-1] = 0;
				data[*datasize-2] = 0;
				*datasize -=2;
				return 0;
			}
		} else {
			data[i++] = c;
		}
	}
	return 1;
}

int MultipartReader::processMultipartData(int postsize){
	char name[256];
	char filename[256];
	char mime[64];
	char boundaryChange[256];
	char *line = new char[65536];
	unsigned char *data = new unsigned char [postsize];
	int skipboundary = 1;
	do {
		name[0] = 0;
		filename[0] = 0;
		line[0] = 0;
		memset( name, 0, 256);
		memset( filename, 0, 256);
		memset( mime, 0, 64);
		memset(line, 0, 65536 );
		if( skipboundary ) {
			readline( line );
			if( strcmp( line , boundary ) != 0 ){
				strcpy( boundaryChange , line );
				int l = strlen(boundaryChange);
				if( boundaryChange[l-1] == '\n' ){
					boundaryChange[l-1] = 0;
					boundaryChange[l-2] = 0;
					boundary = boundaryChange;
				}
			}
			skipboundary = 0;
		}
		//debuglog("ERRR: Boundary read : %s\n", line);
		do {
			readline( line );
		} while( strcmp(line, "\r\n") == 0 );

		//if( feof(postFile) ){
		//	break;
		//}
		if( eof )
			break;
		//skipemptyline();

		readContentDisposition(line, name, filename );
		if( filename[0] == 0 || strcmp (filename , "" ) == 0 ){
			readline( line );
			//skipemptyline();
			readline( line );
			MultipartObject *mo = new MultipartObject();
			int x = strlen(line);
			if( line[x-1] == '\n' ){
				line[x-1] = 0;
				line[x-2] = 0;
			}
			mo->name = name;
			mo->value = line;
			debuglog("ERRR: Name=%s & value=%s \n", name, line );
			mo->length = 0;
			mo->filename = "";
			mo->mime = "";
			line = new char[65536];
			vm->push_back(mo);
			readline( line );
		} else {
			readline( line) ;
			readContentType( line, mime );
			int len = 0;
			readline (line);
			readTillBoundaryEnd( data, &len );
			readline(line);
			MultipartObject *mo = new MultipartObject();
			mo->name = name;
			mo->value = "";
			mo->filename = filename;
			mo->length = len;
			mo->data = data;
			mo->mime = mime;
			debuglog("ERRR: Name=%s, filename=%s, mime=%s, length=%d \n", name, filename, mime, len );
			data = new unsigned char [postsize];
			vm->push_back(mo);
		}

	//} while ( !feof(postFile) );
	} while ( !eof );
	return 0;

}


int MultipartReader::readContentType (char *line , char *mime){
	int n  = sscanf( line, "Content-Type: %s\r\n", mime );
	if( n == 1 )
		return 1;
	else
		return 0;
}

int MultipartReader::readContentDisposition(char *line , char * name, char *filename){
	const char *nameprefix = "Content-Disposition: form-data; name=\"";
	size_t n = strlen( nameprefix );
	if( n > strlen(line) ) {
		name[0] = 0;
		filename[0] = 0;
		return 0;
	}
	size_t i = n;
	int j = 0;
	while( line[i] != '"' ){
		name[j++] = line[i++];
	}
	name[j] = 0;

	const char *filenameprefix = "\"; filename=\"";
	i += strlen( filenameprefix );
	if( i > strlen(line) ){
		filename[0] = 0;
		return 1;
	}
	j=0;
	while( line[i] != '"' ){
		filename[j++] = line[i++];
	}
	filename[j] = 0;
	n = 2;

	if( n == 2 ){
		return 2;
	}

	return 0;
}

int MultipartReader::checkBoundary (char *line, char *boundary){
	//fdebuglog( stderr, "Line=%s    &    boundary=%s \n",line, boundary );
	if( strncmp( line, boundary, strlen(boundary) ) == 0 ){
		return 1;
	} else {
		//fdebuglog(stderr, "Boundary not valid \n");
		return 0;
	}
}

#ifdef TEST_MP


int main(int argc , char *argv[]){

	int fd = open(argv[1], O_LARGEFILE );

	MultipartReader *mpr = new MultipartReader ( fd, argv[2] );

	mpr->processMultipartData( 1048576 );

}


#endif
