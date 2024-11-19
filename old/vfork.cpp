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

struct msg {
	int id;
	int length;
	char name[32];
	char *data;
};


pid_t bpid = 0;

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

	printf(" client socket = %s \n", addr.sun_path );

	int count = 1000;
    while (connect(sockfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un)) == -1) {
		//perror("connect");
		usleep( 10000 );
    }

	printf(" Connected \n");

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	dup2(sockfd, STDIN_FILENO);
	dup2(sockfd, STDOUT_FILENO);
	dup2(sockfd, STDERR_FILENO);

	printf(" Starting bash \n");
	char *argv[32] = { "/bin/bash", "./input3.bash", 0} ;
	int rc = execvp( argv[0], argv);
	if( rc == -1 ){
		perror("execv");
		exit(3);
	}
	printf(" Stoping bash \n");
	return 0;
}

int serverprocess(){
	struct sockaddr_un addr;
	ssize_t bytesread = 0;
	char buf[4096];
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	printf("Server socket fd = %d\n", sockfd);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	snprintf( addr.sun_path, sizeof(addr.sun_path) - 1, USOCKET, bpid );
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

	while ((bytesread = read(bashcfd, buf, 4096)) > 0) {
		buf[bytesread] = 0;
		printf("%s", buf );
    }

	if (bytesread == -1) {
		perror("read");
		return 4;
    }

    close(bashcfd);
	return 0;
}

int main() {

	bpid = vfork();
	if( bpid == 0 ){
		printf(" Starting Child\n");
		//bashprocess();

		char *argv[32] = { "./myvforkchild", 0} ;
		int rc = execvp( argv[0], argv);
		if( rc == -1 ){
			perror("execvp");
		}
	} else if( bpid > 0 ){
		printf(" Starting Parent : child pid = %d \n",bpid);
		serverprocess();
		int rc;
	} else {
		printf(" Starting Parent did not happen\n");
		return 5;
	}

	printf(" exiting %d \n", bpid );
	return 0;
}
