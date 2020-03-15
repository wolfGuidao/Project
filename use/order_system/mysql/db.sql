create database if not exists order_system;
use order_system;

create table if not exists dish_table(
  dish_id int not null primary key auto_increment,
  name varchar(50),
  price int 
);

insert into dish_table values(null,'红烧肉',2200);
insert into dish_table values(null,'回锅肉',2200);
insert into dish_table values(null,'糖醋里脊',2200);
insert into dish_table values(null,'红烧鱼块',2200);

create table if not exists order_table(
  order_id int not null primary key auto_increment,
  table_id varchar(50),
  time varchar(50),
  dishes varchar(1024),
  status int
);

insert into order_table values(null,'武当派','2020/01/01','1,2,3',0);
insert into order_table values(null,'少林派','2020/01/01','1',0);
insert into order_table values(null,'华山派','2020/01/01','1,2',0);
