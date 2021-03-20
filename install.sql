create table config  ( id integer primary key, name varchar(64), value varchar(64) );
insert into  config values ( 0, 'isPrimary' , '1');
insert into  config values ( 1, 'primary' , 'empty');

create table primarydatatype ( pdid integer primary key autoincrement, pdname varchar (20) );
insert into  primarydatatype values( 0, 'Boolean' );
insert into  primarydatatype values( 1, 'Number');
insert into  primarydatatype values( 2, 'Decimal');
insert into  primarydatatype values( 3, 'String' );
insert into  primarydatatype values( 4, 'Date' );
insert into  primarydatatype values( 5, 'Time' );
insert into  primarydatatype values( 6, 'Timestamp' );
insert into  primarydatatype values( 7, 'Date-Range' );
insert into  primarydatatype values( 8, 'Time-Slot' );
insert into  primarydatatype values( 9, 'Single Choice' );
insert into  primarydatatype values( 10, 'Single Choice List' );
insert into  primarydatatype values( 11, 'Multiple Choice' );
insert into  primarydatatype values( 12, 'Multiple Choice List' );
insert into  primarydatatype values( 13, 'Attachment' );
insert into  primarydatatype values( 14, 'XML-Data' );
insert into  primarydatatype values( 15, 'Program Data');
insert into  primarydatatype values( 16, 'Periodicity');

create table jsconfig( name varchar(20) primary key, value varchar(20) , type smallint,  foreign key (type) references primarydatatype(pdid) );
insert into jsconfig values ('isIE', '0', 0);
insert into jsconfig values ('showExpWiz', '1', 0);
insert into jsconfig values ('showExpHelp', '1', 0 );

create table cssconfig ( name varchar(20) primary key, type smallint, value varchar(20), foreign key (type) references primarydatatype(pdid) );

create table plugins ( id integer primary key autoincrement , name varchar(32), active integer  );
insert into plugins ( name, active) values ( 'p3', 1 );

create table help ( hid integer primary key autoincrement, title varchar(40), mandatory smallint, defvalue varchar(10), information varchar(100));
insert into  help values ( 100 , 'Resource Id', 0, '0', 'This is a number used to uniquely identify a resource.' );
insert into  help values ( 101 , 'Resource Name', 1, '', 'A name given to this resource to identify the its contents. Ex: Groceries, Vegetables, People... etc' );
insert into  help values ( 102 , 'Description', 0, '', 'Description of this resource.' );
insert into  help values ( 103 , 'Type', 1, 'Inventory/Products', 'What kind of resource is this ? There are three types of resources 1.Inventory/Products, 2. Spaces, 3.Products' );
insert into  help values ( 104 , 'Visibility', 1, 'Public',  'Do you want the details/contents of this resource to be made visible to the whole world ?' );
insert into  help values ( 105 , 'Demarcation', 0, 'No','Do you want to create a clear demarcation of this resource on the website ? In the case of Inventory/Products you may not need it, however in the case of people and space related resource you may need it.' );
insert into  help values ( 106 , 'Advance Mgmt', 1, '30', 'How many days in advance would you like to manage the resource ?' );
insert into  help values ( 107 , 'Warning Level', 1, '10', 'At what level should a warning be raised when this resource starts to deplet ? This is a percentage value.' );

create table wizard ( wid integer , sid integer, mtitle varchar(40), stitle varchar(40), msg varchar(100), primary key ( wid, sid) );
create table wizoptions( wid integer , sid integer, opt integer , primary key ( wid, sid, opt ) , foreign key ( wid) references wizard(wid), foreign key (sid) references wizard(sid), foreign key (opt) references wizard(wid) );

create table acl     ( id integer primary key autoincrement, ipaddr integer );
insert into acl ( ipaddr ) values (2130706433);
create table sessions( id integer , ipaddr integer, eos integer, cookie integer, name varchar(32), value varchar(32), primary key ( id, ipaddr, eos, name) );

