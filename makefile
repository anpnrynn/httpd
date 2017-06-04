#INC_DIR=-I./ -I./nspr-4.6.4/include/ -I/usr/local/include -I../Encode
INC_DIR=-I./ -I/usr/local/include -I/usr/include/nspr4 

#LIB_DIR=-L./nspr-4.6.4/lib/ -L/usr/local/lib -L./
LIB_DIR=-L/usr/local/lib -L./
LIBS=-lsqlite3 -lplds4 -lplc4 -lnspr4 -lpthread -ldl -lminiupnpc -lpthread
ELIBS=-lhttp 
#-lplc4 -lplds4
CFLAGS=-Wall -DLINUX_BUILD -DUSE_MINIUPNP -DUSE_PTHREAD -fPIC -Wwrite-strings
CPPFLAGS=-std=c++11
LFLAGS=-DCOMPILER_C_LINKAGE
DEBUG=-g3
#DEBUG=-O2
OBJS=cookie.o httpcodes.o httpconn.o httphandlers.o mimetypes.o plugin.o session.o tools.o Md5.o upnp.o
#SOBJS=threadmgr.o server.o login.o
SOBJS=threadmgr.o server.o
#httpconn.o tools.o httpcodes.o mimetypes.o session.o threadmgr.o httphandlers.o plugin.o base64.o pass.o
MAKELIBS=

#all:libhttp.so libpass.so libp1.so libp2.so httpdsrv 
all:libhttp.so httpdsrv 


#httpdsrv:libhttp.so libp1.so libp2.so libp3.so $(SOBJS) 
httpdsrv:libhttp.so libp3.so $(SOBJS) 
	g++ -rdynamic -o httpdsrv $(SOBJS) $(INC_DIR) $(LIB_DIR) $(LIBS) $(ELIBS) $(CFLAGS) $(DEBUG)

libhttp.so:$(OBJS)
	g++ $(OBJS) -shared -o libhttp.so $(INC_DIR) $(LIB_DIR) $(LIBS) $(CFLAGS) $(DEBUG)

server.o:server.cpp mozhdr.h
	g++ server.cpp -c -o server.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(LFLAGS) $(CPPFLAGS)

tools.o:tools.cpp tools.h
	g++ tools.cpp -c -o tools.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

upnp.o:upnp.cpp upnp.h
	g++ upnp.cpp -c -o upnp.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

Md5.o:Md5.c
	g++ Md5.c -c -o Md5.o $(INC_DIR) $(CFLAGS) $(DEBUG)

httpconn.o:httpconn.cpp httpconn.h tools.o version.h
	g++ httpconn.cpp -c -o httpconn.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

cookie.o:cookie.cpp cookie.h
	g++ cookie.cpp -c -o cookie.o  $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

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

libpass.so:pass.o base64.o 
	g++ pass.o base64.o -shared -o libpass.so $(INC_DIR) $(CFLAGS) $(DEBUG) $(LIB_DIR) $(LIBS) $(CPPFLAGS)

pass.o: pass.c pass.h base64.o
	g++ pass.c -c -o pass.o  -DIS_HTTPD $(INC_DIR) $(CFLAGS) $(DEBUG) $(LFLAGS) $(CPPFLAGS)

base64.o: base64.c base64.h
	g++ base64.c -c -o base64.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)

#login.o: login.cpp
#	g++ login.cpp -c -o login.o $(INC_DIR) $(CFLAGS) $(DEBUG) $(CPPFLAGS)
#
#libp1.so: p1.cpp
#	g++ p1.cpp -shared -o libp1.so $(INC_DIR) $(CFLAGS) $(DEBUG) $(LIB_DIR) $(LIBS) $(CPPFLAGS) 
#
#libp2.so: p2.cpp
#	g++ p2.cpp -shared -o libp2.so $(INC_DIR) $(CFLAGS) $(DEBUG) $(LIB_DIR) $(LIBS) $(CPPFLAGS) 

libp3.so: p3.cpp
	g++ p3.cpp -shared -o libp3.so $(INC_DIR) $(CFLAGS) $(DEBUG) $(LIB_DIR) $(LIBS) $(CPPFLAGS) 

clean:
	rm -f *.o httpdsrv libp1.so libp2.so libp3.so libhttp.so libpass.so core.*
