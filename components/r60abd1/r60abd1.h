#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include <vector>

namespace esphome {
namespace r60abd1 {

// Define constants based on the protocol document
const uint8_t FRAME_HEADER[] = {0x53, 0x59};
const uint8_t FRAME_FOOTER[] = {0x54, 0x43};
const uint8_t MAX_FRAME_LENGTH = 64; // Adjust based on longest possible frame + overhead

// Control Words (Page 8)
const uint8_t CTRL_HEARTBEAT = 0x01;
const uint8_t CTRL_PRODUCT_INFO = 0x02;
const uint8_t CTRL_OTA = 0x03;
const uint8_t CTRL_WORK_STATUS = 0x05;
const uint8_t CTRL_RADAR_RANGE = 0x07;
const uint8_t CTRL_PRESENCE = 0x80;
const uint8_t CTRL_RESPIRATION = 0x81;
const uint8_t CTRL_SLEEP_MONITOR = 0x84;
const uint8_t CTRL_HEART_RATE = 0x85;

// Command Words for Presence (0x80) (Page 10-11)
const uint8_t CMD_PRESENCE_SWITCH = 0x00;
const uint8_t CMD_PRESENCE_REPORT = 0x01;
const uint8_t CMD_MOTION_REPORT = 0x02;
const uint8_t CMD_BODY_MVMT_REPORT = 0x03;
const uint8_t CMD_DISTANCE_REPORT = 0x04;
const uint8_t CMD_POSITION_REPORT = 0x05;
const uint8_t CMD_PRESENCE_SWITCH_QUERY = 0x80;
const uint8_t CMD_PRESENCE_QUERY = 0x81;
const uint8_t CMD_MOTION_QUERY = 0x82;
const uint8_t CMD_BODY_MVMT_QUERY = 0x83;
const uint8_t CMD_DISTANCE_QUERY = 0x84;
const uint8_t CMD_POSITION_QUERY = 0x85;

// Command Words for Heart Rate (0x85) (Page 12-13)
const uint8_t CMD_HEART_RATE_SWITCH = 0x00;
const uint8_t CMD_HEART_RATE_VALUE_REPORT = 0x02;
const uint8_t CMD_HEART_RATE_WAVE_REPORT = 0x05;
const uint8_t CMD_HEART_RATE_WAVE_SWITCH = 0x0A;
const uint8_t CMD_HEART_RATE_SWITCH_QUERY = 0x80;
const uint8_t CMD_HEART_RATE_VALUE_QUERY = 0x82;
const uint8_t CMD_HEART_RATE_WAVE_QUERY = 0x85;
const uint8_t CMD_HEART_RATE_WAVE_SWITCH_QUERY = 0x8A;

// Command Words for Respiration (0x81) (Page 13-14)
const uint8_t CMD_RESPIRATION_SWITCH = 0x00;
const uint8_t CMD_RESPIRATION_INFO_REPORT = 0x01;
const uint8_t CMD_RESPIRATION_VALUE_REPORT = 0x02;
const uint8_t CMD_RESPIRATION_WAVE_REPORT = 0x05;
const uint8_t CMD_RESPIRATION_LOW_THRESHOLD_SET = 0x0B;
const uint8_t CMD_RESPIRATION_WAVE_SWITCH = 0x0C;
const uint8_t CMD_RESPIRATION_SWITCH_QUERY = 0x80;
const uint8_t CMD_RESPIRATION_INFO_QUERY = 0x81;
const uint8_t CMD_RESPIRATION_VALUE_QUERY = 0x82;
const uint8_t CMD_RESPIRATION_WAVE_QUERY = 0x85;
const uint8_t CMD_RESPIRATION_LOW_THRESHOLD_QUERY = 0x8B;
const uint8_t CMD_RESPIRATION_WAVE_SWITCH_QUERY = 0x8C;

// Command Words for Sleep Monitor (0x84) (Page 14-18) - Partial list
const uint8_t CMD_SLEEP_SWITCH = 0x00;
const uint8_t CMD_SLEEP_BED_STATUS_REPORT = 0x01; // In/Out bed
const uint8_t CMD_SLEEP_STAGE_REPORT = 0x02;      // Deep/Light/Awake
const uint8_t CMD_SLEEP_SCORE_REPORT = 0x06;
const uint8_t CMD_SLEEP_COMPOSITE_REPORT = 0x0C;
const uint8_t CMD_SLEEP_ANALYSIS_REPORT = 0x0D;
const uint8_t CMD_SLEEP_ABNORMAL_REPORT = 0x0E;
const uint8_t CMD_SLEEP_QUALITY_RATING_REPORT = 0x10;
const uint8_t CMD_SLEEP_STRUGGLE_REPORT = 0x11;
const uint8_t CMD_SLEEP_UNATTENDED_REPORT = 0x12;
// Add more command words as needed for switches, numbers, etc.
const uint8_t CMD_SLEEP_STRUGGLE_SWITCH_SET = 0x13;
const uint8_t CMD_SLEEP_UNATTENDED_SWITCH_SET = 0x14;
const uint8_t CMD_SLEEP_UNATTENDED_TIME_SET = 0x15;
const uint8_t CMD_SLEEP_END_TIME_SET = 0x16;
const uint8_t CMD_SLEEP_STRUGGLE_SENSITIVITY_SET = 0x1A;


// Forward declaration
class R60ABD1;

// Base class for sensors associated with the hub
class MicRadarR60ABD1Listener {
 public:
  virtual void on_radar_data(const std::vector<uint8_t> &data) = 0;
};


// Main Hub Component
class R60ABD1 : public Component, public uart::UARTDevice {
 public:
  // --- Sensor Configuration Methods (called by generated code) ---
  void register_listener(MicRadarR60ABD1Listener *listener) { this->listeners_.push_back(listener); }