create table ulogin  ( id integer primary key autoincrement , name varchar(40), passwd varchar(40), power integer , lpage varchar(20));
insert into ulogin values ( 0, 'root', 'rootzilla', 4 , 'xxx.xul' );
insert into ulogin values ( 1, 'admin', 'adminzilla', 3 , 'estctrlwin.xul' );
insert into ulogin values ( 2, 'manager', 'managerzilla', 2 , 'estctrlwin.xul' );

create table qcmds (qid integer , powerreq smallint, qtype smallint, paramnum smallint, key smallint, qcmd varchar(200), primary key (qid,powerreq));
insert into qcmds values ( 1, 3, 2, 2, 0,  'update estdetails set datavalue="$s" where dataname="$s";' );
insert into qcmds values ( 2, 1, 0, 0, 0,  'select dataname, datavalue, type from estdetails where id<100;' );
insert into qcmds values ( 3, 3, 0, 0, 0,  'select dataname, datavalue, type from estdetails where id>99;' );
insert into qcmds values ( 4, 3, 4, 0, 0,  'insert into products ' );
insert into qcmds values ( 5, 3, 1, 0, 0,  'select * from products; ');
insert into qcmds values ( 6, 1, 1, 2, 0,  'select power,lpage,id from ulogin where name="$s" and passwd="$s"; ');
insert into qcmds values ( 7, 3, 1, 0, 0,  'select * from estdetails; ');
insert into qcmds values ( 20, 3, 1, 0, 0, 'select * from resource;');
insert into qcmds values ( 21, 3, 3, 13, 1, 'insert into resource values( ( select max(rid) from resource )+1, "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s"  );');
insert into qcmds values ( 22, 3, 3, 14, 0, 'replace into resource values( "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s", "$s");');
insert into qcmds values ( 23, 3, 3, 1, 0, 'select * from resource where type = "$s";');
insert into qcmds values ( 24, 3, 3, 1, 0, 'select * from resource where rid = "$s";');
insert into qcmds values ( 25, 3, 2, 1, 0, 'delete from resource where rid = "$s";');
insert into qcmds values ( 40, 3, 1, 0, 0, 'select * from service;');
insert into qcmds values ( 41, 3, 1, 0, 0, 'select service.sid, sname, sdesc, spack, spovr, stype, periodicy, paymentcycle, importance, visibility, delivtype, advbook, autoacpt, stime, scharge from service, servicepack where servicepack.sid = "$s" and  service.sid = servicepack.spid;');
insert into qcmds values ( 42, 3, 3, 13, 1, 'insert  into service values( ( select max(sid) from service )+1, "$s", "$s", "$s", "$s", "$s","$s", "$s", 0, "$s", "$s", "$s", "$s", "$s", "$s"  );');
insert into qcmds values ( 43, 3, 3, 14, 0, 'replace into service values( "$s", "$s", "$s", "$s", "$s", "$s","$s", "$s", 0, "$s", "$s", "$s", "$s", "$s", "$s");');
insert into qcmds values ( 44, 3, 3, 1, 0, 'select * from service where spack = "$s";');
insert into qcmds values ( 45, 3, 3, 1, 0, 'select * from service where sid = "$s";');
insert into qcmds values ( 46, 3, 2, 4, 0, 'delete from servicedepend where sid = "$s"; delete from servicepack where sid = "$s" or spid="$s"; delete from service where sid = "$s";');
insert into qcmds values ( 47, 3, 3, 1, 0, 'select sid,resource.rid,rname,type from servicedepend, resource where servicedepend.sid = "$s" and servicedepend.rid = resource.rid;');
insert into qcmds values ( 48, 3, 3, 1, 0, 'select rid,service.sid,sname,stype from servicedepend, service where servicedepend.rid = "$s" and servicedepend.sid = service.sid;');
insert into qcmds values ( 49, 3, 2, 2, 0, 'insert into servicedepend values ( "$s", "$s" );');
insert into qcmds values ( 50, 3, 2, 2, 0, 'insert into servicedepend ( rid, sid ) values ( "$s", "$s" );');
insert into qcmds values ( 51, 3, 3, 1, 0, 'delete from servicedepend where rid = "$s";');
insert into qcmds values ( 52, 3, 3, 1, 0, 'delete from servicedepend where sid = "$s";');
insert into qcmds values ( 53, 3, 3, 1, 0, 'delete from servicedepend where sid = "$s" and rid = "$s";');
insert into qcmds values ( 54, 3, 3, 1, 0, 'delete from servicedepend where rid = "$r" and sid = "$s";');
insert into qcmds values ( 55, 3, 3, 1, 0, 'select * from servicedepend where rid = "$s";');
insert into qcmds values ( 56, 3, 3, 1, 0, 'select * from servicedepend where sid = "$s";');
insert into qcmds values ( 57, 3, 1, 0, 0, 'select * from servicedepend;');
insert into qcmds values ( 58, 3, 2, 2, 0, 'insert into servicepack values ( "$s", "$s" );');
insert into qcmds values ( 59, 3, 3, 1, 0, 'delete from servicepack where sid = "$s" or spid="$s";');
insert into qcmds values ( 60, 3, 3, 1, 0, 'select * from servicepack where sid = "$s";');
insert into qcmds values ( 61, 3, 3, 1, 0, 'select * from servicepack where spid = "$s";');
insert into qcmds values ( 62, 3, 1, 0, 0, 'select * from servicepack;');

