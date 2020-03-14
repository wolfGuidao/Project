all: oj_server

ojServer: ./serverSrc/main.cpp 
	g++ ./serverSrc/main.cpp -o ojServer -std=c++11 -lpthread\
		-ljsoncpp -lctemplate \
		-L/usr/lib64/mysql -lmysqlclient  -lm -lrt -ldl\

oj_server: ./serverSrc/oj_server.cpp ./serverSrc/oj_model.hpp ./serverSrc/oj_view.hpp
	g++  ./serverSrc/oj_server.cpp -o oj_server -std=c++11 -lpthread\
		-ljsoncpp -lctemplate 

main: main.cpp httplib.h compiler.hpp
	g++ main.cpp -o $@ -g -std=c++11 -lpthread -ljsoncpp
test: test.cpp httplib.h
	g++ test.cpp -o $@ -std=c++11 -lpthread -ljsoncpp

.PHONY: clean

clean: oj_server
	rm -f $^
