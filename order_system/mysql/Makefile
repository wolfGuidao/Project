.PHONY:all
all:select insert

select:select.cc
	g++ $^ -o $@ -std=c++11 -L/usr/lib64/mysql -lmysqlclient

insert:insert.cc
	g++ $^ -o $@ -std=c++11 -L/usr/lib64/mysql -lmysqlclient
