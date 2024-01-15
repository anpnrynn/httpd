//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#ifndef PR_WRAPPER_H
#define PR_WRAPPER_H

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#define _FILE_OFFSET_BITS 64

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include <dlfcn.h>
#include <poll.h>

#define PRFileDesc   int32_t
#define PRFileInfo64 struct stat
#define PRErrorCode  int32_t
#define PRLibrary    void
#define PRNetAddr    struct sockaddr_in
#define PRNetAddr6   struct sockaddr_in6
#define PRStatus     int32_t
#define PRPollDesc   struct pollfd

//functions

#define PR_Sleep(secs)         sleep(secs)
//#define PR_Sleep(msecs)      usleep((msecs)*1000)
#define PR_GetError()          errno
#define PR_GetOSError()        strerror(errno)
#define PR_GetErrorText(str)   perror(str)


//IO defines

#define PR_CREATE_FILE O_CREAT
#define PR_TRUNCATE    O_TRUNC
#define PR_RDWR        O_RDWR
#define PR_RDONLY      O_RDONLY
#define PR_WRONLY      O_WRONLY

//IO modes
#define PR_IRWXO       S_IRWXO
#define PR_IRWXG       S_IRWXG
#define PR_IRWXU       S_IRWXU

//IO Seek
#define PR_SEEK_SET    SEEK_SET
#define PR_SEEK_END    SEEK_END
#define PR_SEEK_CUR    SEEK_CUR

//IO functions

#define PR_Open(filename,flags,mode)     open((filename),(flags),(mode))
#define PR_Read(fd,buf,len)              read(*(fd),(buf),(len))
#define PR_Write(fd,buf,len)             write(*(fd),(buf),(len))
#define PR_Close(fd)                     close(*(fd))
#define PR_Closefd(fd)                   close((fd))
#define PR_Seek64(fd,offset,whence)      lseek((*fd),(offset),(whence))
#define PR_GetFileInfo64(filename,s)     fstat((filename),s)
#define PR_Unlink(filename)              unlink((filename))

//DLLs

#define PR_LoadLibrary(name)           dlopen((name),RTLD_NOW)
#define PR_FindSymbol(lib,symname)     dlsym((lib),(symname))





#define PR_NewTCPSocket()              socket(AF_INET,  SOCK_STREAM, 0)
#define PR_NewTCPSocket6()             socket(AF_INET6, SOCK_STREAM, 0)
#define PR_AF_INET                     AF_INET
#define PR_AF_INET6                    AF_INET6
#define PR_htons(port)                 htons(port)
#define PR_htonl(addr)                 htonl(addr)
#define PR_ntohs(port)                 ntohs(port)
#define PR_ntohl(addr)                 ntohl(addr)
#define PR_Bind(fd,addr)               bind((*fd),((const sockaddr*)(addr)),sizeof(*(addr))
#define PR_SUCCESS                     0
#define PR_FAILURE                     !(PR_SUCCESS)
#define PR_Cleanup()
#define PR_Listen(fd,count)            listen((*fd),(count))

#define PR_SHUTDOWN_BOTH               SHUT_RDWR
#define PR_SHUTDOWN_READ               SHUT_RD
#define PR_SHUTDOWN_WRITE              SHUT_WR
#define PR_Shutdown(fd,how)            shutdown(*(fd),(how))
#define PR_Shutdownfd(fd,how)          shutdown((fd),(how))
#define PR_Recv(sockfd,buf,len,flags,flags2)  recv(*(sockfd),(buf),(len),(flags))
#define PR_Recvfd(sockfd,buf,len,flags,flags2)  recv((sockfd),(buf),(len),(flags))
#define PR_Accept(sockfd,sock,count)            accept(*(sockfd),((const sockaddr*)(sock)),sizeof(*sock))

#define PR_POLL_IN           POLLIN
#define PR_POLL_READ         POLLIN
#define PR_POLL_PRI          POLLPRI
#define PR_POLL_EXCEPT       POLLPRI
#define PR_POLL_OUT          POLLOUT
#define PR_POLL_WRITE        POLLOUT
#define PR_POLL_RDHUP        POLLRDHUP
#define PR_POLL_ERR          POLLERR
#define PR_POLL_HUP          POLLHUP
#define PR_POLL_NVAL         POLLNVAL

#define PR_WOULD_BLOCK_ERROR EWOULDBLOCK


#define PR_Poll                        poll




#endif
