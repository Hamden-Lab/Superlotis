from opcua import Client, ua
import time

class PfeifferOPCUA(object):
    """
    Class to connect to the Pfeiffer Vacuum OPC UA Server and read and write ststuses.
    """

    def __init__(self, ip_address="192.168.1.100", port=4840, pump_names=["TC80", "MVP"]):
        self.ip_address = ip_address
        self.port = port
        self.url = f"opc.tcp://{self.ip_address}:{self.port}"
        self.client = Client(self.url)
        self.pump_names = pump_names

    def pump_address(self, pump_name):
        """
        Convert pump name -> address index.
        """
        try:
            idx = self.pump_names.index(pump_name) + 1
            return idx

        except ValueError:
            raise ValueError(f"Unknown pump name: {pump_name}")
        
    # def pump_bool(self, pump_name):
    #     """
    #     Get boolean in case of pump specific commands.
    #     """

    def connect(self):
        """
        Connect to the OPC UA Server.
        """
        try:
            self.client.connect()
            print(f"Connected to OPC UA Server at {self.url}")
        except Exception as e:
            print(f"Failed to connect to OPC UA Server: {e}")

    def disconnect(self):
        """
        Disconnect from the OPC UA Server.
        """
        try:
            self.client.disconnect()
            print("Disconnected from OPC UA Server")
        except Exception as e:
            print(f"Failed to disconnect from OPC UA Server: {e}")

    def _set_node_value(self, node_id, value):
        """
        Set the value of a node.

        NOTE: Check reference for second definition if this one does not work.
        """
        try:
            node = self.client.get_node(node_id)
            variant = ua.Variant(value, ua.VariantType.Boolean)
            node.set_value(variant)
            print(f"Set value of node {node_id} to {value}")
        except Exception as e:
            print(f"Failed to set value of node {node_id}: {e}")

    def wait_until_running(self, timeout=60):
        """
        Wait until the pump is running or timeout.
        """
        start = time.time()
        while time.time() - start < timeout:
            status = self.get_status()
            if status == 2:
                return True
            time.sleep(1)
        return False
    
    # 001 Turbo heating
    
    def standby(self, pump_name, boolean=False):
        """
        Set the pump to standby mode.
        """
        idx = self.pump_address(pump_name)
        node_id = f"ns=1;s=P{idx}_002_Standby"
        self._set_node_value(node_id, boolean)

    # 004 Turbo Run-up time monitor mode

    def error_acknowledge(self, pump_name):
        """
        Acknowledge and clear any errors on the pump.
        """
        idx = self.pump_address(pump_name)
        node_id = f"ns=1;s=P{idx}_009_ErrorAckn"
        self._set_node_value(node_id, True)
        time.sleep(0.5)
        self._set_node_value(node_id, False)

    def pump_power(self, pump_name, boolean=False):
        """
        Start/stop the pump.
        """
        idx = self.pump_address(pump_name)
        node_id = f"ns=1;s=P{idx}_010_PumpgStatn"
        self._set_node_value(node_id, boolean)

    # 012 Turbo pump venting

    # 017 Turbo rotation speed switchpoint config

    # 019 both Output D02 configuration

    # 024 both Output D01 configuration