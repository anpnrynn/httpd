create table plugins ( id integer primary key autoincrement , name varchar(32), active integer  );
insert into  plugins ( name, active) values ( 'p3', 1 );
insert into  plugins ( name, active) values ( 'login', 1);
insert into  plugins ( name, active) values ( 'fileupload', 1);
insert into  plugins ( name, active) values ( 'bookrest', 1);
create table sessions( id integer , ipaddr integer, eos integer, cookie integer, name varchar(32), value varchar(32), primary key ( id, ipaddr, eos, name) );
create table ulogin  ( id integer primary key autoincrement , name varchar(40), passwd varchar(40), power integer , lpage varchar(128));
insert into ulogin values ( 0, 'super', 'superpassword',  4 , 'super.html' );
insert into ulogin values ( 1, 'root',  'rootpassword',   3 , 'root.html' );
insert into ulogin values ( 2, 'admin', 'adminpassword',  2 , 'admin.html' );
create table acl     ( id integer primary key autoincrement, invert boolean, ipv4 boolean, subnetmask boolean, prefixmask boolean, ip varchar(48) );
