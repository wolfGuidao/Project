FLAGS=-I ~/third_part/include -L ~/third_part/lib \
			-lpthread -ljsoncpp -lctemplate -std=c++11
HPP_FILE=$(shell ls *.hpp *.h)

.PHONY:all
all:compile_server oj_server

compile_server:compile_server.cc $(HPP_FILE) 
	g++ compile_server.cc -o $@ $(FLAGS)

oj_server:oj_server.cc $(HPP_FILE)
	g++ oj_server.cc -o $@ $(FLAGS)

.PHONY:clean
clean:
	rm compile_server oj_server
