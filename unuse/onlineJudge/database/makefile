tst: ./dataBaseMysql.hpp ./tstDBMysql.cpp 
	g++ tstDBMysql.cpp -o $@ -std=c++11 -lmysqlclient -L/usr/lib64/mysql/

.PHONY: clean

clean: tst
	rm -f $^
