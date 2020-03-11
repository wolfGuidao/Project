all : server
	
server: server.cc ../include/httplib.h
	g++ server.cc -o $@ -std=c++11 -lpthread

.PHONY: clean

clean: server
	rm -f $^

