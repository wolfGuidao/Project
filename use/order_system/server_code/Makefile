FLAG=-std=c++11 -L/usr/lib64/mysql -lmysqlclient -ljsoncpp -lpthread

.PHONY:all
all:order_server db_test

order_server:order_server.cc
	g++ $^ -o $@ $(FLAG) 

db_test:db_test.cc
	g++ $^ -o $@ $(FLAG)
