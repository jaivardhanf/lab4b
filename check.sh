#!/bin/bash
./lab4b --log=log.txt --scale=F --period=5 <<-EOF
OFF
EOF

if [ $? -ne 0 ]
then
	echo "Error: Exited incorrectly"
else
	echo "No error"
fi

grep "OFF" log.txt &> /dev/null; \
if [ $? -ne 0]
then
	echo "Command not logged"
else
	echo "Command successfully logged"
fi

grep "SHUTDOWN" log.txt &> /dev/null; \
if [ $? -ne 0]
then 
	echo "Shutdown not logged"
else
	echo "Shutdown was successfully logged and conducted"
fi

rm -f log.txt
