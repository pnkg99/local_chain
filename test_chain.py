#!/usr/bin/python3
import subprocess
import time
import json
import os
import sys
import shutil

def get_ubuntu_version():
    try:
        result = subprocess.run(['lsb_release', '-a'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if result.returncode == 0:
            output = result.stdout
            lines = output.split('\n')
            for line in lines:
                if 'Release:' in line:
                    version = line.split(':')[1].strip()
                    major_version = int(version.split('.')[0])
                    if  18 <= major_version <= 24:
                        print(f"Ubuntu version : {version}")
                        return 
                    else:
                        print("Ubuntu version must be  ")
                        exit()
        else:
            print("Failed to obtain Ubuntu version. Make sure lsb_release is installed.")
            exit() 

    except Exception as e:
        print(f"An error occurred: {str(e)}")
        exit()


class TestChain:
    def __init__(self) -> None:
        self.path = os.getcwd()
        self.config_path = os.path.join(self.path, "Config")
        self.gen_config = self.read_file(os.path.join(self.config_path, "general.json"), True)
        self.dev_keys = self.gen_config["dev_keys"]
        self.server_url = self.gen_config["server_url"]
        
        self.data_path = os.path.join(self.path, "Data")
        self.genesis_path = os.path.join(self.data_path, "genesis")
        
        self.masters = self.read_file(os.path.join(self.config_path, "masters.json"), True)
        self.masters_path = os.path.join(self.data_path, "masters")
        self.max_masters = self.gen_config["max_masters"]
        
        self.genesis_script_path = {
            "genesis_json": os.path.join(self.genesis_path, "genesis.json"),
            "genesis_start": os.path.join(self.genesis_path, "genesis_start.sh"),
            "hard_replay": os.path.join(self.genesis_path, "hard_replay.sh"),
            "stop": os.path.join(self.genesis_path, "stop.sh"),
            "blockchain": os.path.join(self.genesis_path, "blockchain")
        }
        self.genesis_json = {
            "initial_timestamp": "2024-07-15T00:00:00.000",
            "initial_key": self.dev_keys["PUBLIC_KEY"],
            "initial_configuration": {
                "max_block_net_usage": 1048576, "target_block_net_usage_pct": 1000,
                "max_transaction_net_usage": 524288, "base_per_transaction_net_usage": 12,
                "net_usage_leeway": 500, "context_free_discount_net_usage_num": 20,
                "context_free_discount_net_usage_den": 100, "max_block_cpu_usage": 100000,
                "target_block_cpu_usage_pct": 500, "max_transaction_cpu_usage": 50000,
                "min_transaction_cpu_usage": 100, "max_transaction_lifetime": 3600,
                "deferred_trx_expiration_window": 600, "max_transaction_delay": 3888000,
                "max_inline_action_size": 4096, "max_inline_action_depth": 4, "max_authority_depth": 6
            },
            "initial_chain_id": "0000000000000000000000000000000000000000000000000000000000000000"
        }  
        self.wallet_psw = self.read_file(os.path.join(self.config_path, "defWall.txt"))

        self.port = 9009
        
        self.system_accounts = self.read_file(os.path.join(self.config_path, "system_accounts.json"), True)
        self.contract_path = os.path.join(self.config_path, "contracts")
        self.boot_path = os.path.join(self.contract_path, "inery.boot")
        self.system_path = os.path.join(self.contract_path, "inery.system")
        self.token_path = os.path.join(self.contract_path, "inery.token")

        self.features = ["825ee6288fb1373eab1b5187ec2f04f6eacb39cb3a97f356a07c91622dd61d16",
                        "c3a6138c5061cf291310887c0b5c71fcaffeab90d5deb50d3b9e687cead45071",
                        "bf61537fd21c61a60e542a5d66c3f6a78da0589336868307f94a82bccea84e88",
                        "5443fcf88330c586bc0e5f3dee10e7f63c76c00249c87fe4fbf7f38c082006b4",
                        "f0af56d2c5a48d60a4a5b5c903edfb7db3a736a94ed589d0b797df33ff9d3e1d",
                        "2652f5f96006294109b3dd0bbde63693f55324af452b799ee137a81a905eed25",
                        "8ba52fe7a3956c5cd3a656a3174b931d3bb2abb45578befc59f283ecd816a405",
                        "ad9e3d8f650687709fd68f4b90b41f7d825a365b02c23a636cef88ac2ac00c43",
                        "68dcaa34c0517d19666e6b33add67351d8c5f69e999ca1e37931bc410a297428",
                        "e0fb64b1085cc5538970158d05a009c24e276fb94e1a0bf6a528b48fbc4ff526",
                        "ef43112c6543b88db2283a2e077278c315ae2c84719a8b25f25cc88565fbea99",
                        "4a90c00d55454dc5b059055ca213579c6ea856967712a56017487886a4d4cc0f",
                        "1a99a59d87e06e09ec5b028a9cbb7749b4a5ad8819004365d02dc4379a8b7241",
                        "4e7bf348da00a945489b2a681749eb56f5de00b900014e137ddae39f48f69d67",
                        "4fca8bd82bbd181e714e283f83e1b45d95ca5af40fb89ad3977b653c448f78c2",
                        "299dcb6af692324b899b39f16d5a530a33062804e41f09dc97e9f156b4476707"]

        self.home_path = os.getenv("HOME")
        self.bashrc_path = os.path.join(self.home_path, '.bashrc')
        self.wallet_path = os.path.join(self.home_path, "inery-wallet")
        self.default_wallet_path = os.path.join(self.wallet_path, "default.wallet")
        self.inery_bin_path = os.path.join(self.path, "Packages", "inery", "bin")
        self.inery_nodine_path = "nodine"
        self.inery_cline_path =  "cline"
        self.inery_kined_path =  "kined"
        self.inery_cdt_path = os.path.join(self.path, "Packages", "inery.cdt")
        
    def check_nodine(self):
        binary = "nodine"
        path = shutil.which(binary)
        if path:
            print(f"{binary} is available at {path}.")
        else: 
            print(f"No global {binary} binaries found.")
            # self.inery_nodine_path = os.path.join(self.inery_bin_path, "nodine")
            # self.inery_cline_path = os.path.join(self.inery_bin_path, "cline")
            
            with open(self.bashrc_path, 'r') as bashrc:
                bashrc_content = bashrc.read()

            export_path = f'export PATH="$PATH:{self.inery_bin_path}"'
            os.environ['PATH'] += f":{self.inery_bin_path}"
            print(f"Path updated. Inery binary PATH: {self.inery_bin_path}")
            if export_path in bashrc_content:
                pass
            else:
                print(f"Adding {self.inery_bin_path} to {self.bashrc_path}...")
                with open(self.bashrc_path, 'a') as bashrc:
                    bashrc.write(f'\n{export_path}')
                
        nodine_v = subprocess.Popen([binary, "--version"], stdout=subprocess.PIPE).communicate()[0].decode()
        print(f"{binary} version : ", nodine_v)


    def _get_port(self) :
        self.port +=1
        return self.port

    def read_file(self, file_path, json_load=False):
        with open(file_path, "r") as file:
            return json.loads(file.read()) if json_load else file.read().strip()

    def write_file(self, file_path, data, json_dump=False, mode="w+"):
        with open(file_path, mode) as file:
            json.dump(data, file, indent=6) if json_dump else file.write(data)

    # Wallet Actions
    
    def check_default_wallet(self) :
        s = subprocess.Popen([self.inery_cline_path, "wallet", "list"], stdout=subprocess.PIPE).communicate()[0].decode()
        first_open = s.find("[")
        last_close = s.rfind("]")
        if first_open != -1 and last_close != -1 and first_open < last_close:
            try :
                s = s[first_open + 1:last_close-1]
                wallets = s.replace('"', "").replace(",", "").split()
                if "default *" in wallets :
                    print("A")
                    exit()
                    return
                elif "default" in wallets :                  
                    self.unlock_wallet()
                else :
                    self.create_wallet()
            except Exception as e :
                print(e)
                return False
            
            
    def create_wallet(self):
        try:
            if os.path.exists(self.default_wallet_path):
                os.remove(self.default_wallet_path)
            subprocess.Popen([self.inery_cline_path, "wallet", "create", "--file", f"{self.config_path}/defWall.txt"], stdout=subprocess.PIPE).communicate()[0]
            self.import_keys()
        except Exception as error:
            print('An error has occured while creating a wallet! \n')
            print(error)

    def unlock_wallet(self):
        try:
            # Start the subprocess
            process = subprocess.Popen(
                [self.inery_cline_path, "wallet", "unlock"],
                stdin=subprocess.PIPE,  # Enable writing input to the process
                stdout=subprocess.PIPE,  # Capture output (optional)
                stderr=subprocess.PIPE,  # Capture errors (optional)
                text=True               # Handle input/output as text
            )

            # Send the password to the process
            output, error = process.communicate(input=self.wallet_psw + '\n', timeout=15)

            # Check for errors
            if process.returncode != 0:
                if "Already unlocked" in error:
                    print("Default wallet is already unlocked.")
                    return
                elif "Invalid wallet password" in error :
                    print("Wallet password is not correct! ")
                    x = input("Want to restart wallet? Press [y/Y]")
                    if x in ["y", "Y", "yes", "Yes", "YES"] :
                        self.create_wallet()
            else:
                print("Default wallet unlocked.")
        except subprocess.TimeoutExpired:
            print("The unlock process timed out.")
        except Exception as e:
            print(f"An error occurred: {e}")
    
    def import_key(self, private_key):
        subprocess.run([self.inery_cline_path, "wallet", "import", "--private-key", private_key])
        
    def import_keys(self) :
        self.import_key(self.dev_keys["PRIVATE_KEY"])
        for account in self.system_accounts :
            self.import_key(account["PRIVATE_KEY"])
        for master in self.masters :
            self.import_key(master["PRIVATE_KEY"])
            
    # Blockchain Account 
    
    def create_account(self, account, ownerkey="", activekey="",creator="creator"):
        if not ownerkey :
            ownerkey = self.dev_keys["PUBLIC_KEY"]
        if not activekey :
            activekey = ownerkey
        subprocess.Popen([self.inery_cline_path, "system", "opendb", creator, account, ownerkey, activekey]).communicate()[0]

    
    ## Main Processes
    
    def run_chain(self) :
        self.check_default_wallet()
        self.run_genesis()
        self.run_masters()

    def run_genesis(self) -> None:
        
        os.makedirs(self.genesis_path, exist_ok=True)
        port = self._get_port()
        self.write_file(self.genesis_script_path["genesis_json"], self.genesis_json, True)
        
        self.write_file(self.genesis_script_path["genesis_start"], self.genesis_script(self.genesis_path,  self.dev_keys, port, genesis=True), mode="w+")
        self.write_file(self.genesis_script_path["hard_replay"], self.hard_replay_script(self.genesis_path,  self.dev_keys, port, genesis=True), mode="w+")
        self.write_file(self.genesis_script_path["stop"], self.stop_script(self.genesis_path), mode="w+")
        
        os.chmod(self.genesis_script_path["genesis_start"], 0o777)
        os.chmod(self.genesis_script_path["hard_replay"], 0o777)
        os.chmod(self.genesis_script_path["stop"], 0o777)
        
        os.system(self.genesis_script_path["genesis_start"])
        
        print("Blockchain started")
        time.sleep(1)
        try:

            os.popen('curl --request POST --url http://127.0.0.1:8888/v1/master/schedule_protocol_feature_activations -d \'{"protocol_features_to_activate":["0ec7e080177b2c02b278d5088611686b49d739925a92d9bfcacd7fc6b74053bd"]}\'').read()
            i=0
            while i<20  : 
                result = subprocess.Popen([self.inery_cline_path, "set", "contract", "inery", f"{self.boot_path}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
                error = str(result[1])
                if "transaction executed locally" in error or "the new code is the same as the existing" in error :
                    break
                if error:
                    print(error)
                    i+=1
                    continue
                else:
                    break

            for account in self.system_accounts :
                account_name = account["NAME"]
                public_key = account["PUBLIC_KEY"]
                subprocess.run([self.inery_cline_path, "create", "opendb", "inery", f"{account_name}", f"{public_key}"], stdout=subprocess.PIPE)


            for feature in self.features:
                subprocess.Popen([self.inery_cline_path, "push", "action", "inery", "activate" , f"[\"{feature}\"]", "-p", "inery"]).communicate()[0]
            
            while True  : 
                result = subprocess.Popen([self.inery_cline_path, "set", "contract", "inery", f"{self.system_path}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
                error = str(result[1])
                if "transaction executed locally" in error or "the new code is the same as the existing" in error :
                    break
                if error:
                    print(error)
                    continue
                else:
                    break

            subprocess.Popen([self.inery_cline_path, "set", "contract", "inery.token", f"{self.token_path}"]).communicate()[0]
            # 268435456000 INR = 256GB u bajtovima 
            subprocess.Popen([self.inery_cline_path, "push", "action", "inery.token", "create", f"[\"inery\", \"268435456000.0000 INR\"]", "-p", "inery.token@active"]).communicate()[0]
            ## Issue 128GB in INR to inery account
            subprocess.Popen([self.inery_cline_path, "push", "action", "inery.token", "issue", f"[\"inery\", \"134217728000.0000 INR\", \"Issuing tokens for inery account\"]", "-p", "inery@active" ]).communicate()[0]
            subprocess.Popen([self.inery_cline_path, "push", "action", "inery", "init", f"[\"0\", \"4,INR\"]", "-p", "inery@active"]).communicate()[0]
            print("Contracts initilized")
            ## Transfer 64GB in INR to creator account
            subprocess.Popen([self.inery_cline_path, "system", "opendb", "inery", "creator", self.dev_keys["PUBLIC_KEY"]]).communicate()[0]
            subprocess.Popen([self.inery_cline_path, "transfer", "inery", "creator", "67108864000 INR"]).communicate()[0]
            time.sleep(1)
            
        except Exception as error:
            print('An error has occured while initializatig contracts! \n')
            print(error)

    def run_masters(self) :
        master_cnt = 0
        for master in self.masters :
            if master_cnt >= self.max_masters:
                break
            try:
                self.process_master(master)
                master_cnt += 1
                time.sleep(0.2)
            except Exception as ex:
                print(ex)
        self.approve_masters()
                
    def approve_masters(self) :
        port = 9010
        master_cnt = 0
        for master in self.masters :
            if master_cnt >= self.max_masters:
                break
            try:
                time.sleep(0.5)
                master_cnt+=1
                master_name = master["NAME"]
                subprocess.run([self.inery_cline_path, "transfer", "inery", master_name, "50000 INR"])
                subprocess.run([self.inery_cline_path, "system", "stake", "inery", master_name , "50000 INR"])
                subprocess.run([self.inery_cline_path, "master", "regmaster", master_name , master["PUBLIC_KEY"], f"{self.server_url}{port}"])
                port+=1
            except Exception as ex:
                print(ex)

    def process_master(self, master):
        master_name, master_public = master["NAME"], master["PUBLIC_KEY"]
        self.create_account(master_name,master_public )
        
        dir_path = os.path.join(self.masters_path, "master_" + master_name)
        os.makedirs(dir_path, exist_ok=True)
        
        genstart_path = os.path.join(dir_path, "genesis_start.sh")
        hardreplay_path = os.path.join(dir_path, "hard_replay.sh")
        stop_path = os.path.join(dir_path, "stop.sh")
           
        port = self._get_port()
        print(f"Starting master {master_name} at port : {port} ")
        self.write_file(genstart_path, self.genesis_script(dir_path,  master, port, peers=True, master=master_name), mode="w+")
        self.write_file(hardreplay_path, self.hard_replay_script(dir_path,  master, port, peers=True, master=master_name), mode="w+")
        self.write_file(stop_path, self.stop_script(dir_path), mode="w+")      
        
        os.chmod(genstart_path, 0o777)
        os.chmod(hardreplay_path, 0o777)
        os.chmod(stop_path, 0o777)
	
        os.system(genstart_path)
        print(f"Started master {master_name} at port : {port} ")
        time.sleep(0.2)

    def stop_chain(self) -> None:
        
        nodine_pids = subprocess.Popen(["pidof", "nodine"], stdout=subprocess.PIPE).communicate()[0].decode()
        for pid in nodine_pids.split():
            os.system(f"kill -9 {pid}")
            print(f"Killed process pid={pid}")
            time.sleep(0.2)   
        if os.path.exists(self.genesis_path):
            shutil.rmtree(self.genesis_path)
        if os.path.exists(self.masters_path):
            shutil.rmtree(self.masters_path)


    ## SH Scripts

    def add_peer_addresses(self, script, port):
        peer_lines = ''
        for peer in range(9010, port):
            peer_lines += f'--p2p-peer-address {self.server_url}:{peer} \\\n'
        return script.replace(
            f'--p2p-listen-endpoint {self.server_url}:{port} \\\n',
            f'--p2p-listen-endpoint {self.server_url}:{port} \\\n{peer_lines}'
        )
    
    def add_https_configuration(self, script):
        https_lines = (
            f'--https-server-address 0.0.0.0:443 \\\n'
            f'--https-certificate="/etc/letsencrypt/live/{self.server_url}/fullchain.pem" \\\n'
            f'--https-private-key="/etc/letsencrypt/live/{self.server_url}/privkey.pem" \\\n'
        )
        return script.replace(
            f'--p2p-listen-endpoint {self.server_url}:', 
            f'{https_lines}--p2p-listen-endpoint {self.server_url}:'
        )

    def genesis_script(self, path, keys, port, peers=False, genesis=False, master=""):
        master_name = master if master else 'inery'
        script = (
            f'#!/bin/bash\nDATADIR="{path}/blockchain"\n\nif [ ! -d $DATADIR ]; then\n  mkdir -p $DATADIR;\nfi\n\n'
            f'{self.inery_nodine_path} --genesis-json "{self.genesis_path}/genesis.json" --plugin inery::master_plugin \\\n'
            f'--plugin inery::master_api_plugin --plugin inery::chain_plugin \\\n'
            f'--plugin inery::chain_api_plugin --plugin inery::http_plugin \\\n'
            f'--plugin inery::history_api_plugin --plugin inery::history_plugin \\\n'
            f'--plugin inery::net_plugin --plugin inery::net_api_plugin \\\n'
            f'--plugin inery::trace_api_plugin \\\n' 
            f'--trace-history --trace-no-abis \\\n'
            f'--filter-on=* --data-dir $DATADIR"/data" \\\n--blocks-dir $DATADIR"/blocks" \\\n'
            f'--config-dir $DATADIR"/config" --access-control-allow-origin=* \\\n'
            f'--contracts-console --http-validate-host=false \\\n'
            f'--verbose-http-errors --enable-stale-production \\\n'
            f'--p2p-max-nodes-per-host 100 --connection-cleanup-period 10 \\\n'
            f'--master-name {master_name} --http-server-address 0.0.0.0:{port - 122} \\\n'
            f'--p2p-listen-endpoint {self.server_url}:{port} \\\n'
            f'--signature-provider {keys["PUBLIC_KEY"]}=KEY:{keys["PRIVATE_KEY"]} \\\n'
            f'>> $DATADIR"/nodine.log" 2>&1 & \\\n'
            f'echo $! > $DATADIR"/ined.pid"'
        )

        if peers:
            script = self.add_peer_addresses(script, port)
        if genesis:
            pass
            #script = self.add_https_configuration(script)

        return script
    
    def hard_replay_script(self, path, keys, port, peers= False, genesis=False, master=""):
        master_name = master if master else 'inery'
        script = (
            f'#!/bin/bash\nDATADIR="{path}/blockchain"\n\nif [ ! -d $DATADIR ]; then\n  mkdir -p $DATADIR;\nfi\n\n'
            f'{self.inery_nodine_path} --genesis-json "{self.genesis_path}/genesis.json" --plugin inery::master_plugin \\\n'
            f'--plugin inery::master_api_plugin --plugin inery::chain_plugin \\\n'
            f'--plugin inery::chain_api_plugin --plugin inery::http_plugin \\\n'
            f'--plugin inery::history_api_plugin --plugin inery::history_plugin \\\n'
            f'--plugin inery::net_plugin --plugin inery::net_api_plugin \\\n'
            f'--filter-on=* --data-dir $DATADIR"/data" \\\n'
            f'--blocks-dir $DATADIR"/blocks" --config-dir $DATADIR"/config" \\\n'
            f'--access-control-allow-origin=* --contracts-console \\\n'
            f'--http-validate-host=false --verbose-http-errors \\\n'
            f'--enable-stale-production --p2p-max-nodes-per-host 100 \\\n'
            f'--connection-cleanup-period 10 --hard-replay-blockchain  \\\n'
            f'--master-name {master_name} --http-server-address 0.0.0.0:{port - 122} \\\n'
            f'--p2p-listen-endpoint {self.server_url}:{port} \\\n'
            f'--signature-provider {keys["PUBLIC_KEY"]}=KEY:{keys["PRIVATE_KEY"]} \\\n'
            f'>> $DATADIR"/nodine.log" 2>&1 & \\\n'
            f'echo $! > $DATADIR"/ined.pid"'
        )
        if peers:
            script = self.add_peer_addresses(script, port)
        if genesis:
            script = self.add_https_configuration(script)
        return script

    def stop_script(self, path):
        return (
            f'#!/bin/bash\nDATADIR="{path}/blockchain"\n\nif [ -f $DATADIR"/ined.pid" ]; then\n'
            f'pid=`cat $DATADIR"/ined.pid"`\nkill $pid\nrm -r $DATADIR"/ined.pid"\n'
            f'while [ -d "/proc/$pid/fd" ]; do sleep 1; done\n'
            f'echo "Node Stopped."\nfi'
        )


if __name__== '__main__':
    test_chain = TestChain()
    if len(sys.argv) < 2:
        print("invalid number of arguments try --help")
        exit()

    elif sys.argv[1] == "start":
        get_ubuntu_version()
        test_chain.check_nodine()
        test_chain.run_chain()
    
    elif sys.argv[1] == "stop":
        test_chain.stop_chain()

    elif sys.argv[1] == "restart":
        test_chain.stop_chain()
        test_chain.run_chain()

    else:
        print(sys.argv[1])
        print("wrong flags try:")
        print("\tstart      - to run chain ")
        print("\tstop       - to stop chain")

