# mmwave-esphome(WIP)
[![CI](https://github.com/zomco/mmwave-esphome-component/actions/workflows/ci.yml/badge.svg)](https://github.com/zomco/mmwave-esphome-component/actions/workflows/ci.yml)
[![Publish Firmware](https://github.com/zomco/mmwave-esphome-component/actions/workflows/publish-firmware.yml/badge.svg)](https://github.com/zomco/mmwave-esphome-component/actions/workflows/publish-firmware.yml)
[![Publish Pages](https://github.com/zomco/mmwave-esphome-component/actions/workflows/publish-pages.yml/badge.svg)](https://github.com/zomco/mmwave-esphome-component/actions/workflows/publish-pages.yml)

ESPHome firmware for mmWave radar with calibration capabilities. Optimize your mmwave radar based on its installation location.

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

## Radar Comparison


| Radar Model | Operating Frequency | Power Supply / Current | Detection Distance / Angle | Dimensions | Resolution / Accuracy | TX/RX Antennas | App Configuration | Main Features / Application Scenarios |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HLK-LD2401** | 24GHz | 3.3V/5V, - | 6m, ±60° | - | 0.75m or 0.2m | - | **Yes** (Bluetooth) | Human presence status sensing |
| **HLK-LD2401F** | 24~24.25GHz | 3.3V (with LDO 5V), 50mA | 10m, - | 20x20mm | ±0.15m | - | - | Static human presence sensor |
| **HLK-LD2402** | 24~24.25GHz | 3.3V (with LDO 5V), 50mA | 10m, Base radius 5m | 20x20mm | ±0.15m | - | - | Static human presence sensor |
| **HLK-LD2402-L** | 24~24.25GHz | 3.3V (with LDO 5V), 50mA | 10m, Base radius 5m | 20x20mm | ±0.15m | - | - | With photosensitive firmware |
| **HLK-LD2410** | 24GHz | 5V, 80mA | 6m, ±60° | 7x35mm | 0.75m or 0.2m | - | **Yes** (Bluetooth) | Human presence status sensing |
| **HLK-LD2410B** | 24GHz | 5V, 82mA | 0.75m~6m, ±60°| 7x35mm | 0.75m | - | **Yes** (Bluetooth) | Bluetooth/Serial parameter tuning |
| **HLK-LD2410C** | 24GHz | 5~12V, >200mA | 6m, - | 16x22mm | 0.75m | - | **Yes** (Bluetooth) | Human presence status sensing |
| **HLK-LD2410D-B**| 24~24.25GHz | 3.3/5V, 100mA | 10m, - | 7x35mm | ±0.15m | - | - | Static human presence sensor |
| **HLK-LD2410S** | 24GHz | 3.3V, 0.1mA | 8m, ±60° | 20x20mm | - | - | - | Battery-powered ultra-low power |
| **HLK-LD2411-S** | 24~24.25GHz | 5V, 48mA | 6m, Azimuth ±20° Pitch ±45°| 27.5x27.5mm | ±5cm or ±5% | - | **Yes** (Bluetooth) | Motion/micro-motion sensing & ranging |
| **HLK-LD2412** | 24GHz | 3.3V/5V, 90mA | 0.75m~9m, ±75°| 11x28mm | 0.75m | - | - | Human presence status sensing |
| **HLK-LD2413** | 23~27GHz | 3.3V, 16~23mA | 10m, 12° | 44x36mm | ±3mm | - | - | High-precision liquid/material level detection |
| **HLK-LD2415H** | 24.125GHz | 9~24V, 50mA | ≥180m, Horizontal ±20° Vertical ±8°| 69x53mm | - | - | No (Supports PC Tool) | Vehicle speed measurement |
| **HLK-LD2416** | 24~24.25GHz | 5V, 80mA | 7.5m, - | 14x14mm | ±0.15m | - | - | Ultra-small AiP sensor |
| **HLK-LD2417** | 24~24.25GHz | 5~12V, 40mA | 100m, Azimuth ±60° Pitch ±20°| - | - | - | **Yes** (Bluetooth) | Outdoor vehicle detection |
| **HLK-LD2420** | 24~24.25GHz | 3.3V, 50mA | 8m, - | 20x20mm | ±0.35m | - | No (Supports PC Tool) | Motion/micro-motion human sensing |
| **HLK-LD2450** | 24GHz | 5V, - | 6m, Azimuth ±60° Pitch ±35°| 15x44mm | - | - | - | Target precise positioning & tracking |
| **HLK-LD2450A** | 24~24.25GHz | 5V, - | Human 2m (Gesture 0.3m), Horizontal 120°| 13.4x44mm | 0.2m | - | - | Proximity sensing & gesture recognition |
| **HLK-LD2451** | 24~24.25GHz | 5V, 107mA | 100m, Horizontal ±15° Vertical ±7°| 70x35mm | - | - | **Yes** (Bluetooth) | Outdoor vehicle target detection |
| **HLK-LD2452** | 24~24.25GHz | 3.3~5V, 126mA| 6m, Azimuth ±60° Pitch ±30°| 23x42mm | 0.72m (Accuracy 0.15m)| - | - | High-precision multi-target tracking |
| **HLK-LD2453** | 24~24.25GHz | 3.3V, 76mA | 6m, Horizontal ±40° | 7x30mm | 0.72m (Accuracy 0.15m)| - | - | High-precision multi-target recognition |
| **HLK-LD2460** | 24~24.25GHz | 5V, ≤250mA | 6m, Horizontal 120° Vertical 90°| 32x49.5mm | 0.75m (Accuracy 0.3m) | - | No (Supports PC Tool) | Multi-target trajectory tracking |
| **HLK-LD2461** | 24~24.25GHz | 5V, 260~400mA| 8m, Horizontal ±45° Pitch ±25°| 35x50mm | 0.75m (Accuracy 0.1m) | - | - | 5-person trajectory, breathing & heartbeat |
| **HLK-LD6001 Series**| 60~64GHz | 3.3V, 1.1W~1.7W| 8m, ±60°/±30° | 29x28/60x30mm| - | - | - | Indoor human sensing & tracking |
| **HLK-LD6002B/C**| 58~64GHz | 3.3V, 600mA | 6m, ±60° | 23x25mm | 0.4m | - | No (Supports GUI) | 3D presence radar |
| **HLK-LD6002K** | 58~62GHz | 5V, 100mA | 1.5m, ±100° | - | - | - | - | Vital signs (breathing & heartbeat) |
| **HLK-LD6003** | 58~64GHz | 3.3V, 1.98W | 3m, ±60° | 31.5x25x8.7mm| - | - | - | Human breathing & heart rate monitoring |
| **HLK-LD6003B** | 58~64GHz | - | 6m, ±60° | 25x31.5mm | - | **2T2R** | No (Supports PC Tool) | Multi-target tracking / top & side mount |
| **MR24BSD1** | 24~24.25GHz | 5V, 93mA | 2.75m, 40°/40° | 46x27.5mm | - | - | - | Breathing & sleep detection |
| **MR24FDB1** | 24~24.25GHz | 5V, 93mA | 12m, 90°/60° | 35x30mm | - | - | - | Fall detection |
| **MR24HPB1** | 24GHz | 5V, 200mA | 3m, 100°/80° | - | - | - | - | Sleep & breathing monitoring |
| **MR24HPC1** | 24GHz | 5V, 110mA | 5m, 90°/60° | - | - | - | - | Human static presence |
| **MR60BHA1/2** | 60GHz | - | 2m | - | - | - | - | Breathing & heartbeat radar |
| **MR60FDA2** | 60GHz | - | - | - | - | - | - | Fall threshold setting detection |
| **Rd-01** | 24~24.25GHz | 3.3V, ≥500mA| 5m, ±60° | 35x18mm | - | **1T1R** | **Yes** (Wi-Fi/BLE) | Wi-Fi & BLE human sensing |
| **Rd-03_V2** | 24GHz | 3.3V/5V, 50mA | 7m, ±60° | 20x20mm | - | **1T1R** | No (Supports PC Tool) | Ultra-small motion/micro-motion sensing |
| **Rd-03D_V2** | 24~24.25GHz | 5V, 92mA | 8m, Azimuth ±60° Pitch ±30°| 15x44mm | - | **1T2R** | No (Supports PC Tool) | Multi-target trajectory positioning & tracking |
| **Rd-03E** | 24GHz | 5V, 44mA | 6m, Azimuth ±20° Pitch ±45°| 28x24mm | ±5cm or ±5% | **1T1R** | No (Supports PC Tool) | High-precision ranging, gesture recognition |
| **Rd-03H** | 24GHz | 3.3V/5V, 50mA | 8m, ±60° | 35x7mm | - | **1T1R** | No (Supports PC Tool) | Ultra-small size, motion/micro-motion |
| **Rd-03L_V2** | 24GHz | 3.3V, <80uA | 8m, ±60° | 17.5x19.0mm| - | **1T1R** | No (Supports PC Tool) | Ultra-low power human sensing |

*(Note: "-" denotes that the specific value is not explicitly mentioned or provided in the source manuals.)*

## How it works

## Why it exists
mmWave radar is susceptible to the influence of its installation position and surrounding environment. therefore, a tool is required to assist users in calibrating the radar based on actual conditions, thereby optimizing its real-world performance.