create table jsconfiguser( uid integer, name varchar(20) ,value varchar(20) , type smallint,  foreign key (uid) references ulogin(uid), foreign key (type) references primarydatatype(pdid), primary key (uid, name) );
insert into jsconfiguser values (0, 'isIE', '0', 0);
insert into jsconfiguser values (0,'showExpWiz', '1', 0);
insert into jsconfiguser values (0,'showExpHelp', '1' , 0);
insert into jsconfiguser values (1, 'isIE', '0', 0);
insert into jsconfiguser values (1,'showExpWiz', '1', 0);
insert into jsconfiguser values (1,'showExpHelp', '1' , 0);
insert into jsconfiguser values (2, 'isIE', '0', 0);
insert into jsconfiguser values (2,'showExpWiz', '1', 0);
insert into jsconfiguser values (2,'showExpHelp', '1' , 0);

create table cssconfiguser( uid integer, name varchar(20) ,value varchar(20) , type smallint,  foreign key (uid) references ulogin(uid), foreign key (type) references primarydatatype(pdid), primary key (uid, name) );

create table estdetails ( id integer primary key autoincrement, dataname varchar(40), datavalue varchar(40), type integer, foreign key (type) references primarydatatype(pdid));
insert into estdetails values ( 0, 'estname', '' , 3 );
insert into estdetails values ( 1, 'description', '' , 3 );
insert into estdetails values ( 2, 'category', '' , 3 );
insert into estdetails values ( 3, 'tags', '' , 3 );
insert into estdetails values ( 4, 'aline1', ''   , 3 );
insert into estdetails values ( 5, 'aline2', ''   , 3 );
insert into estdetails values ( 6, 'city', ''     , 3 );
insert into estdetails values ( 7, 'state', ''    , 3 );
insert into estdetails values ( 8, 'country', ''  , 3 );
insert into estdetails values ( 9, 'pincode', ''  , 1 );
insert into estdetails values ( 10, 'landmark', ''     , 3 );
insert into estdetails values ( 11, 'phone1', ''   , 3 );
insert into estdetails values ( 12, 'phone2', ''   , 3 );
insert into estdetails values ( 13, 'phone3', ''   , 3 );
insert into estdetails values ( 14, 'mobile1', '' , 3 );
insert into estdetails values ( 15, 'mobile2', '' , 3 );
insert into estdetails values ( 16, 'mobile3', '' , 3 );
insert into estdetails values ( 17, 'email', ''  , 3 );
insert into estdetails values ( 18, 'web', ''  , 3 );
insert into estdetails values ( 19, 'contact', ''  , 3 );
insert into estdetails values ( 20, 'tin', ''     , 3 );
insert into estdetails values ( 21, 'cst', ''     , 3 );
insert into estdetails values ( 22, 'mapid', ''   , 3 );
insert into estdetails values ( 23, 'maploc', '' , 3 );
insert into estdetails values ( 24, 'delvopt', '' , 3 );
insert into estdetails values ( 25, 'delvpolicy', '' , 3 );
insert into estdetails values ( 26, 'delvlocations', '' , 3 );
insert into estdetails values ( 27, 'paypolicy', '' , 3 );
insert into estdetails values ( 28, 'callow', '' , 3 );
insert into estdetails values ( 29, 'ccharge', '' , 3 );
insert into estdetails values ( 30, 'cponr', '' , 3 );
insert into estdetails values ( 31, 'cancelpolicy', '' , 3 );
insert into estdetails values ( 32, 'drpolicy', '' , 3 );
insert into estdetails values ( 33, 'exchgpolicy', '' , 3 );
insert into estdetails values ( 34, 'othtitle', '' , 3 );
insert into estdetails values ( 35, 'othpolicy', '' , 3 );
insert into estdetails values ( 36, 'workhours', ''   , 3 );
insert into estdetails values ( 37, 'workdays', ''    , 3 );
insert into estdetails values ( 38, 'paymethodta', ''    , 3 );
insert into estdetails values ( 39, 'paymethodtaoth', ''    , 3 );
insert into estdetails values ( 40, 'paymethodhd', ''    , 3 );
insert into estdetails values ( 41, 'paymethodhdoth', ''    , 3 );
insert into estdetails values ( 100, 'uname', ''     , 3 );
insert into estdetails values ( 101, 'password', ''     , 3 );

