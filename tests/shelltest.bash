#!/bin/bash
termid=`curl -vvv --data-urlencode "command=START" http://localhost:15000/bash.xyz 2>&1 | grep -i "Term-Id" | awk -F" " '{print $3 }'`
echo $termid
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=START" http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=EXECUTE" --data-urlencode 'op=ls -l /tmp/' http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=EXECUTE" --data-urlencode 'op=ifconfig' http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=EXECUTE" --data-urlencode 'op=ls -l /usr/bin/' http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=EXECUTE" --data-urlencode 'op=ps -ef' http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=EXECUTE" --data-urlencode 'op=ps -ef| grep httpdsrv' http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=EXECUTE" --data-urlencode 'op=echo alldone' http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=EXECUTE" --data-urlencode 'op=i=1; while [ $i -lt 11 ]; do echo "whiling $i"; i=$((i+1)); done' http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" -H "Content-type', 'application/x-www-form-urlencoded; charset=utf-8" --data-urlencode "command=EXECUTE" --data-urlencode 'op=i=1; while [ $i -lt 11 ]; do echo "whiling $i"; i=$((i+1)); done' http://localhost:15000/bash.xyz
curl -vvv -H "Term-Id: $termid" --data-urlencode "command=STOP" http://localhost:15000/bash.xyz

echo $termid
