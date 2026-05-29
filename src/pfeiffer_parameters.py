from dataclasses import dataclass

from .pfeiffer_enums import gas_model

@dataclass(frozen=True)
class Parameter:
    node_id: str
    dtype: type
    readable: bool = True
    writable: bool = False
    default: any = None


COMMON_PARAMETERS = {
    "standby": Parameter(
        node_id="_002_Stand-by",
        dtype=bool,
        writable=True,
        default=False,
    ),

    "error_acknowledge": Parameter(
        node_id="_009_ErrorAckn",
        dtype=bool,
        writable=True,
        readable=False,
        default=None,
    ),

    "pumping_power": Parameter(
        node_id="_010_PumpgStatn",
        dtype=bool,
        writable=True,
        default=False,
    ),

    #019, 024

    "speed_set_mode": Parameter(
        node_id="_026_SpdSetMode",
        dtype=bool,
        writable=True,
        default=False,
    ),
}

TC80_PARAMETERS = {
    "heating": Parameter(
        node_id="_001_Heating",
        dtype=bool,
        writable=True,
        default=False,
    ),

    "runnup_time_monitor": Parameter(
        node_id="_004_RUTimeCtrl",
        dtype=bool,
        writable=True,
        default=True,
    ),

    "enable_vent": Parameter(
        node_id="_012_EnableVent",
        dtype=bool,
        writable=True,
        default=True,
    ),

    # 017

    "motor_pump": Parameter(
        node_id="_023_MotorPump",
        dtype=bool,
        writable=True,
        default=True,
    ),

    # 025

    "gas_mode": Parameter(
        node_id="_027_GasMode",
        dtype=gas_model,
        writable=True,
        default=0,
    )
}

MVP_PARAMETERS = {
    
}

PUMP_PARAMETERS = {
    "TC80": {
        **COMMON_PARAMETERS,
        **TC80_PARAMETERS,
    },

    "MVP": {
        **COMMON_PARAMETERS,
        **MVP_PARAMETERS,
    },
}