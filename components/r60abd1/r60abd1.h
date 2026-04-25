#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "r60abd1_transform.h"

#include <vector>
#include <cstdint>

namespace esphome {
namespace r60abd1 {

// ─── 帧解析状态机 ────────────────────────────────────────────────────────────

enum class ParseState : uint8_t {
  IDLE,
  HDR2,      // 已收到 0x53，等待 0x59
  CTRL,
  CMD,
  LEN_H,
  LEN_L,
  DATA,
  CHECKSUM,
  TAIL1,     // 等待 0x54
  TAIL2,     // 等待 0x43
};

// 帧头帧尾常量
static constexpr uint8_t FRAME_HDR1 = 0x53;
static constexpr uint8_t FRAME_HDR2 = 0x59;
static constexpr uint8_t FRAME_TAIL1 = 0x54;
static constexpr uint8_t FRAME_TAIL2 = 0x43;
static constexpr size_t  MAX_DATA_LEN = 32;  // 最大数据段长度

// ─── 控制字枚举 ──────────────────────────────────────────────────────────────

static constexpr uint8_t CTRL_HEARTBEAT   = 0x01;
static constexpr uint8_t CTRL_PRODUCT     = 0x02;
static constexpr uint8_t CTRL_WORK_STATE  = 0x05;
static constexpr uint8_t CTRL_RADAR_RANGE = 0x07;
static constexpr uint8_t CTRL_PRESENCE    = 0x80;
static constexpr uint8_t CTRL_BREATH      = 0x81;
static constexpr uint8_t CTRL_SLEEP       = 0x84;
static constexpr uint8_t CTRL_HEART       = 0x85;

// ─── R60ABD1Component ────────────────────────────────────────────────────────

class R60ABD1Component : public Component, public uart::UARTDevice {
 public:
  // ── 生命周期 ──────────────────────────────────────────────────────────────
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // ── 校准参数 Setters ──────────────────────────────────────────────────────
  void set_radar_x(float v)      { cal_.radar_x      = v; }
  void set_radar_y(float v)      { cal_.radar_y      = v; }
  void set_radar_height(float v) { cal_.radar_height = v; }
  void set_yaw(float v)          { cal_.yaw   = v; }
  void set_pitch(float v)        { cal_.pitch = v; }
  void set_roll(float v)         { cal_.roll  = v; }
  void set_polygon(std::vector<::r60abd1::Vec2> poly) { cal_.polygon = std::move(poly); }

  // ── Sensor Setters ────────────────────────────────────────────────────────
  // 存在与运动
  void set_presence_sensor(binary_sensor::BinarySensor *s)    { presence_sensor_     = s; }
  void set_motion_sensor(sensor::Sensor *s)                   { motion_sensor_       = s; }
  void set_body_movement_sensor(sensor::Sensor *s)            { body_movement_       = s; }
  void set_body_distance_sensor(sensor::Sensor *s)            { body_distance_       = s; }
  // 原始坐标（雷达局部坐标系）
  void set_raw_x_sensor(sensor::Sensor *s)                    { raw_x_               = s; }
  void set_raw_y_sensor(sensor::Sensor *s)                    { raw_y_               = s; }
  void set_raw_z_sensor(sensor::Sensor *s)                    { raw_z_               = s; }
  // 变换后坐标（房间坐标系）
  void set_room_x_sensor(sensor::Sensor *s)                   { room_x_              = s; }
  void set_room_y_sensor(sensor::Sensor *s)                   { room_y_              = s; }
  void set_height_floor_sensor(sensor::Sensor *s)             { height_floor_        = s; }
  void set_in_boundary_sensor(binary_sensor::BinarySensor *s) { in_boundary_sensor_  = s; }
  // 呼吸
  void set_breath_value_sensor(sensor::Sensor *s)             { breath_value_        = s; }
  void set_breath_state_sensor(text_sensor::TextSensor *s)    { breath_state_        = s; }
  // 心率
  void set_heart_rate_sensor(sensor::Sensor *s)               { heart_rate_          = s; }
  // 睡眠
  void set_in_bed_sensor(binary_sensor::BinarySensor *s)      { in_bed_sensor_       = s; }
  void set_sleep_state_sensor(text_sensor::TextSensor *s)     { sleep_state_         = s; }
  void set_awake_duration_sensor(sensor::Sensor *s)           { awake_duration_      = s; }
  void set_light_sleep_duration_sensor(sensor::Sensor *s)     { light_sleep_dur_     = s; }
  void set_deep_sleep_duration_sensor(sensor::Sensor *s)      { deep_sleep_dur_      = s; }
  void set_sleep_score_sensor(sensor::Sensor *s)              { sleep_score_         = s; }
  void set_sleep_quality_sensor(text_sensor::TextSensor *s)   { sleep_quality_       = s; }

