from opcua import Client, ua

from .pfeiffer_parameters import PARAMETERS

class Controller:

    def __init__(self, client):
        self.client = client

    def _node_path(self, pump_id, parameter):
        param = PARAMETERS[parameter]

        return (
            f"ns=1;s=P{pump_id}{param.node_id}"
        )

    def read(self, pump_id, parameter):

        if parameter not in PARAMETERS:
            raise AttributeError(
                f"Unknown parameter: {parameter}"
            )

        param = PARAMETERS[parameter]

        node = self.client.get_node(
            self._node_path(pump_id, parameter)
        )

        value = node.get_value()

        return param.dtype(value)

    def write(self, pump_id, parameter, value):

        if parameter not in PARAMETERS:
            raise AttributeError(
                f"Unknown parameter: {parameter}"
            )

        param = PARAMETERS[parameter]

        if not param.writable:
            raise ValueError(
                f"{parameter} is read-only"
            )

        node = self.client.get_node(
            self._node_path(pump_id, parameter)
        )

        node.set_value(
            param.dtype(value)
        )


class Pump:

    def __init__(self, controller, pump_id):

        super().__setattr__(
            "_controller",
            controller
        )

        super().__setattr__(
            "_pump_id",
            pump_id
        )

    def __getattr__(self, name):

        if name in PARAMETERS:
            return self._controller.read(
                self._pump_id,
                name
            )

        raise AttributeError(name)

    def __setattr__(self, name, value):

        if name.startswith("_"):
            super().__setattr__(name, value)
            return

        if name in PARAMETERS:

            self._controller.write(
                self._pump_id,
                name,
                value
            )

            return

        raise AttributeError(
            f"Unknown parameter: {name}"
        )