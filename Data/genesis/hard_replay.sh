#!/bin/bash
DATADIR="/home/petar/local_chain/Data/genesis/blockchain"

if [ ! -d $DATADIR ]; then
  mkdir -p $DATADIR;
fi

nodine --genesis-json "/home/petar/local_chain/Data/genesis/genesis.json" --plugin inery::master_plugin \
--plugin inery::master_api_plugin --plugin inery::chain_plugin \
--plugin inery::chain_api_plugin --plugin inery::http_plugin \
--plugin inery::history_api_plugin --plugin inery::history_plugin \
--plugin inery::net_plugin --plugin inery::net_api_plugin \
--filter-on=* --data-dir $DATADIR"/data" \
--blocks-dir $DATADIR"/blocks" --config-dir $DATADIR"/config" \
--access-control-allow-origin=* --contracts-console \
--http-validate-host=false --verbose-http-errors \
--enable-stale-production --p2p-max-nodes-per-host 100 \
--connection-cleanup-period 10 --hard-replay-blockchain  \
--master-name inery --http-server-address 0.0.0.0:8888 \
--https-server-address 0.0.0.0:443 \
--https-certificate="/etc/letsencrypt/live/bis.blockchain-servers.world/fullchain.pem" \
--https-private-key="/etc/letsencrypt/live/bis.blockchain-servers.world/privkey.pem" \
--p2p-listen-endpoint bis.blockchain-servers.world:9010 \
--signature-provider INE7x5HUXLfGgVs6g9bN6TSUftVJDmhFAAEo36SSnnSZoBqaGNxKx=KEY:5JgJRuwjkgcbmxN4S6eDoo6xfYZjLuQC6sgMbXFQFnNFMs9junJ \
>> $DATADIR"/nodine.log" 2>&1 & \
echo $! > $DATADIR"/ined.pid"