from opcua import Client, ua

from SuperLOTIS_v2.pfeiffer_parameters import Parameter, PUMP_PARAMETERS

class Controller:

    def __init__(self, client):
        self.client = Client(client)

    def _node_path(self, pump_id, pump_name, parameter):
        param = PUMP_PARAMETERS.get(pump_name, {}).get(parameter)

        return (
            f"ns=1;s=P{pump_id}{param.node_id}"
        )

    def read(self, pump_id, pump_name, parameter):

        if parameter not in PUMP_PARAMETERS.get(pump_name, {}):
            raise AttributeError(
                f"Unknown parameter: {parameter}"
            )

        param = PUMP_PARAMETERS.get(pump_name, {}).get(parameter)

        node = self.client.get_node(
            self._node_path(pump_id, pump_name, parameter)
        )

        value = node.get_value()

        return param.dtype(value)

    def write(self, pump_id, pump_name, parameter, value):

        if parameter not in PUMP_PARAMETERS.get(pump_name, {}):
            raise AttributeError(
                f"Unknown parameter: {parameter}"
            )

        param = PUMP_PARAMETERS.get(pump_name, {}).get(parameter)

        if not param.writable:
            raise ValueError(
                f"{parameter} is read-only"
            )

        node = self.client.get_node(
            self._node_path(pump_id, pump_name, parameter)
        )

        node.set_value(
            param.dtype(value)
        )


class Pump:

    def __init__(self, controller, pump_name, pump_id):

        super().__setattr__(
            "_controller",
            controller
        )

        super().__setattr__(
            "_pump_name",
            pump_name
        )

        super().__setattr__(
            "_pump_id",
            pump_id
        )

    def __getattr__(self, name):

        if name in PUMP_PARAMETERS[self._pump_name]:
            return self._controller.read(
                self._pump_id,
                self._pump_name,
                name
            )

        raise AttributeError(name)

    def __setattr__(self, name, value):

        if name.startswith("_"):
            super().__setattr__(name, value)
            return

        if name in PUMP_PARAMETERS.get(self._pump_name, {}):

            self._controller.write(
                self._pump_id,
                self._pump_name,
                name,
                value
            )

            return

        raise AttributeError(
            f"Unknown parameter: {name}"
        )