# mmwave-esphome(WIP)
[![CI](https://github.com/zomco/mmwave-esphome-component/actions/workflows/ci.yml/badge.svg)](https://github.com/zomco/mmwave-esphome-component/actions/workflows/ci.yml)
[![Publish Firmware](https://github.com/zomco/mmwave-esphome-component/actions/workflows/publish-firmware.yml/badge.svg)](https://github.com/zomco/mmwave-esphome-component/actions/workflows/publish-firmware.yml)
[![Publish Pages](https://github.com/zomco/mmwave-esphome-component/actions/workflows/publish-pages.yml/badge.svg)](https://github.com/zomco/mmwave-esphome-component/actions/workflows/publish-pages.yml)

ESPHome firmware for mmwave radar with calibration capabilities. Optimize your mmwave radar based on its installation location.

## Usage

### Factory firmware
TBD.

### Customize firmware
1. Import components for your `radar_model` to `.yaml` in [ESPHome Device Builder](https://esphome.io/guides/getting_started_hassio/)

```yaml
## e.g. radar_model=r60abd1
external_components:
  - source:
      type: git
      path: https://github.com/zomco/mmwave-esphome
    components: [r60abd1]
```

2. Configure the UART bus based on the actual UART communication pins(`mcu_tx` and `mcu_rx`) of the MCU and the radar.

```yaml
## e.g. radar_model=r60abd1, mcu_tx=GPIO14, mcu_rx=GPIO15

uart:
  id: uart_bus
  tx_pin: GPIO14
  rx_pin: GPIO15
  baud_rate: 115200
  debug:

r60abd1:
  id: radar_hub
  uart_id: uart_bus
```

3. Refer to the sample configuration files in the corresponding directory for your `radar_model`, and add configuration entries as needed. 

```yaml
## e.g. radar_model=r60abd1
## find configuration in /docs/r60abd1/example.yaml

sensor:
  - platform: r60abd1 
    distance:
      name: "Distance"
    body_movement:
      name: "Body Movement"
    heart_rate:
      name: "Heart Rate"
    respiration_rate:
      name: "Respiration Rate"
    ...

```

4. Customize UI for your firmware on `Home Assistant`.

## Support Modules

- [MicRadar](https://www.micradar.cn/) 
    - [R60ABD1](./docs/r60abd1/README.md)

- [HiLink](https://www.hlktech.com/)

## How it works

## Why it exists
mmwave radar is susceptible to the influence of its installation position and surrounding environment. therefore, a tool is required to assist users in calibrating the radar based on actual conditions, thereby optimizing its real-world performance.
