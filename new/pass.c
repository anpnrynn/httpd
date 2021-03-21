#include <stdio.h>
#include <prio.h>
#include <prmem.h>
#include <pass.h>
#include <base64.h>
#include <string.h>
#include <time.h>
#include <sfeac.h>
#include <math.h>


#ifdef   IS_HTTPD
#include <sqlite3.h>
#include <defines.h>
#endif


char *pass1="This is a stupid password, setup to confuse the people trying to hack the db, ha ha ha... if you are reading this then your are not supposed to be doing that. That's precisely the reason why we used this approach to hide the real password. Anyway best of luck in breaking it. Its not impossible but it is a little difficult.";
char *pass2="What's this ??? couldn't believe that there is more to it aht 324 characters. Well, the reason for this is that i am trying to do a confusion diffusion approach to blinding any hacking procedure. Lots of people seem to be quite skeptical about its principles but who give a rats ass ... huh ???";
char *pass3="When is this going to end ? My likely suggestion for you is that, you will not find it here... have you played 'hunt the thimble ?'... well, either ways ... its really really cold ... :D !!!! LOL";
char *pass4="Mary had a little lamb, little lamb, little lamb... Mary had a little lamb till she celebrated bbq day !!!!!!!!!!!!!!!!!!!!!!!, its 9:59pm on the 26th of sept 2007, I still have loads and loads of work to do ... my god ... the adreli <--- you get it right ? i don't quite remember the spelling for it, T20 was all fixed so that, it gains wider acceptance..........,";
char *pass5="PBR? WHAT'S THE DEAL WITH THAT ? HOW DO THEY CALL IT EVEN A SPORT, I GUESS SOME PEOPLE JUST LIKE GETTING MAULED BY A BIG BAD ASS BULL... MAYBE WE SHOULD CONVERT THE BULLCART RACING (THAT HAPPENS IN inDIA) IN TO A MAINSTREAM SPORT TOO :p ....189234817304718378172937491723417203947109387409182740917349871093847130987410938740913874198740940698109327412746237428581623758729851836519";
char *pass6="IHAVE NO idEA WHY AXN SHOWS WORLD'S GREATEST POLICE VIDEOS............. IN MY OPINIONS SUCH THINGS SHOULD NOT BE SHOWN ON TV, CAUSEIT WOULD SEEM LIKE A CHALLENGE TO MORE AND MORE PEOPLE...TO TRY AND BREAK THE LAW AND TRY TO FLEE !!! WHAT THEY SHOULD INSTEAD SHOW IS WORLD'S FUNNIEST LAWSUITS !!! ... NOW THAT WILL BE KICK ASS FUN ...DON'T YOU THINK sO ????~!@#$%^&*()__+<>?{}|[]";
char *pass7="!@#$%^&*()_~!@#$%^&*()~!@#$%^&*()_+~!@#$%^&*()_+{}|{}|{}|{}{}{}|{}|<>?<>?,.,./<><>?,./,.<./<>/<./,./l;l;l;'l;'l;'l;'!23$#456756789%^&*()5678($%^&*90#$%^&*90^&8905678903456789)34523!2312312312#$!@#!@#!@#!@#$456$%64%45^$%6$%^&&^&*&*9()(09)00{}|";
char *pass8="daris hilton is the only person in the world to perform a reverso ... from a rich ass corporate babe to a _________ ";

struct Sfea  *encVector[MAX_ENC_KEYS];

int          readKey( void *udata, int argc, char **argv, char **colName)
{
	char *data= (char *) udata;
	if( data )
	{
		if ( argc >= 0 )
		{
			strcpy( data, argv[0] );
		}
	}
	return 0;
}

