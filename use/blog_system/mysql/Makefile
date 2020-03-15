.PHONY:all
all:insert select


insert:insert.cc
	g++ insert.cc -o insert -L/usr/lib64/mysql -lmysqlclient

select:select.cc
	g++ select.cc -o select -L/usr/lib64/mysql -lmysqlclient

.PHONY:clean
clean:
	rm insert