create table datatype ( did integer primary key autoincrement, primarytype integer , datatype varchar (40) , datatypeopt varchar(40) ,foreign key(primarytype) references primarydatatype(pdid));
insert into datatype values ( 1000, 100, 'Order/Booking Information', ''); 
insert into datatype values ( 1,  0,  'Allow/Deny' , '');
insert into datatype values ( 2,  0,  'On/Off', '');
insert into datatype values ( 3,  0,  'Enable/Disable', '' );
insert into datatype values ( 4,  0,  'True/False', '' );
insert into datatype values ( 5,  0,  'Select/De-Select', '' );
insert into datatype values ( 6,  0,  'Male/Female' , '');
insert into datatype values ( 7,  0,  'Man/Woman' , '');
insert into datatype values ( 8,  0,  'Boy/Girl' , '');
insert into datatype values ( 10,  1,  'Number' , '');
insert into datatype values ( 11,  1,  'Integer' , '');
insert into datatype values ( 12,  1,  'Pincode', '');
insert into datatype values ( 13,  1,  'Age', '');
insert into datatype values ( 20,  2,  'Float', '');
insert into datatype values ( 21,  2,  'Double', '');
insert into datatype values ( 22,  2,  'Decimal Point Number', '');
insert into datatype values ( 30,  3,  'Alpha-Numeric Value', '');
insert into datatype values ( 31,  3,  'String', '');
insert into datatype values ( 32,  3,  'Phone', '');
insert into datatype values ( 33,  3,  'Mobile', '');
insert into datatype values ( 34,  3,  'E-Mail', '');
insert into datatype values ( 35,  3,  'Web Address', '');
insert into datatype values ( 36,  3,  'WWW', '');
insert into datatype values ( 37,  3,  'Full Address', '');
insert into datatype values ( 38,  3,  'Aline-1', '');
insert into datatype values ( 39,  3,  'Aline-2', '');
insert into datatype values ( 40,  3,  'City', '');
insert into datatype values ( 41,  3,  'State', '');
insert into datatype values ( 42,  3,  'Country', '');
insert into datatype values ( 43,  3,  'Username', '');
insert into datatype values ( 44,  3,  'Password', '');
insert into datatype values ( 45,  3,  'Customer ID', '');
insert into datatype values ( 46,  3,  'Rebate ID', '');
insert into datatype values ( 60,  4,  'Date', '');
insert into datatype values ( 61,  4,  'Delivery Date', '');
insert into datatype values ( 62,  4,  'Booking Date', '');
insert into datatype values ( 63,  4,  'Appointment Date', '');
insert into datatype values ( 64,  4,  'Shipping Date', '');
insert into datatype values ( 70,  5,  'Time', '');
insert into datatype values ( 71,  5,  'Delivery Time', '');
insert into datatype values ( 72,  5,  'Booking Time', '');
insert into datatype values ( 73,  5,  'Appointment Time', '');
insert into datatype values ( 74,  5,  'Shipping Time', '');
insert into datatype values ( 80,  6,  'Time-Stamp', '');
insert into datatype values ( 81,  6,  'Date & time', '');
insert into datatype values ( 90,  7,  'Date-Range', '');
insert into datatype values ( 100,  8,  'Time-Slot', '');
insert into datatype values ( 101,  8,  'Time-Range', '');

