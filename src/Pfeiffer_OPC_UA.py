from opcua import Client, ua
import time

'''
GO BACK AND CREATE READ METHOD FOR EACH COMMAND METHOD!!!
'''


class PfeifferOPCUA(object):
    """
    Class to connect to the Pfeiffer Vacuum OPC UA Server and read and write statuses.
    """

    def __init__(self, ip_address="192.168.1.100", port=4840, pump_names=["TC80", "MVP"]):
        self.ip_address = ip_address
        self.port = port
        self.url = f"opc.tcp://{self.ip_address}:{self.port}"
        self.client = Client(self.url)
        self.pump_names = pump_names

    '''
    Commands Read/Write (except 009)
    '''

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

    # 025 Turbo backing pump mode?

    def speed_set_mode(self, pump_name, boolean=False):
        """
        Turn on and off the set speed mode of the pump.
        """
        idx = self.pump_address(pump_name)
        node_id = f"ns=1;s=P{idx}_026_SpdSetMode"
        self._set_node_value(node_id, boolean)

    # 027 Turbo gas mode

    def valve_mode(self, pump_name, mode="auto"):
        """
        Set the valve mode of the pump.
        """
        if pump_name == "MVP":
            idx = self.pump_address(pump_name)
            node_id = f"ns=1;s=P{idx}_030_ValveMode"

            if mode == "auto":
                state = 0

            elif mode == "closed":
                state = 1

            elif mode == "open":
                state = 2

            self._set_node_value(node_id, state)

        else:
            raise ValueError(f"Valve mode is only available for MVP pump, not {pump_name}.")
        
    def vent_mode(self, pump_name, mode="direct"):
        """
        Set the venting mode of the pump.
        """
        if pump_name == "TC80":
            idx = self.pump_address(pump_name)
            node_id = f"ns=1;s=P{idx}_030_VentMode"

            if mode == "delayed":
                state = 0

            elif mode == "none":
                state = 1

            elif mode == "direct":
                state = 2

            self._set_node_value(node_id, state)

        else:
            raise ValueError(f"Vent mode is only available for Turbo pump, not {pump_name}.")
        
    # 035 Turbo configure accessory connection A1

    # 036 Turbo configure accessory connection B2

    def purge_gas(self, pump_name, boolean=True):
        """
        Turn on and off the purge gas flow for an MVP.
        """
        if pump_name == "MVP":
            idx = self.pump_address(pump_name)
            node_id = f"ns=1;s=P{idx}_050_PurgeGas"
            self._set_node_value(node_id, boolean)

        else:
            raise ValueError(f"Purge gas is only available for MVP pump, not {pump_name}.")
        
    def sealing_gas(self, pump_name, boolean=False):
        """
        Turn on and off the sealing gas flow for a TC80 turbopump.
        """
        if pump_name == "TC80":
            idx = self.pump_address(pump_name)
            node_id = f"ns=1;s=P{idx}_050_SealingGas"
            self._set_node_value(node_id, boolean)

        else:
            raise ValueError(f"Sealing gas is only available for TC80 turbopump, not {pump_name}.")
        
    # 055 Turbo output AO1 configuration

    # 058 Turbo temperature management mode

    def interface(self, pump_name, mode="remote"):
        """
        Set the interface mode of the pump.
        """
        idx = self.pump_address(pump_name)
        node_id = f"ns=1;s=P{idx}_060_CtrlViaInt"
        if mode == "remote":
            state = 1

        elif mode == "RS485":
            state = 2

        elif mode == "PV.can":
            state = 4
        
        elif mode == "unlock":
            state = 255

        else:
            raise ValueError(f"Unknown interface mode: {mode}")
            return

        self._set_node_value(node_id, state)

    def interace_lock(self, pump_name, boolean=False):
        """
        Lock or unlock the pump interface.
        """
        idx = self.pump_address(pump_name)
        node_id = f"ns=1;s=P{idx}_061_IntSelLckd"
        self._set_node_value(node_id, boolean)

    # 062, 063, 068, 069 Turbo user defined configurations

    '''
    Status Read Methods
    '''

    def remote_priority(self, pump_name, p=False):
        """
        Get the remote priority status of the pump.
        """
        idx = self.pump_address(pump_name)
        node_id = f"ns=1;s=P{idx}_RemotePrio"

        if pump_name == "TC80":
            try:
                node = self.client.get_node(node_id)
                value = node.get_value()

                if p == True:
                    print(f"Remote priority status of pump {pump_name}: {value}")
                return value

            except Exception as e:
                print(f"Failed to read remote priority status of pump {pump_name}: {e}")

        else:
            raise ValueError(f"Remote priority status is only available for TC80 pump, not {pump_name}.")
        
    # 302 Turbo rotation speed switchpoint status

    def error_code(self, pump_name, p=True):
        """
        Get the error code of the pump.
        """
        idx = self.pump_address(pump_name)
        node_id = f"ns=1;s=P{idx}_ErrorCode}"
        try:
            node = self.client.get_node(node_id)
            value = node.get_value()

            if p == True:
                print(f"Error code of pump {pump_name}: {value}\n ¯\_(ツ)_/¯ \nPlease check the manual for error code meanings.")
            return value

        except Exception as e:
            print(f"Failed to read error code of pump {pump_name}: {e}")

    def over_temperature_electronics(self, pump_name, p=True): #NOTE: may need to translate to boolean if it returns as int
        """
        Get the over temperature status of the pump drive electronics.
        """
        if pump_name == "TC80":
            idx = self.pump_address(pump_name)
            node_id = f"ns=1;s=P{idx}_304_OvTempElec"
            try:
                node = self.client.get_node(node_id)
                value = node.get_value()

                if p == True:
                    print(f"Over temperature status of pump {pump_name}: {value}")
                return value

            except Exception as e:
                print(f"Failed to read over temperature status of pump {pump_name}: {e}")

        else:
            raise ValueError(f"Over temperature status is only available for TC80 pump, not {pump_name}.")
        
    def over_temperature_pump(self, pump_name, p=True):
        """
        Get the over temperature status of the pump.
        """
        if pump_name == "TC80":
            idx = self.pump_address(pump_name)
            node_id = f"ns=1;s=P{idx}_305_OvTempPump"
            try:
                node = self.client.get_node(node_id)
                value = node.get_value()

                if p == True:
                    print(f"Over temperature status of pump {pump_name}: {value}")
                return value

            except Exception as e:
                print(f"Failed to read over temperature status of pump {pump_name}: {e}")

        else:
            raise ValueError(f"Over temperature status is only available for TC80 pump, not {pump_name}.")
        
    def set_speed_reached(self, pump_name, p=True):
        """
        Read whether the turbo has reached its set speed.
        """
        if pump_name == "TC80":
            idx = self.pump_address(pump_name)
            node_id = f"ns=1;s=P{idx}_306_SpdSetRchd"
            try:
                node = self.client.get_node(node_id)
                value = node.get_value()

                if p == True:
                    print(f"Set speed reached status of pump {pump_name}: {value}")
                return value

            except Exception as e:
                print(f"Failed to read set speed reached status of pump {pump_name}: {e}")

        else:
            raise ValueError(f"Set speed reached status is only available for TC80 pump, not {pump_name}.")
        
    def pump_accelerating(self, pump_name, p=True):
        """
        Read whether the pump is currently accelerating.
        """
        if pump_name == "TC80":
            idx = self.pump_address(pump_name)
            node_id = f"ns=1;s=P{idx}_307_PumpAccel"
            try:
                node = self.client.get_node(node_id)
                value = node.get_value()

                if p == True:
                    print(f"Accelerating status of pump {pump_name}: {value}")
                return value

            except Exception as e:
                print(f"Failed to read accelerating status of pump {pump_name}: {e}")

        else:
            raise ValueError(f"Accelerating status is only available for TC80 pump, not {pump_name}.")