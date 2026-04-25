# ESPHome Millimeter-Wave Radar Components

Custom [ESPHome](https://esphome.io/) external components for multiple millimeter-wave radar models, with built-in **coordinate transformation** and **boundary filtering** — features not found in typical community radar integrations.

---

## Features

### Coordinate Transformation

Converts raw radar-frame target positions into room-frame coordinates based on the radar's physical installation parameters. Supports arbitrary mounting orientation via a full ZYX Tait-Bryan rotation (yaw, pitch, roll), making it suitable for ceiling-mounted, wall-mounted, or angled installations.

```yaml
r60abd1:
  radar_x: 2.5       # metres from room origin, X-axis
  radar_y: 3.0       # metres from room origin, Y-axis
  radar_z: 2.4       # mounting height, metres
  radar_yaw: 180.0   # degrees
  radar_pitch: 0.0   # degrees (0 = horizontal)
  radar_roll: 0.0    # degrees
```

### Boundary Filtering

Automatically discards targets detected outside a user-defined polygon. The polygon is expressed in room-frame coordinates (after coordinate transformation), so it directly corresponds to the physical layout of the monitored space.

```yaml
r60abd1:
  boundary:
    - x: 0.0
      y: 0.0
    - x: 5.0
      y: 0.0
    - x: 5.0
      y: 4.0
    - x: 0.0
      y: 4.0
```

If `boundary` is omitted, all targets are passed through without filtering.

---

## Radar Model Status

| Radar Model | Status      | Docs | Component | Notes |
|-------------|-------------|------|-----------|-------|
| R60ABD1     | ✅ Completed | [`docs/r60abd1/`](docs/r60abd1/) | [`components/r60abd1/`](components/r60abd1/) | Reference implementation |

**Status definitions:**

| Status       | Description |
|--------------|-------------|
| `Planned`    | Documentation staged; component not yet generated. |
| `Developing` | Component generated; undergoing on-hardware firmware tests. |
| `Testing`    | Firmware validated; undergoing Home Assistant integration tests. |
| `Completed`  | HA integration validated and stable. |
| `Paused`     | Blocked by an external condition (no hardware, no test environment, etc.). |

> This table is the authoritative source of truth. It is updated by the AI whenever a model's status changes.

---

## Repository Structure

```
.
├── .github/
│   └── copilot-instructions.md   # AI development rules and workflow
├── components/
│   └── {radar_model}/
│       ├── __init__.py           # Schema, code generation, all entity declarations
│       ├── {radar_model}.h       # C++ class declaration
│       └── {radar_model}.cpp     # Protocol parser, transform, filter, publish
├── docs/
│   └── {radar_model}/            # Product datasheet and communication protocol
├── tests/
│   └── {radar_model}.yaml        # ESPHome test configuration
└── README.md
```

---

## Usage

### 1. Add the external component

```yaml
external_components:
  - source: github://zomco/mmwave-esphome
    components: [r60abd1]
```

### 2. Configure UART

```yaml
uart:
  tx_pin: TX
  rx_pin: RX
  baud_rate: 115200  # check your radar model's spec
```

### 3. Add the radar component

```yaml
r60abd1:
  # Installation position (metres)
  radar_x: 2.5
  radar_y: 3.0
  radar_z: 2.4

  # Installation orientation (degrees, ZYX Tait-Bryan)
  radar_yaw: 180.0
  radar_pitch: 0.0
  radar_roll: 0.0

  # Room boundary polygon (room-frame coordinates, minimum 3 vertices)
  boundary:
    - x: 0.0
      y: 0.0
    - x: 5.0
      y: 0.0
    - x: 5.0
      y: 4.0
    - x: 0.0
      y: 4.0

  # Sensors (all optional)
  target_count:
    name: "Target Count"
  target_x:
    name: "Target X"
  target_y:
    name: "Target Y"
  target_z:
    name: "Target Z"
  presence:
    name: "Presence"
```

> Refer to `tests/{radar_model}.yaml` for a complete, compilable example for each model.

---

## Adding a New Radar Model

1. Create `docs/{radar_model}/` and place the product datasheet and communication protocol document inside.
2. Set the model's status to `Planned` in the table above.
3. Ask the AI to generate the component. It will read the documentation, generate `components/{radar_model}/`, and update the status to `Developing`.
4. Flash the firmware, test on hardware, and report any issues back to the AI for fixes.
5. Once hardware tests pass, the AI updates the status to `Testing`.
6. Validate in Home Assistant. Once complete, the AI updates the status to `Completed`.

See [`.github/copilot-instructions.md`](.github/copilot-instructions.md) for the full AI workflow and code standards.

---

## Development Notes

- All components follow the [ESPHome external component](https://esphome.io/components/external_components) standard.
- The reference implementation is `components/r60abd1/`. All new components are generated with it as the structural and stylistic template.
- Coordinate transformation uses a pre-computed ZYX rotation matrix (cached in `setup()`); no trigonometry is performed in `loop()`.
- Boundary filtering uses a Ray Casting algorithm and supports both convex and concave polygons.
- Protocol parsers are state-machine based and non-blocking.

---

## License

MIT