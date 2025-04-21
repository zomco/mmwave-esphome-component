import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

# Declare the namespace for our C++ code
CODEOWNERS = ["@zomco"]  # Optional: Add your GitHub username
DEPENDENCIES = ["uart"]  # This component requires UART
MULTI_CONF = True  # Allow multiple instances of this component

# Namespace in C++
r60abd1_ns = cg.esphome_ns.namespace("r60abd1")
# Get the C++ class defined in the .h file
R60ABD1 = r60abd1_ns.class_("R60ABD1", cg.Component, uart.UARTDevice)

# Define the basic configuration schema for the component hub
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(R60ABD1),
        # Add component-level configuration options here if needed later
        # For example:
        # cv.Optional("enable_ota"): cv.boolean,
    }
).extend(
    uart.UART_DEVICE_SCHEMA
)  # Inherit UART settings (tx_pin, rx_pin, baud_rate etc)


# The function that translates YAML config to C++ code
async def to_code(config):
    # Get the UART parent object
    uart_component = await cg.get_variable(config[uart.CONF_UART_ID])
    # Create an instance of our C++ class
    var = cg.new_Pvariable(config[CONF_ID])

    # Register the component with ESPHome core
    await cg.register_component(var, config)
    # Register the component as a UART device
    await uart.register_uart_device(var, config)

    # Example: Pass component-level config to C++
    # if "enable_ota" in config:
    #    cg.add(var.set_ota_enabled(config["enable_ota"]))

    # Set the UART parent for the C++ object
    cg.add(var.set_uart_parent(uart_component))  # This line is important!
