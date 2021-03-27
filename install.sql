create table plugins ( id integer primary key autoincrement , name varchar(32), active integer  );
insert into  plugins ( name, active) values ( 'p3', 1 );
create table acl     ( id integer primary key autoincrement, ipaddr integer );
insert into  acl     ( ipaddr ) values (2130706433);
create table sessions( id integer , ipaddr integer, eos integer, cookie integer, name varchar(32), value varchar(32), primary key ( id, ipaddr, eos, name) );
