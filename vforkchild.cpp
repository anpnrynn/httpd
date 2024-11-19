#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include<sys/un.h>
#include<sys/wait.h>

#define USOCKET "/tmp/unix-socket-%d.sock"


int bashprocess () {
	struct sockaddr_un addr;
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0 );	

	if (sockfd == -1) {
    	perror("socket");
		exit(1);
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	snprintf( addr.sun_path, sizeof(addr.sun_path) - 1, USOCKET, (int)getpid() );
    addr.sun_family = AF_UNIX;

 	if (bind(sockfd, (struct sockaddr *) &addr, 
		sizeof(struct sockaddr_un)) == -1) {
		perror("bind");
		return 2;
	}
	printf(" server socket = %s \n", addr.sun_path );

	if (listen(sockfd, 10) == -1) {
    	perror("listen");
		return 3;
	}

	int bashcfd = 0;
	while(1){
    	printf("Waiting to accept a connection...\n");
    	bashcfd = accept(sockfd, NULL, NULL);
		if( bashcfd > 2 ){
    			printf("Accepted socket fd = %d\n", bashcfd);
				break;
		}
		usleep(1000);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	dup2(sockfd, STDIN_FILENO);
	dup2(sockfd, STDOUT_FILENO);
	dup2(sockfd, STDERR_FILENO);

	printf(" Starting bash \n");
	char *argv[32] = { "/bin/bash", "./contextscript.bash", 0} ;
	int rc = execvp( argv[0], argv);
	if( rc == -1 ){
		perror("execv");
		exit(3);
	}
	printf(" Stoping bash \n");
	return 0;
}

int main(){
	bashprocess();
}
