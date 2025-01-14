import requests, datetime

class ChainMonitor:
    def __init__(self):
        self.base_url = "http://127.0.0.1:8888/v1/chain"

    def get_blockchain_info(self):
        url = f"{self.base_url}/get_info"
        try:
            response = requests.get(url)
            response.raise_for_status()  # Raises an HTTPError if the response status code is 4XX/5XX
            return response.json()  # Returns the JSON response
        except requests.exceptions.RequestException as e:
            print(f"Error making request: {e}")
            return None

    def post_data(self, data):
        url = f"{self.base_url}/post_data"
        try:
            response = requests.post(url, json=data)
            response.raise_for_status()  # Raises an HTTPError if the response status code is 4XX/5XX
            return response.json()  # Returns the JSON response
        except requests.exceptions.RequestException as e:
            print(f"Error making request: {e}")
            return None
    
    def get_table(self, contract, scope, table):
        url = f"{self.base_url}/get_table_rows"
        data = {
            "json": True,
            "code": contract,
            "scope": scope,
            "table": table,
            "limit": 10  # Adjust the limit as needed
        }
        try:
            response = requests.post(url, json=data)
            response.raise_for_status()  # Raises an HTTPError if the response status code is 4XX/5XX
            return response.json()  # Returns the JSON response
        except requests.exceptions.RequestException as e:
            print(f"Error making request: {e}")
            return None

    def get_abi(self, account):
        url = f"{self.base_url}/get_abi"
        data = {
            "account_name": account
        }
        try:
            response = requests.post(url, json=data)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            print(f"Error making request: {e}")
            return None

    def get_table_by_scope(self, contract, table):
        url = f"{self.base_url}/get_table_by_scope"
        data = {
            "json": True,
            "code": contract,
            "table": table
        }
        try:
            response = requests.post(url, json=data)
            response.raise_for_status()  # Raises an HTTPError if the response status code is 4XX/5XX
            return response.json()  # Returns the JSON response
        except requests.exceptions.RequestException as e:
            print(f"Error making request: {e}")
            return None

    def get_contract_data_all(self, account):
        abi = self.get_abi(account)
        if abi is not None:
            tables = abi.get("abi", {}).get("tables", [])
            table_data = {}  # Create an empty dictionary to store table data
            for table in tables:
                table_name = table.get("name")
                scope = self.get_table_by_scope("inery", table_name)
                for row in scope.get("rows"):
                    result = self.get_table(row.get("code"), row.get("scope"), row.get("table"))
                    if result is not None:
                        rows = result.get("rows", [])
                        table_data[table_name] = rows  # Add rows to the table_data dictionary
                else:
                    pass
            
            return table_data 
        else:
            print(f"Failed to retrieve ABI for '{account}'")
            return None  # Return None if ABI retrieval fails
        
    def get_table_data_all(self, account, table):
        abi = self.get_abi(account)
        if abi is not None:
            table_obj = {}
            table_data = self.get_table_by_scope(account, table)
            for row in table_data.get("rows"):
                result = self.get_table(row.get("code"), row.get("scope"), row.get("table"))
                if result is not None:
                    rows = result.get("rows", [])
                    table_obj[table] = rows 
                else:
                    pass
            return table_obj[table]
        else:
            print(f"Failed to retrieve ABI for '{account}'")
            return None
        
    def show_production(self):
        masters = self.get_table_data_all("inery", "masters")
        print(f"Total number of masters : {len(masters)}")
        for master in masters:
            miliseconds = int(master.get("last_produced_block_time"))
            time_format = datetime.datetime.fromtimestamp(miliseconds / 1000)
            print(f"Account: {master.get('owner')}, Total Unpaid Blocks: {master.get('unpaid_blocks')}, Last Produced Block Time: {time_format}")

# Example usage
monitor = ChainMonitor()
monitor.show_production()