insert into datatype values ( 110,  9,  'Select a day', 'Sun,Mon,Tue,Wed,Thu,Fri,Sat');
insert into datatype values ( 111,  10, 'Select a day', 'Sun,Mon,Tue,Wed,Thu,Fri,Sat');
insert into datatype values ( 120,  11, 'Select days',  'Sun,Mon,Tue,Wed,Thu,Fri,Sat');
insert into datatype values ( 121,  12, 'Select days',  'Sun,Mon,Tue,Wed,Thu,Fri,Sat');

insert into datatype values ( 130,  13,  'File' , '');
insert into datatype values ( 131,  13,  'Attachment', '');
insert into datatype values ( 140,  14,  'XML', '');

create table customer ( cid integer primary key autoincrement, name varchar (40), id varchar(40), cgrpid smallint, trusted smallint, dob date, sex smallint, age smallint,  email varchar(40),  phone varchar(15), mobile varchar(15), aline1 varchar(40), aline2 varhar(40), city varchar (40), state varchar(40), country varchar(40), distance float(2),  purchase float(2), extra varchar(10) );

insert into  customer values ( 0,'unknown', 'unknown@unknown.com',1, 0, '2000-01-01', 0 , 0,  'unknown@unknown.com', '0000000000', '0000000000', '','','','','',0, 0, '' );

create table customerextra ( cid integer , dataname varchar (40) ,datavalue varchar(40) ,  datatype smallint, foreign key (datatype) references primarydatatype(pdid));

create table dinfo    ( diid integer primary key autoincrement, dname varchar(40), overlay varchar(40), dtype smallint );

insert into  dinfo values ( 1, 'Cancellation Policy',     'cpolicy.xul',  3);
insert into  dinfo values ( 2, 'Delayed Return Policy','drpolicy.xul' , 3);
insert into  dinfo values ( 3, 'Advance Charges',       'advcharges.xul', 2);

create table occurrence (ocid integer primary key autoincrement,  available smallint, selflag integer, daysflag integer, weeksflag integer, datesflag integer, date1 date, date2 date, stime integer, etime integer);

create table resource ( rid integer primary key autoincrement, rname varchar (40) default '', rdesc varchar (40) default '', type smallint default 0, visibility smallint default 1, demarcation smallint default 0,  advmgmt smallint default 30, warning smallint default 10, rentaltype smallint default 0, rentalbooktype smallint default 0, chargetype smallint default 0, mintype smallint default 0, mintime integer default 0 , charge float(2) default 0.0 );

