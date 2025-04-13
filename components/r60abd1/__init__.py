import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@limengdu"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

R60ABD1_ns = cg.esphome_ns.namespace("R60ABD1")

R60ABD1Component = R60ABD1_ns.class_(
    "R60ABD1Component", cg.Component, uart.UARTDevice
)

CONF_R60ABD1_ID = "R60ABD1_id"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(R60ABD1Component),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "R60ABD1",
    require_tx=True,
    require_rx=True,
    baud_rate=115200,
    parity="NONE",
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)