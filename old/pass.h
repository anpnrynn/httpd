#ifndef ___PASSWD__H___
#define ___PASSWD__H___


typedef enum {
	ENC_KEY_DBKEY = 0,
	ENC_KEY_FILESTORE,
	ENC_KEY_DATABASE,
	MAX_ENC_KEYS,
} ENC_KEY;

#ifndef COMPILER_C_LINKAGE
int  		  acquireEncryptionKeys  (short *firstTime);
struct Sfea*  getEncryptionStructure (ENC_KEY);
void          releaseEncryptionKeys  ();
int           readKey( void *udata, int argc, char **argv, char **colName);

#else
extern "C" int  		  acquireEncryptionKeys  (short *firstTime);
extern "C" struct Sfea*  getEncryptionStructure (ENC_KEY);
extern "C" void          releaseEncryptionKeys  ();
extern "C" int           readKey( void *udata, int argc, char **argv, char **colName);

#ifdef OS_WIN
void srandom ( unsigned int seedvalue );
unsigned int random ();
#endif

#endif

#endif