create table resourcecharge ( rid integer, timetype smallint ,  mintime integer, charge float(2) , foreign key (rid) references resource(rid) );

create table resourceinfo ( rid integer,  iname varchar(40), ivalue varchar(40) , foreign key (rid) references resource(rid) );

create table resourceavailability( rid integer, ocid integer ,  primary key (rid, ocid) , foreign key (ocid) references occurrence(ocid), foreign key (rid) references resource(rid)  );

create table service ( sid integer primary key autoincrement, sname varchar (40), sdesc varchar (40), spack smallint, spovr smallint, stype smallint, periodicity smallint, paymentcycle smallint, importance smallint, visibility smallint, delivtype smallint,  advbook smallint, autoaccept smallint, stime integer, scharge float(2));

create table serviceinfo ( sid integer ,  iname varchar(40), ivalue varchar(40), foreign key (sid) references service(sid) , primary key ( sid, iname));

create table servicereqinfo ( sid integer,  did smallint , mand smallint , foreign key (sid) references service(sid), foreign key (did) references datatype(did), primary key( sid, did) );

create table servicedepend ( sid integer , rid integer , primary key (sid, rid), foreign key (sid) references service(sid), foreign key (rid) references resource(rid));

create table servicepack ( sid integer , spid integer , primary key ( sid, spid) , foreign key (spid) references service(sid), foreign key ( sid) references service (sid));

create table orderstate ( osid integer primary key autoincrement, sname varchar(40 ) );

insert into  orderstate values ( 0, 'New' );
insert into  orderstate values ( 1, 'User Modified' );
insert into  orderstate values ( 2, 'User Accepted Changes' );
insert into  orderstate values ( 3, 'User Cancelled' );
insert into  orderstate values ( 4, 'Editing' );
insert into  orderstate values ( 5, 'Establishment Modified' );
insert into  orderstate values ( 6, 'Received/Viewing' );
insert into  orderstate values ( 7, 'Verified' );
insert into  orderstate values ( 8, 'Processing/Packaging' );
insert into  orderstate values ( 9, 'Delivering/Pickup Ready' );
insert into  orderstate values ( 10, 'Delivered/Completed' );
insert into  orderstate values ( 11, 'Establishment Cancelled' );

create table ezorder ( oid integer primary key autoincrement, otype smallint, cid integer, eoid varchar(20), orderts timestamp, sts timestamp, rts timestamp, state integer, optid integer, total float(2), gtotal float(2), foreign key (cid) references customer(cid), foreign key( state) references orderstate (osid) );

create table orderentry ( oid integer, lid integer , ets timestamp, desc varchar(20), sid integer, rid integer, rtype smallint, rtid integer, stime integer, etime integer, quantity float(2), unitprice float(2), subtotal float(2), foreign key (oid) references ezorder(oid) , foreign key( sid) references service(sid) , foreign key (rid) references resource(rid), primary key (oid, lid) ); 

create table orderinfo( oid integer , did smallint , dvalue varchar (40), foreign key (oid) references ezorder(oid) ,foreign key (did) references datatype(did) );

create table products     ( rid integer, pid integer , actualpid varchar(20) default '', pname varchar(20) default '' , pdesc varchar(20) default '', pmake smallint default 1, ptype smallint default 0, pgrp smallint default 0, pdisplay smallint default 1, quantity float(2) default 0.0, sellprice float(2) default 0.0, costprice float(2) default 0.0, mfgdate date , expdate date , unitq float(2) default 1.0,  defaultq float(2) default 0.0, minq float(2) default 0.0, maxq float(2) default 0.0,  foreign key(rid) references resource(rid), primary key (pid,rid));

create table productgroup ( rid integer, pgid integer, pid integer, pgtype smallint, primary key ( rid, pgid, pid ), foreign key ( rid) references resource( rid) , foreign key ( pgid ) references products ( pid ) , foreign key ( pid ) references products (pid) );

