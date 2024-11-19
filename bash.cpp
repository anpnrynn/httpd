//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#include <httpconn.h>
#include <plugin.h>
#include <httphandlers.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <chunkedencoding.h>

/*
struct PluginHandler{
    unsigned char type;
    unsigned char authLvlReq;
    unsigned int  version;
    char sname[40];
    char aname[40];
	PRLib *lib;
    void (*init)();
    void (*processReq)(Connection *);
    void (*exit)();
};
*/

//extern MapToBash mapToBash;

int bash_init();
int bash_exit();
int bash_processReq ( Connection *conn );

struct PluginHandler pInfo = { 0x00, 0x01, 1, "bash.xyz", "", NULL, bash_init, bash_processReq, bash_exit };

int bash_init() {
    HttpHandler *hHdlr = HttpHandler::createInstance();
	 fprintf(stderr, "plugin initing bash \n");
    if ( hHdlr )
    { hHdlr->addHandler ( pInfo.sname, ( void * ) &pInfo ); 
	 fprintf(stderr, "plugin inited bash\n");
	}
    return 0;
}

int bash_exit() {
	//mapToBash.clear();
    return 0;
}


int bash_processReq ( Connection *conn ) {
    struct sockaddr_un addr;
    ssize_t bytesread = 0;
    char buf[4096];

    if ( conn->req.cLen > conn->req.rLen )
    { conn->req.processHttpPostData ( conn ); }


	debuglog( "INFO: Bash ---------------> : %s , %d , %d , %d \n", conn->bash->socket.c_str(), conn->bash->pid, conn->bash->shellsockfd, conn->bash->input );
	Vector *param = new Vector();
	debuglog( "INFO: Bash ---------------> : %s , %d , %d , %d Vector conv done \n", conn->bash->socket.c_str(), conn->bash->pid, conn->bash->shellsockfd, conn->bash->input );
	conn->req.convertPostDataToVector ( param, NULL );
	if( conn->bash->pid == -1 ){
			fprintf( stderr, "INFO: Aborting invalid PID  %d \n",conn->bash->pid );
			conn->resp.setContentLen ( 0 );
    		sendConnRespHdr ( conn, HTTP_RESP_OK );
			return 0;
	}
	//conn->req.removePostTempFile();

	debuglog("INFO: Executing bash control commands \n");
	//bash?
	//command=START
	if( param->size() > 0 && conn && conn->bash ){
		if( (*param)[0] == "START" ){
			fprintf( stderr, "INFO: Executing START bash control commands \n");
			if( conn->bash->shellsockfd < 0 ){
			 	conn->bash->shellsockfd = socket(AF_UNIX, SOCK_STREAM, 0); 
    			if ( conn->bash->shellsockfd < 0 ){
    				debuglog("ERRR: Plugin socket fd = %d\n", conn->bash->shellsockfd);
					//sndhdr
        			return 1;
    			}
				char USOCKET[256];
				snprintf(USOCKET, 255, "/tmp/unix-socket-%d.sock", conn->bash->pid );
				fprintf(stderr, "INFO: Connecting to unix socket %s \n", USOCKET );
				fprintf(stderr, "INFO: Connecting to unix socket with %d \n", conn->bash->shellsockfd );
    			memset(&addr, 0, sizeof(struct sockaddr_un));
    			snprintf( addr.sun_path, sizeof(addr.sun_path) - 1, USOCKET, conn->bash->pid );
    			addr.sun_family = AF_UNIX;
				while (connect(conn->bash->shellsockfd, (struct sockaddr *) &addr,
                	sizeof(struct sockaddr_un)) == -1) {
        			usleep( 10000 );
    			}
				//#int flags = fcntl(, F_GETFL, 0); 
				//fcntl(fd, F_SETFL, flags | O_NONBLOCK);
				conn->resp.setContentLen ( 0 );
			}

			conn->resp.setContentType ( "text/plain" );
    		sendConnRespHdr ( conn, HTTP_RESP_OK );
		} 
		else if ( (*param)[0] == "STOP"){
			fprintf( stderr, "INFO: Executing STOP bash control commands \n");
			conn->resp.setContentLen ( 0 );
			kill( conn->bash->pid, SIGTERM );
			close(conn->bash->shellsockfd);
			conn->bash->socket="";
			conn->bash->pid = -1;
			conn->bash->input = 0;
			conn->bash->shellsockfd = -1;

			conn->resp.setContentType ( "text/plain" );
    		sendConnRespHdr ( conn, HTTP_RESP_OK );
		}
		else if ( (*param)[0] == "ABORT"){
			fprintf( stderr, "INFO: Executing ABORT bash control commands \n");
			conn->resp.setContentLen ( 0 );
			kill( conn->bash->pid, SIGKILL );
			close(conn->bash->shellsockfd);
			conn->bash->socket="";
			conn->bash->pid = -1;
			conn->bash->input = 0;
			conn->bash->shellsockfd = -1;

			conn->resp.setContentType ( "text/plain" );
    		sendConnRespHdr ( conn, HTTP_RESP_OK );
		}
		else if ( (*param)[0] == "XCMD"){
			fprintf( stderr, "INFO: Executing XCMD bash control commands \n");
			conn->resp.setContentLen ( 0 );
			char cmd[256] = {0};
			snprintf( cmd, 255, "p=`pgrep -P %d` && kill -9 $p;", conn->bash->pid );
			system ( cmd );
			conn->resp.setContentType ( "text/plain" );
    		sendConnRespHdr ( conn, HTTP_RESP_OK );
		}else if ( (*param)[0] == "INT"){
			fprintf( stderr, "INFO: Executing INTERRUPT bash control commands \n");
			conn->resp.setContentLen ( 0 );
			kill( conn->bash->pid, SIGINT );
			conn->resp.setContentType ( "text/plain" );
    		sendConnRespHdr ( conn, HTTP_RESP_OK );
		}

		else if ( (*param)[0] == "EXECUTE"){
			//shellcommand=cmds
			fprintf( stderr, "INFO: Executing bash commands  %s \n", ((*param)[1]).c_str());
			char file[256];
			snprintf( file, 255, "/tmp/input-%d.%d", conn->bash->pid, conn->bash->input );
			conn->bash->input++;
			FILE *f = fopen(file, "w");
			if( f){
				debuglog( "INFO:  writing to file : %s\n",file );
				int n = 0;
				string script = "#!/bin/bash\n";
				script += (*param)[1] + "\n";
				n =	fputs( script.c_str(), f );
				fflush(f);
				fclose(f);
			}			

			char execfile[256];
			snprintf( execfile, 255, "%s-execute", file );
			FILE *f2 = fopen( execfile, "w");
			fprintf(f2, "EXECUTE\n");
			fclose(f2);
			debuglog( "INFO:  created file : %s\n",execfile );

    		fprintf(stderr, "INFO: Plugin socket fd = %d\n", conn->bash->shellsockfd);

			conn->resp.setContentType ( "text/plain" );
			conn->resp.setContentLen ( -1 );
    		sendConnRespHdr ( conn, HTTP_RESP_OK );

			ChunkedEncoding ce; // = new ChunkedEncoding();
    		if ( conn->bash->shellsockfd >= 0 ){
					struct stat s;
				char sfile[256] = {0};
				char ofile[256] = {0};
				snprintf( sfile, 255, "/tmp/output-%d.%d" , conn->bash->pid, conn->bash->input-1 );
				snprintf( ofile, 255, "/tmp/output-%d-exe.%d" , conn->bash->pid, conn->bash->input-1 );
				debuglog("INFO: Stating %s , %d \n", sfile, conn->bash->shellsockfd );
				debuglog("INFO: Output %s \n", ofile);
				do{
					usleep(1000);
    			}while ( -1 == stat( sfile, &s ) ) ;

				debuglog("INFO: out of stat loop \n");
				//int fd = open( ofile , O_NONBLOCK);
				int fd = open (ofile, O_RDONLY|O_NONBLOCK);
			    if (fd == -1) {
            		perror ("open");
					debuglog("INFO: Opening output file error : %s \n", ofile);
    			}
				debuglog("INFO: CE object created: %x \n", ce);
				do{
					bytesread = read(fd, buf, 4096);
					debuglog("INFO: Bytes read : %d \n",bytesread);
					if( bytesread > 0 ){
        				buf[bytesread] = 0;
						ce.reset();
						ce.setData( buf, bytesread, false );
							debuglog("INFO: sending connection data \n");
						sendConnectionData( conn, ce.data, ce.size );
							debuglog("INFO: sending connection data - end  \n");
						//appendrespstring( con, buf );
						fprintf(stderr, "%s", buf);
						buf[0] = 0;
					} else {
						if( bytesread == 0 ) {
							usleep(1000);
							break;
						} else {
							debuglog("INFO: fd erro, %d \n", errno);
							usleep(1000);
						}
					} 
    			}while ( bytesread > 0 || ( bytesread == -1 && errno == EAGAIN ) ) ;

				debuglog("INFO: out of read loop \n");
				ce.reset();
				ce.setData( buf, 0, true );
				sendConnectionData( conn, ce.data, ce.size );
				debuglog("INFO: done \n");
				close(fd);
				unlink ( sfile);
				unlink ( ofile);
				unlink ( execfile );
			}
		}
	}	else {
		fprintf( stderr, "ERRR: param size is 0\n");
	}

    return 0;
}

