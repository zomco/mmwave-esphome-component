# Copilot Instructions — Millimeter-Wave Radar ESPHome Components

## Project Overview

This repository contains custom ESPHome external components for multiple millimeter-wave radar models. Each component is developed independently under `components/{radar_model}/` and follows the [ESPHome starter-components](https://github.com/esphome/starter-components) template structure.

In addition to standard radar integration, every component in this project implements two signature features absent from typical community radar components:

- **Coordinate Transformation** — translates raw radar-frame target coordinates into room-frame coordinates based on the radar's physical installation parameters (position + full 3-D orientation).
- **Boundary Filtering** — discards targets detected outside a user-defined polygonal boundary, operating in room-frame coordinates (post-transform).

The **canonical reference implementation** is `components/r60abd1/`. All new components must structurally and stylistically follow it.

The **authoritative radar model status table** lives in `README.md`. The AI must update `README.md` whenever a model's status changes.

---

## Repository Layout

```
.
├── .github/
│   └── copilot-instructions.md        # This file
├── components/
│   ├── r60abd1/                        # ✅ Reference implementation (Completed)
│   │   ├── __init__.py                 # CONFIG_SCHEMA, to_code, DEPENDENCIES,
│   │   │                               # and ALL sensor/binary_sensor/number entity declarations
│   │   ├── r60abd1.h                   # Class declaration
│   │   └── r60abd1.cpp                 # Protocol parser, transform, filter, publish
│   └── {radar_model}/
│       ├── __init__.py
│       ├── {radar_model}.h
│       └── {radar_model}.cpp
├── docs/
│   └── {radar_model}/                 # Product documentation (manual, protocol spec)
├── tests/
│   └── {radar_model}.yaml             # Minimal ESPHome test configuration
└── README.md                          # ← Radar model status table lives here
```

> **Note on platform files:** This project declares all sensor, binary sensor, and number entities directly inside `__init__.py`. Separate `sensor.py` / `binary_sensor.py` / `number.py` files are **not used**. Do not create them.

---

## Status Table (maintained in `README.md`)

The status table in `README.md` tracks every radar model. Valid statuses and their transitions:

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
| `Paused`     | Progress blocked by an external condition (e.g., no hardware, no HA test environment). AI makes no changes until the maintainer explicitly resumes. |

**AI rule:** After any action that changes a model's status, update the status table in `README.md` in the same response.

---

## Development Workflow

### Status: `Planned`

**Trigger:** The maintainer has added product documentation under `docs/{radar_model}/`.

**AI responsibilities:**

1. Read **all** files under `docs/{radar_model}/` before writing any code — especially the communication protocol, frame format, byte order, checksum algorithm, and the coordinate system description (dimensionality, axis orientation).
2. Use `components/r60abd1/` as the canonical reference for file structure, class hierarchy, naming conventions, entity declaration style, and feature architecture.
3. Determine the radar's **output dimensionality** from the protocol document:
   - **1-D** (distance/range only): transform projects range along the radar's forward axis; boundary filter is replaced by a `distance_min` / `distance_max` range gate.
   - **2-D** (X + Y, or range + azimuth): full 2-D transform; polygon boundary filter applies.
   - **3-D** (X + Y + Z): full 3-D transform; polygon boundary filter applies on the XY projection.
4. Generate the complete component (`__init__.py`, `{radar_model}.h`, `{radar_model}.cpp`) and `tests/{radar_model}.yaml`.
5. If `docs/{radar_model}/` is **missing or incomplete**, do not generate partial code. Report specifically what is missing and leave status as `Planned`.
6. On success, update `README.md` status to `Developing`.

**Failure conditions (status stays `Planned`):**
- Communication protocol document absent.
- Frame format ambiguous and unresolvable from available docs.
- Coordinate system or axis orientation undocumented.

---

### Status: `Developing`

**Trigger:** Component code exists; maintainer is running on-hardware firmware tests.

**AI responsibilities:**

1. When the maintainer reports a firmware error (compile error, crash, wrong values), diagnose by tracing the protocol parser against `docs/{radar_model}/`. Fix only the root cause.
2. Never guess byte layouts — always verify against the protocol document.
3. Re-generate only the affected files; do not touch unrelated components.
4. On maintainer confirmation of successful hardware test, update `README.md` status to `Testing`.

---

### Status: `Testing`

**Trigger:** Firmware passes hardware tests; component is under Home Assistant integration validation.

**AI responsibilities:**

1. Diagnose and fix only the reported HA integration issue.
2. Most likely sources of HA-level bugs: coordinate transform math, boundary filter edge cases, entity naming, unit-of-measure mismatches.
3. On maintainer confirmation, update `README.md` status to `Completed`.

---

### Status: `Paused`

**Trigger:** Maintainer sets this status due to an external blocking condition.

**AI responsibilities:** Make **no changes** to the component. Wait for the maintainer to explicitly resume and state which workflow step to continue from.

---

### Status: `Completed`

No AI-initiated changes unless explicitly requested by the maintainer.

---

## Mandatory Feature Specifications

### 1. Coordinate Transformation

#### Installation Model

Radars in this project may be installed in **any physical orientation** — ceiling-mounted, wall-mounted, angled, or tilted. A full 3-D rotation is always required. The convention is **ZYX Tait-Bryan (Yaw → Pitch → Roll)**, standard in robotics and aerospace.

| Parameter     | Type    | Unit    | Default | Description                                                 |
|---------------|---------|---------|---------|-------------------------------------------------------------|
| `radar_x`     | `float` | m       | `0.0`   | Radar origin along the room X-axis.                        |
| `radar_y`     | `float` | m       | `0.0`   | Radar origin along the room Y-axis.                        |
| `radar_z`     | `float` | m       | `0.0`   | Radar mounting height (room Z-axis).                       |
| `radar_yaw`   | `float` | degrees | `0.0`   | Rotation around the room Z-axis (azimuth, horizontal pan). |
| `radar_pitch` | `float` | degrees | `0.0`   | Rotation around the intermediate Y-axis (elevation tilt).  |
| `radar_roll`  | `float` | degrees | `0.0`   | Rotation around the radar's own X-axis (bank/roll).        |

All six parameters must be exposed in `__init__.py` CONFIG_SCHEMA for every radar model, regardless of dimensionality. For horizontally-mounted radars, `pitch` and `roll` simply default to `0.0`.

#### Rotation Matrix Pre-computation (C++ reference)

```cpp
/// Pre-compute the ZYX Tait-Bryan rotation matrix.
/// Must be called in setup() and whenever any angle parameter changes.
/// Caches the result in r_[3][3] to avoid repeated trig in loop().
void precompute_rotation_matrix() {
  const float cy = cosf(yaw_rad_),   sy = sinf(yaw_rad_);
  const float cp = cosf(pitch_rad_), sp = sinf(pitch_rad_);
  const float cr = cosf(roll_rad_),  sr = sinf(roll_rad_);

  // R = Rz(yaw) * Ry(pitch) * Rx(roll)
  r_[0][0] = cy * cp;
  r_[0][1] = cy * sp * sr - sy * cr;
  r_[0][2] = cy * sp * cr + sy * sr;

  r_[1][0] = sy * cp;
  r_[1][1] = sy * sp * sr + cy * cr;
  r_[1][2] = sy * sp * cr - cy * sr;

  r_[2][0] = -sp;
  r_[2][1] = cp * sr;
  r_[2][2] = cp * cr;
}

/// Transform a radar-local point (lx, ly, lz) to room-frame (rx, ry, rz).
void transform_point(float lx, float ly, float lz,
                     float &rx, float &ry, float &rz) const {
  rx = r_[0][0] * lx + r_[0][1] * ly + r_[0][2] * lz + radar_x_;
  ry = r_[1][0] * lx + r_[1][1] * ly + r_[1][2] * lz + radar_y_;
  rz = r_[2][0] * lx + r_[2][1] * ly + r_[2][2] * lz + radar_z_;
}
```

Store the matrix as `float r_[3][3]` in the class. **Never** call `sinf` / `cosf` inside `loop()` or the parser.

#### Dimensionality Mapping

| Radar output     | Radar-local inputs fed to transform   | Room-frame entities published |
|------------------|---------------------------------------|-------------------------------|
| 1-D (range only) | `(range, 0, 0)` along radar +X axis   | `room_x`, `room_y`, `room_z` |
| 2-D (X, Y)       | `(lx, ly, 0)`                         | `room_x`, `room_y`           |
| 3-D (X, Y, Z)    | `(lx, ly, lz)`                        | `room_x`, `room_y`, `room_z` |

The radar-local axis convention (which physical direction is +X, +Y, +Z) **must** be documented in a header comment in `{radar_model}.h`, derived from `docs/{radar_model}/`.

---

### 2. Boundary Filtering

Boundary filtering operates **exclusively in room-frame coordinates** (after coordinate transformation). Polygon vertices are provided in room coordinates.

**Configuration (YAML):**

```yaml
{radar_model}:
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

Rules:
- Minimum 3 vertices; validate in `__init__.py` and reject fewer with a descriptive error message.
- If `boundary` is absent or the list is empty, the filter is **disabled** (all targets pass through).
- For 2-D and 3-D radars: polygon evaluated in the XY plane (floor-plan projection).
- For 1-D radars: polygon is replaced by a scalar `distance_min` / `distance_max` range gate.

**Implementation (Ray Casting — supports convex and concave polygons):**

```cpp
/// Returns true if room-frame point (px, py) lies inside the boundary polygon.
/// Always returns true (pass-through) when the polygon has fewer than 3 vertices.
bool is_inside_boundary(float px, float py) const {
  const size_t n = boundary_polygon_.size();
  if (n < 3) return true;

  bool inside = false;
  for (size_t i = 0, j = n - 1; i < n; j = i++) {
    const float xi = boundary_polygon_[i].x, yi = boundary_polygon_[i].y;
    const float xj = boundary_polygon_[j].x, yj = boundary_polygon_[j].y;

    const bool crosses = ((yi > py) != (yj > py)) &&
                         (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
    if (crosses) inside = !inside;
  }
  return inside;
}
```

**Required processing order (must not be changed):**

```
raw UART frame
      ↓
  parse & validate bytes
      ↓
  coordinate transform      ← radar frame → room frame
      ↓
  boundary filter           ← room-frame coordinates only
      ↓
  publish to ESPHome sensors
```

---

## Platform Exposure Convention

**All entities — sensor, binary sensor, and number — are declared directly in `__init__.py`.**

This is the pattern established by the reference implementation `components/r60abd1/`. Do not create separate `sensor.py`, `binary_sensor.py`, or `number.py` files.

In `__init__.py`, entity platforms are registered via the appropriate ESPHome code-generation helpers (e.g., `cg.get_variable`, `sensor.new_sensor`, `binary_sensor.new_binary_sensor`) within the `to_code` coroutine. CONFIG_SCHEMA must include sub-schemas for each entity using `sensor.SENSOR_SCHEMA`, `binary_sensor.BINARY_SENSOR_SCHEMA`, etc., as applicable.

---

## Code Standards

### C++

- Standard: **C++14** (Arduino / ESP-IDF as used by ESPHome).
- `#pragma once` at the top of every header.
- Namespace: `esphome::{radar_model}`.
- Class member names: trailing underscore (`target_count_`, `r_[3][3]`).
- Constants: `static constexpr`, `ALL_CAPS_WITH_UNDERSCORES`.
- No dynamic allocation (`new` / `delete`) in hot paths; use `std::array` or fixed-size member arrays.
- Optional sensor handles: `esphome::optional<sensor::Sensor *>`.
- Protocol parser: **state-machine based**, non-blocking. Never call `delay()` or block `loop()`.
- Validate all sensor values before publishing: reject NaN, ±Inf, and physically impossible values.
- Rotation matrix computed in `setup()` or on config change — never recomputed inside `loop()`.
- Logging: `ESP_LOGD` / `ESP_LOGI` / `ESP_LOGW` / `ESP_LOGE` with a module-level `TAG`. Never `Serial.print`.
- Every public method and configuration parameter must have a `///` Doxygen comment.

### Python (`__init__.py`)

- All config keys defined as module-level `CONF_*` constants.
- Schema keys in `snake_case`.
- `DEPENDENCIES = ["uart"]` (plus any additional required ESPHome components).
- Entity sub-schemas use the canonical ESPHome helpers: `sensor.SENSOR_SCHEMA`, `binary_sensor.BINARY_SENSOR_SCHEMA`, `number.NUMBER_SCHEMA`.
- Boundary validator: custom `cv` function that rejects fewer than 3 vertices with a human-readable error message.
- Angle parameters (`yaw`, `pitch`, `roll`) validated for range `[-360, 360]` degrees.
- Distance/position parameters validated for non-negative or physically meaningful ranges.

### General

- No magic numbers; all protocol constants named and commented.
- Protocol constants (headers, function codes, data lengths, checksums) must match `docs/{radar_model}/` exactly.
- Checksum / CRC algorithm implemented precisely as specified in the protocol document.

---

## Reference Implementation: `components/r60abd1/`

When generating a new component, consult `components/r60abd1/` for:

| Aspect | Where to look |
|--------|---------------|
| UART frame state machine | `r60abd1.cpp` → `parse_byte()` |
| Coordinate transform + matrix pre-computation | `r60abd1.cpp` → `precompute_rotation_matrix()`, `transform_point()` |
| Boundary filter | `r60abd1.cpp` → `is_inside_boundary()` |
| CONFIG_SCHEMA layout + entity declaration style | `r60abd1/__init__.py` |
| Test YAML structure | `tests/r60abd1.yaml` |

Adapt the logic to the new radar's protocol; do not copy-paste blindly. Differences in frame header, length encoding, checksum algorithm, coordinate axes, or data fields must be reflected accurately from `docs/{radar_model}/`.

---

## Pre-`Developing` Checklist

Before updating `README.md` to `Developing`, verify all of the following:

- [ ] All three files present and complete under `components/{radar_model}/` (`__init__.py`, `.h`, `.cpp`).
- [ ] No separate `sensor.py` / `binary_sensor.py` / `number.py` files created.
- [ ] `__init__.py`: `DEPENDENCIES = ["uart"]` declared.
- [ ] `__init__.py`: CONFIG_SCHEMA includes `radar_x/y/z` and `radar_yaw/pitch/roll`.
- [ ] `__init__.py`: CONFIG_SCHEMA includes `boundary` list with ≥ 3-vertex validation.
- [ ] `__init__.py`: all entity sub-schemas declared using canonical ESPHome helpers.
- [ ] Radar-local coordinate axis convention documented in a header comment in `{radar_model}.h`.
- [ ] Rotation matrix uses ZYX Tait-Bryan convention; pre-computed in `setup()`.
- [ ] Boundary filter operates on room-frame coordinates (after transform).
- [ ] Processing order: parse → transform → filter → publish (strictly in this order).
- [ ] Protocol parser is state-machine based and non-blocking.
- [ ] All published values range-checked before `publish_state()`.
- [ ] `tests/{radar_model}.yaml` compiles cleanly with `esphome compile`.
- [ ] `README.md` status table updated to `Developing`.