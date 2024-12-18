#!/bin/bash

#curl -d "username=admin&password=adminpassword" -X POST http://localhost:15000/login.xyz 
#curl -vvv -d "gid=1&pid=1&qid=1&username=admin&password=adminpassword" -X POST http://localhost:15000/login.xyz
SID=`curl -vvv -d "gid=1&pid=1&qid=1&username=admin&password=adminpassword" -X POST http://localhost:15000/login.xyz 2>&1 | grep "Set-Cookie" | awk -F "=" '{ print $2 }' | awk -F ";" '{ print $1 }'`
echo "Session ID = $SID, downloading admin page"
curl -H "Cookie: SID=$SID" -vvv http://localhost:15000/admin.html
echo "Accessing multipart backend script"
curl -H "Cookie: SID=$SID" -vvv --form "name=test1" --form "name2=test2" --form "name3=test3" --form "data=@testloopdown.sh;type=text/plain"  --form "data=@testloop3types.sh;type=text/plain"  \
--form "data=@testloop3typesv6.sh;type=text/plain" \
--form "data=@testloopdown.sh;type=text/plain" \
--form "data=@testloopplug.sh;type=text/plain" \
--form "data=@testloopplugup.sh;type=text/plain" \
--form "data=@testloopsmallreq.sh;type=text/plain" \
--form "data=@testloopsmallreqv6.sh;type=text/plain" \
   http://localhost:15000/bookrest.xyz
