#!/bin/bash

# List of objects
plugins=(
  '{"id":1,"required":0,"plugin_name":"Encryption","plugin_desc":"When active this plugin makes all entry data pushed to blockchain to be encrypted with private key"}'
  '{"id":2,"required":0,"plugin_name":"Backup","plugin_desc":"When active this plugin can be used from setings to create local json copy of desired datasets"}'
  '{"id":3,"required":0,"plugin_name":"Replica sync","plugin_desc":"When active this plugin execute database operation on local chain and resync with main network once gui gets online back"}'
  '{"id":4,"required":0,"plugin_name":"MongoDB proxy","plugin_desc":"When active MongoDB plugin enable mongo query, CLI, backup on inery datasets"}'
  '{"id":5,"required":1,"plugin_name":"Export data","plugin_desc":"Builtin plugin for exporting selected data into desired format"}'
  '{"id":7,"required":1,"plugin_name":"Hot Key","plugin_desc":"When active plugin enables private keys to be saved in browser cache for session to enforce fast interagation with data"}'
  '{"id":8,"required":0,"plugin_name":"Redis Import","plugin_desc":"The IneryDB Redis import plugin facilitates the smooth transfer of data from Redis to Inery blockchain."}'
  '{"id":9,"required":0,"plugin_name":"Explorer Lite","plugin_desc":"Widget for Dashboard page for monitoring blockchain network state with block, transaction and action details query"}'
  '{"id":10,"required":0,"plugin_name":"Mongo Import","plugin_desc":"The IneryDB Mongo import plugin facilitates the smooth transfer of data from Mongo database to Inery blockchain."}'
)

# Loop through each object and execute the command
for plugin in "${plugins[@]}"; do
  id=$(echo "$plugin" | jq -r '.id')
  required=$(echo "$plugin" | jq -r '.required')
  plugin_name=$(echo "$plugin" | jq -r '.plugin_name')
  plugin_desc=$(echo "$plugin" | jq -r '.plugin_desc')

  cline push action inerygui addpluging "[$id, \"$plugin_name\", \"$plugin_desc\", $required]" -p inerygui
done
