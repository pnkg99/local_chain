#!/bin/bash
DATADIR="/home/petar/local_chain/Data/genesis/blockchain"

if [ -f $DATADIR"/ined.pid" ]; then
pid=`cat $DATADIR"/ined.pid"`
kill $pid
rm -r $DATADIR"/ined.pid"
while [ -d "/proc/$pid/fd" ]; do sleep 1; done
echo "Node Stopped."
fi