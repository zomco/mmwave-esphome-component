# Copilot Instructions — Millimeter-Wave Radar ESPHome Components

## Project Overview

This repository contains custom ESPHome external components for multiple millimeter-wave radar models. Each component is developed independently under `components/{radar_model}/` and follows the [ESPHome starter-components](https://github.com/esphome/starter-components) template structure.

In addition to standard radar integration, every component implements two signature features:

- **Coordinate Transformation** — translates raw radar-frame target coordinates into room-frame coordinates based on the radar's physical installation parameters (position + full 3-D orientation).
- **Boundary Filtering** — discards targets outside a user-defined polygonal boundary, operating in room-frame coordinates (post-transform).

The **canonical reference implementation** is `components/r60abd1/`. All new components must structurally and stylistically follow it.

The **authoritative radar model status table** lives in `README.md`. The AI must update `README.md` whenever a model's status changes.

---

## Repository Layout

```
.
├── .github/
│   ├── copilot-instructions.md
│   └── workflows/
│       ├── ci.yml                     # PR: compiles all tests/*.yaml
│       ├── publish.yml                # Builds firmware, uploads artifacts
│       └── publish-pages.yml          # Deploys GitHub Pages (separate concern)
├── components/
│   ├── r60abd1/                        # ✅ Reference implementation
│   │   ├── __init__.py                 # CONFIG_SCHEMA, to_code, DEPENDENCIES,
│   │   │                               # and ALL entity declarations
│   │   ├── r60abd1.h
│   │   └── r60abd1.cpp
│   └── {radar_model}/
│       ├── __init__.py
│       ├── {radar_model}.h
│       └── {radar_model}.cpp
├── docs/
│   └── {radar_model}/
│       ├── <datasheet>.pdf            # Product documentation
│       ├── <protocol>.pdf             # Communication protocol
│       └── {radar_model}-*.{png|jpg}  # Wiring diagram images → published to Pages
├── static/
│   ├── _config.yml                    # Jekyll site config
│   └── index.html                     # GitHub Pages install site (ESP Web Tools)
├── tests/
│   └── {radar_model}.yaml             # Firmware config — one per radar model
└── README.md                          # ← Radar model status table lives here
```

> **Note on platform files:** All entities are declared in `__init__.py`. Do not create `sensor.py`, `binary_sensor.py`, or `number.py`.

---

## Status Table (maintained in `README.md`)

```
Planned → Developing → Testing → Completed
             ↕               ↕
           Paused          Paused
```

| Status       | Meaning |
|--------------|---------|
| `Planned`    | Documentation exists (or is awaited); component not yet generated. |
| `Developing` | Component generated; awaiting or undergoing on-hardware firmware tests. |
| `Testing`    | Firmware validated on hardware; undergoing Home Assistant integration tests. |
| `Completed`  | HA integration validated. No further changes unless explicitly requested. |
| `Paused`     | Blocked by an external condition (no hardware, no HA environment). AI makes no changes until maintainer explicitly resumes. |

**AI rule:** After any status change, update the table in `README.md` in the same response.

---

## Development Workflow

### Status: `Planned`

**Trigger:** Product documentation added to `docs/{radar_model}/`.

**AI responsibilities:**

1. Read **all** files in `docs/{radar_model}/` before writing any code — frame format, byte order, checksum, coordinate system, dimensionality.
2. Use `components/r60abd1/` as the canonical reference.
3. Determine output dimensionality:
   - **1-D** (range only): transform projects range along radar +X axis; boundary → `distance_min/distance_max`.
   - **2-D** (X, Y): full 2-D transform; polygon filter applies.
   - **3-D** (X, Y, Z): full 3-D transform; polygon filter on XY projection.
4. Generate `components/{radar_model}/__init__.py`, `.h`, `.cpp`, and `tests/{radar_model}.yaml`.
5. If docs are **missing or incomplete**, report specifically what is missing and leave status as `Planned`.
6. On success, update `README.md` status to `Developing`.

**Failure conditions (stay `Planned`):** Missing protocol doc; ambiguous frame format; undocumented coordinate axes.

---

### Status: `Developing`

Diagnose and fix firmware errors by tracing the parser against `docs/{radar_model}/`. Update status to `Testing` on maintainer confirmation.

### Status: `Testing`

Fix HA integration issues. Update status to `Completed` on maintainer confirmation.

### Status: `Paused`

Make no changes. Wait for maintainer to explicitly resume.

### Status: `Completed`

No AI-initiated changes unless explicitly requested.

---

## CI/CD Pipeline

The three workflow files have distinct, non-overlapping responsibilities.

### `ci.yml` — PR Validation

- **Trigger:** Push to non-main branches; pull requests.
- **What it does:** Compiles every `tests/*.yaml` in parallel using `esphome/build-action@v6`. Fails if any config does not compile.
- **Does not** build artifacts or deploy anything.

### `publish.yml` — Firmware Build

- **Trigger:** Push to `main`; new GitHub release; `workflow_dispatch`.
- **What it does:**
  1. Auto-discovers all `tests/*.yaml` files.
  2. Builds each with `esphome/build-action@v6` (`complete-manifest: true`).
  3. Uploads one artifact per model: `firmware-{name}` (e.g. `firmware-r60abd1-ESP32-C3`).
- **Does not** touch GitHub Pages. Separation of concerns: firmware build failure never breaks the live install site.

### `publish-pages.yml` — GitHub Pages Deployment

- **Trigger:** Completion of `publish.yml` on `main`; push to `static/**` or `docs/**`; `workflow_dispatch`.
- **What it does:**
  1. Resolves which `publish.yml` run to pull firmware artifacts from.
  2. Downloads all `firmware-*` artifacts.
  3. Builds the Jekyll site from `static/` → `_site/`.
  4. Copies firmware files and wiring images into `_site/`.
  5. Generates `_site/models.json` (name, version, chip family, manifest path, wiring image paths).
  6. Deploys `_site/` to GitHub Pages.
- **Triggered independently** from `publish.yml`: static content (wiring images, install page) can be redeployed without rebuilding firmware.

**Wiring images** are discovered automatically: any file matching `docs/{radar_model}/{radar_model}-*.{png|jpg|jpeg}` is copied to `_site/images/{radar_model}/` and listed in `models.json.wiring_images`. The install page displays them when a user opens the Wiring Guide for a model.

### Enabling GitHub Pages

1. **Settings → Pages → Source → GitHub Actions.**
2. Push to `main` or run `publish.yml` → `publish-pages.yml` triggers automatically.

---

## Firmware Configuration: `tests/{radar_model}.yaml`

Both `ci.yml` and `publish.yml` auto-discover `tests/*.yaml`. Adding a new YAML file is the only step needed to include a new model in CI and firmware builds — no workflow edits required.

### Required structure

```yaml
substitutions:
  name: r60abd1
  friendly_name: "R60ABD1 Radar"

esphome:
  name: "${name}"
  friendly_name: "${friendly_name}"
  project:
    name: "mmwave-radar.${name}"      # required — used in manifest.json
    version: "1.0.0"                  # bump on each release

# Target MCU: ESP32-C3 by default; adapt board for ESP32-S3 and others
esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: esp-idf                     # always esp-idf, never arduino

external_components:
  - source:
      type: local
      path: ../components             # relative to tests/
    components: ["{radar_model}"]

logger:
api:
ota:
  - platform: esphome

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  ap:
    ssid: "${friendly_name} Fallback"

uart:
  tx_pin: GPIO21
  rx_pin: GPIO20
  baud_rate: 115200                   # verify against protocol document

{radar_model}:
  radar_x: 0.0
  radar_y: 0.0
  radar_z: 2.4
  radar_yaw: 0.0
  radar_pitch: 0.0
  radar_roll: 0.0
```

### Rules

- `esphome.project.name` and `esphome.project.version` are **mandatory** — the build action uses them to generate `manifest.json`.
- `esp32.board` must be an ESP32-C3 variant for the current default target. For ESP32-S3 or others, use the appropriate board identifier.
- `esp32.framework.type` must be `esp-idf`.
- `external_components.source.path` must be `../components`.
- `uart.baud_rate` must match the radar's protocol document exactly.
- GPIO pin assignments should reflect the target hardware. Use `GPIO20` (RX) and `GPIO21` (TX) as defaults with a `# TODO` comment if not yet determined.

### Firmware artifact naming

`esphome/build-action@v6` names the build output `{esphome.name}-{chipFamily}`, e.g. `r60abd1-ESP32-C3`. This name is used for:
- The artifact uploaded by `publish.yml`: `firmware-r60abd1-ESP32-C3`
- The subdirectory in `_site/`: `_site/r60abd1-ESP32-C3/`
- The `id` field in `models.json`

`publish-pages.yml` derives the radar model name (for looking up `docs/` and wiring images) by stripping the chip-family suffix: `r60abd1-ESP32-C3` → `r60abd1`.

---

## Mandatory Feature Specifications

### 1. Coordinate Transformation

Full 3-D ZYX Tait-Bryan rotation (Yaw → Pitch → Roll). All six parameters exposed in CONFIG_SCHEMA for every model.

| Parameter     | Type    | Unit    | Default | Description                                                 |
|---------------|---------|---------|---------|-------------------------------------------------------------|
| `radar_x`     | `float` | m       | `0.0`   | Radar origin along the room X-axis.                        |
| `radar_y`     | `float` | m       | `0.0`   | Radar origin along the room Y-axis.                        |
| `radar_z`     | `float` | m       | `0.0`   | Radar mounting height (room Z-axis).                       |
| `radar_yaw`   | `float` | degrees | `0.0`   | Rotation around the room Z-axis (azimuth).                 |
| `radar_pitch` | `float` | degrees | `0.0`   | Rotation around the intermediate Y-axis (elevation tilt).  |
| `radar_roll`  | `float` | degrees | `0.0`   | Rotation around the radar's own X-axis (bank/roll).        |

#### C++ rotation matrix (pre-computed in `setup()`)

```cpp
void precompute_rotation_matrix() {
  const float cy = cosf(yaw_rad_),   sy = sinf(yaw_rad_);
  const float cp = cosf(pitch_rad_), sp = sinf(pitch_rad_);
  const float cr = cosf(roll_rad_),  sr = sinf(roll_rad_);
  // R = Rz(yaw) * Ry(pitch) * Rx(roll)
  r_[0][0]=cy*cp;  r_[0][1]=cy*sp*sr-sy*cr;  r_[0][2]=cy*sp*cr+sy*sr;
  r_[1][0]=sy*cp;  r_[1][1]=sy*sp*sr+cy*cr;  r_[1][2]=sy*sp*cr-cy*sr;
  r_[2][0]=-sp;    r_[2][1]=cp*sr;            r_[2][2]=cp*cr;
}

void transform_point(float lx, float ly, float lz,
                     float &rx, float &ry, float &rz) const {
  rx = r_[0][0]*lx + r_[0][1]*ly + r_[0][2]*lz + radar_x_;
  ry = r_[1][0]*lx + r_[1][1]*ly + r_[1][2]*lz + radar_y_;
  rz = r_[2][0]*lx + r_[2][1]*ly + r_[2][2]*lz + radar_z_;
}
```

Never call `sinf`/`cosf` inside `loop()`.

#### Dimensionality mapping

| Radar output     | Input to transform     | Published entities          |
|------------------|------------------------|-----------------------------|
| 1-D (range only) | `(range, 0, 0)`        | `room_x`, `room_y`, `room_z`|
| 2-D (X, Y)       | `(lx, ly, 0)`          | `room_x`, `room_y`          |
| 3-D (X, Y, Z)    | `(lx, ly, lz)`         | `room_x`, `room_y`, `room_z`|

---

### 2. Boundary Filtering

Operates **exclusively in room-frame coordinates** (after transform).

- Minimum 3 vertices; validate in `__init__.py`.
- Empty → filter disabled (pass-through).
- For 2-D/3-D: Ray Casting on XY projection. For 1-D: `distance_min`/`distance_max` range gate.

**Required processing order:** parse → transform → filter → publish.

---

## Platform Exposure Convention

All entities declared in `__init__.py`. No `sensor.py`, `binary_sensor.py`, `number.py`.

Entity sub-schemas: `sensor.SENSOR_SCHEMA`, `binary_sensor.BINARY_SENSOR_SCHEMA`, `number.NUMBER_SCHEMA`. Entities registered in `to_code` via `sensor.new_sensor`, etc.

---

## Code Standards

- **C++14**, `#pragma once`, namespace `esphome::{radar_model}`.
- Members: trailing underscore. Constants: `static constexpr ALL_CAPS`.
- No dynamic allocation in hot paths.
- Parser: **state-machine based**, non-blocking.
- Validate values before `publish_state()` (reject NaN, ±Inf, out-of-range).
- Logging: `ESP_LOGD/I/W/E` with `TAG`. No `Serial.print`.
- All public methods: `///` Doxygen comment.
- Python: `CONF_*` constants, `snake_case` keys, `DEPENDENCIES = ["uart"]`.

---

## Pre-`Developing` Checklist

- [ ] `__init__.py`, `.h`, `.cpp` present (no separate platform files).
- [ ] `DEPENDENCIES = ["uart"]` in `__init__.py`.
- [ ] CONFIG_SCHEMA: `radar_x/y/z`, `radar_yaw/pitch/roll`, `boundary` (≥3-vertex validation).
- [ ] All entities in `__init__.py` using canonical ESPHome helpers.
- [ ] Radar-local axis convention in header comment.
- [ ] Rotation matrix: ZYX Tait-Bryan, pre-computed in `setup()`.
- [ ] Boundary filter: room-frame coords, post-transform.
- [ ] Processing order: parse → transform → filter → publish.
- [ ] Non-blocking state-machine parser.
- [ ] All values range-checked before `publish_state()`.
- [ ] `tests/{radar_model}.yaml` with `esphome.project.name/version`, ESP32-C3 board, `esp-idf`.
- [ ] `README.md` status updated to `Developing`.