  void set_presence_sensor(binary_sensor::BinarySensor *sensor) { presence_sensor_ = sensor; }
  void set_motion_sensor(sensor::Sensor *sensor) { motion_sensor_ = sensor; }
  void set_motion_text_sensor(text_sensor::TextSensor *sensor) { motion_text_sensor_ = sensor; }
  void set_distance_sensor(sensor::Sensor *sensor) { distance_sensor_ = sensor; }
  void set_body_movement_sensor(sensor::Sensor *sensor) { body_movement_sensor_ = sensor; }
  void set_heart_rate_sensor(sensor::Sensor *sensor) { heart_rate_sensor_ = sensor; }
  void set_respiration_rate_sensor(sensor::Sensor *sensor) { respiration_rate_sensor_ = sensor; }
  void set_respiration_info_sensor(text_sensor::TextSensor *sensor) { respiration_info_sensor_ = sensor; }
  void set_bed_status_sensor(binary_sensor::BinarySensor *sensor) { bed_status_sensor_ = sensor; }
  void set_sleep_stage_sensor(text_sensor::TextSensor *sensor) { sleep_stage_sensor_ = sensor; }
  void set_sleep_score_sensor(sensor::Sensor *sensor) { sleep_score_sensor_ = sensor; }
  void set_position_x_sensor(sensor::Sensor *sensor) { position_x_sensor_ = sensor; }
  void set_position_y_sensor(sensor::Sensor *sensor) { position_y_sensor_ = sensor; }
  void set_position_z_sensor(sensor::Sensor *sensor) { position_z_sensor_ = sensor; }

  // --- Component Overrides ---
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  // --- Public Methods for Control (called by switch/number/etc.) ---
  void send_command(uint8_t control_word, uint8_t command_word, const std::vector<uint8_t>& data_payload);
  // Example control methods - add more as needed
  // void set_presence_detection(bool enable);
  // void set_heart_rate_detection(bool enable);
  // void set_respiration_detection(bool enable);
  // void set_sleep_monitoring(bool enable);
  // void set_unattended_time(uint8_t minutes); // For number entity
  // void set_struggle_sensitivity(uint8_t level); // For select entity

 protected:
  // --- Data Processing ---
  void handle_byte_(uint8_t byte);
  void process_frame_(const std::vector<uint8_t> &frame);
  uint8_t calculate_checksum_(const uint8_t* buffer, size_t length); // Modified for sending
  int16_t decode_16bit_signed_(uint8_t msb, uint8_t lsb);

  // --- Sensor Pointers ---
  // Keep these as before, they will be set by the generated code via the public setters
  binary_sensor::BinarySensor *presence_sensor_{nullptr};
  sensor::Sensor *motion_sensor_{nullptr};
  text_sensor::TextSensor *motion_text_sensor_{nullptr};
  sensor::Sensor *distance_sensor_{nullptr};
  sensor::Sensor *body_movement_sensor_{nullptr};
  sensor::Sensor *heart_rate_sensor_{nullptr};
  sensor::Sensor *respiration_rate_sensor_{nullptr};
  text_sensor::TextSensor *respiration_info_sensor_{nullptr};
  binary_sensor::BinarySensor *bed_status_sensor_{nullptr};
  text_sensor::TextSensor *sleep_stage_sensor_{nullptr};
  sensor::Sensor *sleep_score_sensor_{nullptr};
  sensor::Sensor *position_x_sensor_{nullptr};
  sensor::Sensor *position_y_sensor_{nullptr};
  sensor::Sensor *position_z_sensor_{nullptr};

  // --- Internal State ---
  std::vector<uint8_t> buffer_;
  enum class ParseState {
    WAITING_HEADER_1,
    WAITING_HEADER_2,
    READING_CONTROL,
    READING_COMMAND,
    READING_LENGTH_H,
    READING_LENGTH_L,
    READING_DATA,
    READING_CHECKSUM,
    READING_FOOTER_1,
    READING_FOOTER_2
  } parse_state_ = ParseState::WAITING_HEADER_1;
  uint16_t data_length_ = 0;
  uint8_t expected_checksum_ = 0;

  // For potential sub-components (not used in this basic refactor)
  std::vector<MicRadarR60ABD1Listener *> listeners_;
};

}  // namespace r60abd1
}  // namespace esphome
