#Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
INC_DIR=-I./ -I/usr/include -I/usr/local/include

#SSL=-DUSE_SSL
#SSLINC=
#SSLLIB=-lssl -lcrypto

LIB_DIR=-L/usr/lib -L/usr/local/lib -L./
LIBS=-lsqlite3 -lpthread -ldl $(SSLLIB)
ELIBS=-lhttp 

SHELLFLAGS=-DUSE_SHELL

CFLAGS=-Wall -DLINUX_BUILD -DUSE_CPP11THREAD $(SSL) -fPIC $(SHELLFLAGS)
CPPFLAGS=-std=c++11
LFLAGS=-DCOMPILER_C_LINKAGE
DEBUG=-O2

OBJS=cookie.o httpcodes.o httpconn.o httphandlers.o mimetypes.o plugin.o session.o tools.o log.o
SOBJS=threadmgr.o server.o
MAKELIBS=

PLUGINS=libp3.so liblogin.so libfileupload.so

#Change these
INSTALL_HOME=/tmp
INSTALL_PAGE_STORE=/tmp
SERVER_HOME=$(INSTALL_HOME)/httpd
CERT_STORE=$(INSTALL_HOME)/httpd/share
PAGE_STORE=$(INSTALL_PAGE_STORE)/var/www/Pages

all:libhttp.so httpdsrv 

httpdsrv:libhttp.so libp3.so liblogin.so libfileupload.so libbash.so vforkchild $(SOBJS) 
	g++ -rdynamic -o httpdsrv $(SOBJS) $(INC_DIR) $(LIB_DIR) $(LIBS) $(ELIBS) $(CFLAGS) $(DEBUG)

vforkchild:vforkchild.cpp
	g++ vforkchild.cpp -o vforkchild $(INC_DIR) $(CFLAGS) $(DEBUG)

libhttp.so:$(OBJS)
	g++ $(OBJS) -shared -o libhttp.so $(INC_DIR) $(LIB_DIR) $(LIBS) $(CFLAGS) $(DEBUG)

server.o:server.cpp
	g++ server.cpp -c -o server.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(LFLAGS) $(CPPFLAGS)

tools.o:tools.cpp tools.h
	g++ tools.cpp -c -o tools.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

httpconn.o:httpconn.cpp httpconn.h tools.o version.h
	g++ httpconn.cpp -c -o httpconn.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

config.h:
	echo "#ifndef CONFIG_H"  > config.h
	echo "#define CONFIG_H" >> config.h
	echo "" >> config.h
	echo "#define INSTALL_HOME \"$(INSTALL_HOME)\"" >> config.h
	echo "#define INSTALL_PAGE_STORE \"$(INSTALL_PAGE_STORE)\"" >> config.h
	echo "#endif" >> config.h

cookie.o:cookie.cpp cookie.h defines.h config.h
	g++ cookie.cpp -c -o cookie.o  $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

log.o:log.cpp log.h
	g++ log.cpp -c -o log.o  $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

httphandlers.o: httphandlers.cpp httphandlers.h 
	g++ httphandlers.cpp -c -o httphandlers.o  $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

httpcodes.o: httpcodes.cpp httpcodes.h 
	g++ httpcodes.cpp -c -o httpcodes.o  $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

mimetypes.o: mimetypes.cpp mimetypes.h
	g++ mimetypes.cpp -c -o mimetypes.o  $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

session.o: session.cpp session.h
	g++ session.cpp -c -o session.o  $(INC_DIR) $(CFLAGS) $(DEBUG) $(LFLAGS) $(CPPFLAGS)

plugin.o: plugin.cpp plugin.h
	g++ plugin.cpp -c -o plugin.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

threadmgr.o: threadmgr.cpp threadmgr.h
	g++ threadmgr.cpp -c -o threadmgr.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(LFLAGS) $(CPPFLAGS)

liblogin.so: login.cpp
	g++ login.cpp -shared -o liblogin.so $(INC_DIR) $(CFLAGS) $(DEBUG) $(LIB_DIR) $(LIBS) $(CPPFLAGS)

libfileupload.so: fileupload.cpp
	g++ fileupload.cpp -shared -o libfileupload.so $(INC_DIR) $(CFLAGS) $(DEBUG) $(LIB_DIR) $(LIBS) $(CPPFLAGS)

libp3.so: p3.cpp
	g++ p3.cpp -shared -o libp3.so $(INC_DIR) $(CFLAGS) $(DEBUG) $(LIB_DIR) $(LIBS) $(CPPFLAGS) 

libbash.so: bash.cpp chunkedencoding.h
	g++ bash.cpp -shared -o libbash.so $(INC_DIR) $(CFLAGS) $(DEBUG) $(LIB_DIR) $(LIBS) $(CPPFLAGS) 

clean:
	rm -f config.h *.o httpdsrv libp1.so libp2.so libbash.so $(PLUGINS) libhttp.so core.*

distclean:
	rm -f config.h *.o httpdsrv libp1.so libp2.so libp3.so liblogin.so libhttp.so core.* /var/local/infostore.sqlite3 libbash.so vforkchild

install:
	rm -f /var/local/infostore.sqlite3
	mkdir -p $(INSTALL_HOME)/httpd/bin/
	mkdir -p $(INSTALL_HOME)/httpd/include/
	mkdir -p $(INSTALL_HOME)/httpd/lib/
	mkdir -p $(INSTALL_HOME)/httpd/share/
	mkdir -p $(INSTALL_PAGE_STORE)/var/www/Pages/
	cp -f install.sql $(INSTALL_HOME)/httpd/share/
	cp -f httpdsrv httpdsrv.sh $(INSTALL_HOME)/httpd/bin/
	cp -f vforkchild contextscript.bash $(INSTALL_HOME)/httpd/bin/
	chmod 755 $(INSTALL_HOME)/httpd/bin/httpdsrv $(INSTALL_HOME)/httpd/bin/httpdsrv.sh
	chmod 755 $(INSTALL_HOME)/httpd/bin/vforkchild $(INSTALL_HOME)/httpd/bin/contextscript.bash
	cp -f libhttp.so libp3.so liblogin.so libbash.so $(INSTALL_HOME)/httpd/lib/
	chmod 755 $(INSTALL_HOME)/httpd/lib/libhttp.so $(INSTALL_HOME)/httpd/lib/libp3.so $(INSTALL_HOME)/httpd/lib/liblogin.so $(INSTALL_HOME)/httpd/lib/libbash.so 
	cp -Rfp Pages/* $(INSTALL_PAGE_STORE)/var/www/Pages/ 
	cp -f *.h $(INSTALL_HOME)/httpd/include/
	chmod 744 $(INSTALL_HOME)/httpd/include/*.h


installclean:
	rm -rf $(INSTALL_HOME)/httpd/ $(INSTALL_PAGE_STORE)/var/www/Pages /var/local/infostore.sqlite3
