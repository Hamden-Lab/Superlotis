from opcua import Client, ua
import time

class PfeifferOPCUA(object):
    """
    Class to connect to the Pfeiffer Vacuum OPC UA Server and read and write ststuses.
    """

    def __init__(self, ip_address="192.168.1.100", port=4840):
        self.ip_address = ip_address
        self.port = port
        self.url = f"opc.tcp://{self.ip_address}:{self.port}"
        self.client = Client(self.url))

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