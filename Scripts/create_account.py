#!/usr/bin/python3
from api.cline import Cline
import sys
import urllib3

# Disable InsecureRequestWarning
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)


cli = Cline("https://mak.blockchain-servers.world")

#print(cli.get_info())

accountname = sys.argv[1]
cli.create_account(acct_name=accountname,creator="createacc",
                   creator_privkey="5JewWmidTzqg5zaDVU5tjFSj3ZaiZ389aGUyvMJfaFLVfK26gni",
                   owner_key="INE5BJra8uDD8ZMjgPN5VH2jwrNiXh2K5m9QduuGEQZFJanT2QXBc" )