create table productsrestock   (  rid integer,  pid integer, rdate date, quantity float(2) default 0.0, sellprice float(2) default 0.0, costprice float(2) default 0.0, mfgdate date, expdate date, foreign key(rid) references resource(rid), foreign key(pid) references products(pid), primary key (rid, pid, rdate));

create table space  (  rid integer, sid integer , sname varchar(30) default '', sdesc varchar (40) default '', stype smallint default 0, sgrp smallint default 0, sdisplay smallint default 1, scount smallint default 1, smintime integer default 5, smaxtime integer default 5, scharge float(2) default 0.0, ltype smallint default 0, lxvalue integer default 0, lyvalue integer default 0, width integer default 20, height integer default 20, foreign key (rid) references resource(rid), primary key (sid,rid) );

create table spacegroup ( rid integer, sgid integer , sid integer, sgtype smallint, primary key ( rid, sgid, sid ),  foreign key ( rid) references resource( rid) , foreign key (sgid) references space(sid), foreign key ( sid) references space(sid) );

create table spaceavailability( rid integer,  sid integer, ocid integer , primary key (rid, ocid) , foreign key (ocid) references occurrence(ocid) , foreign key (sid) references space(sid), foreign key (rid) references resource(rid) );

create table spacebook ( rid integer,  sid integer, sdate date, tid integer, oid integer, stime integer , etime integer , ttype smallint, foreign key (rid) references resource(rid) ,foreign key (sid) references spaces(sid), foreign key (oid) references ezorder(oid), primary key(rid, sid, sdate, tid) );

create table employee ( rid integer, eid integer , ename varchar(30) default '', edesc varchar (40) default '' , etype smallint default 0 ,  egrp smallint default 0, edisplay smallint default 1,  foreign key (rid) references resource(rid), primary key (eid,rid) ); 

create table empgroup ( rid integer,  egid integer , eid integer, egtype smallint, primary key ( rid, egid, eid ),  foreign key (egid) references emploee(eid),  foreign key (eid) references emploee(eid) );

create table empavailability( rid integer,  eid integer, ocid integer , primary key (rid, eid, ocid) ,foreign key (rid) references resource(rid) ,  foreign key (ocid) references occurrence(ocid) , foreign key (eid) references employee(eid));

create table empbook( rid integer, eid integer, edate date, tid integer, oid integer, stime integer , etime integer , ttype smallint, foreign key (rid) references resource(rid) , foreign key(oid) references ezorder(oid), foreign key (eid) references employee(eid),  primary key (rid, eid, edate, tid) );

create table stdvalue  ( stdid integer, stdtype smallint,  svalue varchar(100 ) );

create table condition ( cid integer primary key autoincrement, condlbl  varchar(10), condflags varchar(4), operation smallint  ,  value1 float(2) , value2 float(2), std1 integer, std2 integer, cond1 integer , cond2 integer, foreign key (cond1) references condition(cid), foreign key (cond2) references condition(cid));

create table action      ( aid integer primary key autoincrement, stdvalue varchar(100), optype smallint, modtype smallint, valtype smallint, value float(2), stdid integer , foreign key (stdid) references stdvalue(stdid));

create table actiongroup ( agid integer primary key , aid integer, priority smallint, foreign key (aid) references action (aid) );

create table trigger      ( trgid integer primary key autoincrement, ocid integer, name varchar(40), description varchar(100), state smallint, event smallint, condition varchar(40), action varchar(40) );

create table bill ( billid integer primary key autoincrement, oid integer, cid integer, btime timestamp, bstate smallint, paymode smallint, cardnum varchar(16), gtotal float(2) );

create table billentry ( billid integer, bentryid integer, info varchar(50), quantity float(2), ppu float(2), subtotal float(2) , foreign key (billid) references bill(billid), primary key (billid, bentryid) );

create table billlayout ( billobjid integer, xval integer, yval integer , width integer, height integer );