int           acquireEncryptionKeys  (short *firstTime)
{
#if 0
	unsigned char  pwd1[64];
	unsigned char  pwd2[64];
	unsigned char  pwd3[128];
	char  pwdBase64[128];
	char  tempBuf  [256];
	char *error=0;
	unsigned char *pass=NULL;
	int i, rc; 
	int len=0;
	struct sqlite3 *db;
	time_t sec;
	unsigned char temp;

	memset (pwd1, 0   , 64 );
	memset (pwd2, 0xe1, 64 );
	memset (pwd3, 0x1e, 128 );
	memset (pwdBase64, 0   , 128 );

	len=strlen(pass1);
	pass=(unsigned char *)pass1;
	for( i=0; i<len; i++ )
		pwd1[i%64] = pwd1[i%64] ^ (unsigned char )pass[i];
	len=strlen(pass3);
	pass=(unsigned char *)pass3;
	for( i=0; i<len; i++ )
		pwd1[i%64] = pwd1[i%64] ^ (unsigned char )pass[i];
	len=strlen(pass5);
	pass=(unsigned char *)pass5;
	for( i=0; i<len; i++ )
		pwd1[i%64] = pwd1[i%64] ^ (unsigned char )pass[i];
	len=strlen(pass7);
	pass=(unsigned char *)pass7;
	for( i=0; i<len; i++ )
		pwd1[i%64] = pwd1[i%64] ^ (unsigned char )pass[i];

	len=strlen(pass2);
	pass=(unsigned char *)pass2;
	for( i=0; i<len; i++ )
		pwd2[i%64] = pwd2[i%64] ^ (unsigned char )pass[i];
	len=strlen(pass4);
	pass=(unsigned char *)pass4;
	for( i=0; i<len; i++ )
		pwd2[i%64] = pwd2[i%64] ^ (unsigned char )pass[i];
	len=strlen(pass6);
	pass=(unsigned char *)pass6;
	for( i=0; i<len; i++ )
		pwd2[i%64] = pwd2[i%64] ^ (unsigned char )pass[i];
	len=strlen(pass8);
	pass=(unsigned char *)pass8;
	for( i=0; i<len; i++ )
		pwd2[i%64] = pwd2[i%64] ^ (unsigned char )pass[i];

	encVector[ENC_KEY_DBKEY] = (struct Sfea *) PR_Malloc(sizeof(struct Sfea));
	//SfeaInit( encVector[ENC_KEY_DBKEY], pwd1, 64 );
	//SfeaEncrypt( encVector[ENC_KEY_DBKEY], pwd2, 64, 0 ); 

	encVector[ENC_KEY_FILESTORE] = (struct Sfea *) PR_Malloc(sizeof(struct Sfea));
	//SfeaInit( encVector[ENC_KEY_FILESTORE], pwd2, 64 );
	//SfeaEncrypt( encVector[ENC_KEY_FILESTORE], pwd1, 64, 0 );	

	memset ( encVector[ENC_KEY_DBKEY], 0, sizeof( struct Sfea ) );
	//SfeaInit( encVector[ENC_KEY_DBKEY], pwd1, 64 );

#ifdef IS_HTTPD
	PRFileDesc *f = PR_Open(KEYFILE, PR_RDONLY, 0);
	if( f )
	{
		if(firstTime) 
			*firstTime = 0;
		rc = sqlite3_open(KEYFILE, &db, encVector[ENC_KEY_DBKEY]);
		if( rc )
		{
			fprintf(stderr,"ERRR: Unable to open db, Corrupted ? \n");
       		sqlite3_close(db);
			db = NULL;
			return 1;
		}
		else
		{
			fprintf(stderr,"KEYGEN: File Opened \n");
		}
		
		rc = sqlite3_exec(db, "select * from KEYS;", readKey , pwdBase64, &error);
		if( rc != SQLITE_OK )
		{	
			if( error )
			{
				sqlite3_free(error);
				fprintf(stderr,"ERRR: KEYGEN: Unable to read key file  \n");
			}
			sqlite3_close(db);
			return 1;
		}
		if( pwdBase64[0] == 0 )
		{
			fprintf(stderr,"ERRR: KEYGEN: Data read unsuccessful (If your running it the first time delete '%s' File and restart) \n",KEYFILE);
			sqlite3_close(db);
			return 1;  
		}
		//printf("Base64 Password(read):'%s'\n",pwdBase64);
		convertToBinary(pwdBase64, strlen(pwdBase64), pwd3);

		//SfeaDecrypt( encVector[ENC_KEY_FILESTORE], pwd3, 64, 0 );	
		//SfeaDecrypt( encVector[ENC_KEY_DBKEY], pwd3, 64, 0 );	
		encVector[ENC_KEY_DATABASE] = (struct Sfea *) PR_Malloc(sizeof(struct Sfea));
		//SfeaInit( encVector[ENC_KEY_DATABASE], pwd3, 64 );
	}
	else
	{
		if(firstTime) 
			*firstTime = 1;
		sec = time(NULL);
		srandom ( sec );
		temp  = (unsigned char) ( time( NULL ) ^ random() );
		pwd3[i] = pwd1[i] ^ pwd2[64-i] ^ temp;
		for( i=1; i<64; i++ )
		{
			temp  = (unsigned char) ( time( NULL ) ^ random() );
			pwd3[i] = pwd1[i] ^ pwd2[64-i] ^ temp ^ (~pwd3[i-1]);
		}
		//SfeaEncrypt( encVector[ENC_KEY_FILESTORE], pwd3, 64, 0 );	
		convertToBase64 ( pwd3, 64, pwdBase64 );
		//printf("Base64 Password:'%s'\n",pwdBase64);
		sprintf(tempBuf, "insert into KEYS values ( '%s' );",pwdBase64 );

		//SfeaDecrypt( encVector[ENC_KEY_FILESTORE], pwd3, 64, 0 );	
		//SfeaDecrypt( encVector[ENC_KEY_DBKEY], pwd3, 64, 0 );	
		encVector[ENC_KEY_DATABASE] = (struct Sfea *) PR_Malloc(sizeof(struct Sfea));
		//SfeaInit( encVector[ENC_KEY_DATABASE], pwd3, 64 );

		rc = sqlite3_open(KEYFILE, &db, encVector[ENC_KEY_DBKEY]);
		if( rc )
		{
			fprintf(stderr,"ERRR: Unable to open db, Corrupted ? \n");
       		sqlite3_close(db);
			db = NULL;
			return 1;
		}
		else
		{
			fprintf(stderr,"KEYGEN: File Opened For the First Time\n");
		}
		rc = sqlite3_exec(db, "create table KEYS ( key varchar(128) );", NULL , NULL, &error);
		if( rc != SQLITE_OK )
		{	
			if( error )
			{
				sqlite3_free(error);
				fprintf(stderr,"ERRR: KEYGEN: Unable to create data storage area  \n");
			}
			sqlite3_close(db);
			return 1;
		}
		rc = sqlite3_exec(db, tempBuf, NULL , NULL, &error);
		if( rc != SQLITE_OK )
		{	
			if( error )
			{
				sqlite3_free(error);
				fprintf(stderr,"ERRR: KEYGEN: Unable to create data storage area  \n");
			}
			sqlite3_close(db);
			return 1;
		}
	}
	sqlite3_close(db);
#endif
	memset (pwd1, 0   , 64 );
	memset (pwd2, 0, 64 );
	memset (pwd3, 0, 128 );
	memset (pwdBase64, 0   , 128 );
	return 0;
#endif
}



struct Sfea*  getEncryptionStructure (ENC_KEY i)
{
	return encVector[i];
}

void          releaseEncryptionKeys  ()
{
	int i;
	for( i=0; i< MAX_ENC_KEYS; i++)
	{
		if( encVector[i] )
		{
			PR_Free( encVector[i] );	
			encVector[i] = 0;
		}
	}
}

#ifdef OS_WIN

unsigned int randomSeed=0;
unsigned int c;
unsigned int a;
unsigned int m;

void srandom ( unsigned int seedvalue )
{
     randomSeed = seedvalue;
     m = ((unsigned int)pow ( 2, 31 )) - 1;
     c += seedvalue < m ? ( ((unsigned int )pow(seedvalue,m/seedvalue))%m): (((unsigned int )pow(seedvalue,seedvalue/m))%m);
     a += c < m ? (c*m+randomSeed)%m: (a*m*c+randomSeed)%m;
     unsigned int temp;
     if ( c > m )
     {
           temp = c;
           c = m;
           m= temp;
     }
     if ( a > m )
     {
          temp = a;
           a = m;
           m= temp;
     }    
}

unsigned int random ()
{
         randomSeed = (a*randomSeed + c)%m;
         return m;
}
#endif