  // ── 命令发送（公开供 lambda 调用） ────────────────────────────────────────
  void send_cmd(uint8_t ctrl, uint8_t cmd, const uint8_t *data, uint16_t len);
  void enable_presence()      { const uint8_t d = 0x01; send_cmd(CTRL_PRESENCE, 0x00, &d, 1); }
  void enable_breath()        { const uint8_t d = 0x01; send_cmd(CTRL_BREATH,   0x00, &d, 1); }
  void enable_heart_rate()    { const uint8_t d = 0x01; send_cmd(CTRL_HEART,    0x00, &d, 1); }
  void enable_sleep()         { const uint8_t d = 0x01; send_cmd(CTRL_SLEEP,    0x00, &d, 1); }
  void reset_module()         { const uint8_t d = 0x0F; send_cmd(CTRL_HEARTBEAT, 0x02, &d, 1); }
  void query_init_state()     { const uint8_t d = 0x0F; send_cmd(CTRL_WORK_STATE, 0x81, &d, 1); }

 protected:
  // ── 帧解析 ────────────────────────────────────────────────────────────────
  void process_byte_(uint8_t byte);
  void dispatch_frame_();

  // ── 各控制字处理 ──────────────────────────────────────────────────────────
  void handle_presence_frame_();
  void handle_breath_frame_();
  void handle_heart_frame_();
  void handle_sleep_frame_();
  void handle_work_state_frame_();

  // ── 坐标变换 & 发布 ───────────────────────────────────────────────────────
  void publish_position_(int16_t rx, int16_t ry, int16_t rz);

  // ── 内部状态 ──────────────────────────────────────────────────────────────
  ParseState parse_state_{ParseState::IDLE};
  uint8_t    ctrl_{0}, cmd_{0};
  uint16_t   data_len_{0}, data_idx_{0};
  uint8_t    rx_buf_[MAX_DATA_LEN]{};
  uint8_t    checksum_accum_{0};   // 滚动累加（帧头到数据）

  bool       initialized_{false};
  uint32_t   last_rx_ms_{0};

  ::r60abd1::CalibrationParams cal_;

  // Sensors（全部可为 nullptr）
  binary_sensor::BinarySensor *presence_sensor_    = nullptr;
  sensor::Sensor               *motion_sensor_     = nullptr;
  sensor::Sensor               *body_movement_     = nullptr;
  sensor::Sensor               *body_distance_     = nullptr;
  sensor::Sensor               *raw_x_             = nullptr;
  sensor::Sensor               *raw_y_             = nullptr;
  sensor::Sensor               *raw_z_             = nullptr;
  sensor::Sensor               *room_x_            = nullptr;
  sensor::Sensor               *room_y_            = nullptr;
  sensor::Sensor               *height_floor_      = nullptr;
  binary_sensor::BinarySensor  *in_boundary_sensor_= nullptr;
  sensor::Sensor               *breath_value_      = nullptr;
  text_sensor::TextSensor      *breath_state_      = nullptr;
  sensor::Sensor               *heart_rate_        = nullptr;
  binary_sensor::BinarySensor  *in_bed_sensor_     = nullptr;
  text_sensor::TextSensor      *sleep_state_       = nullptr;
  sensor::Sensor               *awake_duration_    = nullptr;
  sensor::Sensor               *light_sleep_dur_   = nullptr;
  sensor::Sensor               *deep_sleep_dur_    = nullptr;
  sensor::Sensor               *sleep_score_       = nullptr;
  text_sensor::TextSensor      *sleep_quality_     = nullptr;
};

}  // namespace r60abd1
}  // namespace esphome