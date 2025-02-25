import json
import argparse
import subprocess
import time
import shutil
import os.path
from sonic_py_common.general import check_output_pipe

CONFIGURED_PORTS_PATH = "/etc/mloop_conf/"
CONFIGURED_PORTS_FILE = "mloop_ports.json"
SAISDKDUMP_PATH = "/saisdkdump_file"
SERVICE_FILE = "persistent_mloop.conf"
SERVICE_PATH = "/etc/supervisor/conf.d/"

def port_sorting(port):
    """
    For translation dict sorting. Returns the port number in port name.
    """
    return int(port[0][len("Ethernet"):]) if "Ethernet" in port[0] else 0

class MloopConfig:
    def __init__(self):
        self.ports = []
        self.build_translation_dict()

    def config_ports(self, ports=None):
        configured_ports = []

        if ports:
            self.ports = ports

        # The first time doesn't work for some reason
        first_port = self.port_translation.get(self.ports[0])

        if first_port:
            self.config_port_to_mloop(first_port)

        for port in self.ports:
            logical_port = self.port_translation.get(port)

            if not logical_port:
                print(f"{port} doesn't exist")
                continue

            self.config_port_to_mloop(logical_port)
            configured_ports.append(port)

    def config_range(self, port_range):
        logical_ports = self.parse_range(port_range)

        if not logical_ports:
            print("Invalid port range")
            return
        
        for logical_port in logical_ports:
            self.config_port_to_mloop(logical_port)

        self.save_mloop_ports()
    
    def build_translation_dict(self):
        self.port_translation = {}

        subprocess.run(["saisdkdump", "-f", SAISDKDUMP_PATH], shell=False, stdout=subprocess.PIPE, text=True)

        with open(SAISDKDUMP_PATH, 'r') as saisdk_file:
            dump_lines = saisdk_file.read()

        start_index = dump_lines.find("netdev_dump")
        end_index = dump_lines.find("cmd_ifc_dump")

        port_table = dump_lines[start_index:end_index]
        port_table = port_table.split('\n')
        port_table = port_table[4:]

        for line in port_table:
            line = line.split()
            self.port_translation[line[1]] = line[2]

        self.port_translation = dict(sorted(self.port_translation.items(), key=port_sorting))
        
    def save_mloop_ports(self):
        os.makedirs(CONFIGURED_PORTS_PATH, exist_ok=True)
        full_path = os.path.join(CONFIGURED_PORTS_PATH, CONFIGURED_PORTS_FILE)
        with open(full_path, 'w') as ports_file:
            json.dump(self.ports, ports_file)

    def config_port_to_mloop(self, logical_port):
        configured = False
        retries = 0    
        subprocess.run(["sx_api_port_phys_loopback.py", "--cmd", "0", "--log_port", logical_port, "--loopback_type", "2", "--force" ], 
                        shell=False, stdout=subprocess.PIPE, text=True)
        while (not configured) and (retries < 10):
            try:
                check_output_pipe(["echo", "y"], ["sx_api_port_tx_signal_set.py", "--log_port", logical_port, "--state", "up"])      
                configured = True
            except:
                retries += 1
                if retries < 10:
                    print("Retrying to config {0}".format(logical_port))
                else:
                    print("Failed to config {0}".format(logical_port)
                time.sleep(10)
                continue

    def parse_range(self, port_range):
        found_first = False
        found_last = False
        logical_ports = []

        for port, logical_port in self.port_translation.items():
            if port == port_range[0]:
                found_first = True

            if port == port_range[1]:
                found_last = True

            if found_first and not found_last:
                logical_ports.append(logical_port)
                self.ports.append(port)
            elif found_last and not found_first:
                print("Error: invalid range - end port found before start port")

            if found_last:
                break

        if found_first and not found_last:
            print(f"Error: invalid range - {port_range[0]} doesn't exist")
            return None

        return logical_ports if logical_ports else None

    def read_saved_ports(self):
        full_path = os.path.join(CONFIGURED_PORTS_PATH, CONFIGURED_PORTS_FILE)
        with open(full_path, 'r') as ports_file:
            self.ports = json.load(ports_file)
        

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ports", nargs="+", type=str, help="List of ports to be configured to mloop, in the following format: port1 port2 ...")
    parser.add_argument("--port-range", nargs=2, help="Range of ports to be configured to mloop, in the following format: <start_port> <end_port>")
    args = parser.parse_args()

    if args.ports and args.port_range:
        print("Error: must specify only one of the options - PORTS or PORT-RANGE")
        return 

    service_file_path = os.path.join(SERVICE_PATH, SERVICE_FILE)
    if not os.path.exists(service_file_path):
        current_dir_file_path = os.path.join(os.getcwd(), SERVICE_FILE)
        if not os.path.exists(current_dir_file_path):
            print(f"Error: {SERVICE_FILE} is not found in {SERVICE_PATH} or the current directory.")
            return
    
        try:
            shutil.copy(current_dir_file_path, SERVICE_PATH)
        except Exception as e:
            print(f"Error while copying {SERVICE_FILE} to {SERVICE_PATH}: {e}")

    mloopconfig = MloopConfig()

    if args.port_range:
        mloopconfig.config_range(args.port_range)

    elif args.ports:
        mloopconfig.config_ports(args.ports)
        mloopconfig.save_mloop_ports()
    else:
        mloopconfig.read_saved_ports()
        mloopconfig.config_ports()


if __name__ == "__main__":
    main()
