#include "r60abd1.h"

#include <utility>
#include "esphome/core/log.h"
#include "esphome/core/hal.h" // Include for delay

#define highbyte(val) (uint8_t)((val) >> 8)
#define lowbyte(val) (uint8_t)((val) &0xff)

namespace esphome
{
  namespace r60abd1
  {

    static const char *const TAG = "r60abd1";

    R60ABD1Component::R60ABD1Component() {}

    void R60ABD1Component::setup()
    {
      ESP_LOGCONFIG(TAG, "Setting up R60ABD1...");
      this->buffer_.reserve(MAX_FRAME_LENGTH); // Pre-allocate buffer

      // Query firmware version after a delay (e.g., 5 seconds) to allow radar initialization
      this->set_timeout("initial_query", 5000, [this]()
                        {
        ESP_LOGD(TAG, "Querying initial states...");
        this->send_command(CTRL_PRODUCT_INFO, CMD_FIRMWARE_VERSION_QUERY, {0x0F});

        this->send_command(CTRL_PRESENCE, CMD_PRESENCE_SWITCH_QUERY, {0x0F}); 
        this->send_command(CTRL_HEART_RATE, CMD_HEART_RATE_SWITCH_QUERY, {0x0F});
        this->send_command(CTRL_HEART_RATE, CMD_HEART_RATE_WAVE_SWITCH_QUERY, {0x0F});
        this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_SWITCH_QUERY, {0x0F});
        this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_WAVE_SWITCH_QUERY, {0x0F});
        this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_SWITCH_QUERY, {0x0F});
        this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_STRUGGLE_SWITCH_QUERY, {0x0F});
        this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_UNATTENDED_SWITCH_QUERY, {0x0F});

        this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_LOW_THRESHOLD_QUERY, {0x0F});
        this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_UNATTENDED_TIME_QUERY, {0x0F});
        this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_END_TIME_QUERY, {0x0F});

        this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_STRUGGLE_SENSITIVITY_QUERY, {0x0F});
      });
    }

    void R60ABD1Component::loop()
    {
      while (this->available())
      {
        uint8_t byte;
        this->read_byte(&byte);
        this->handle_byte_(byte);
      }
    }

    void R60ABD1Component::dump_config()
    {
      ESP_LOGCONFIG(TAG, " R60ABD1:");

      #ifdef USE_BINARY_SENSOR
      LOG_BINARY_SENSOR("  ", "Presence Sensor", this->presence_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "Bed Status Sensor", this->bed_status_binary_sensor_);
      #endif

      #ifdef USE_SENSOR
      LOG_SENSOR("  ", "Motion Sensor", this->motion_sensor_);
      LOG_SENSOR("  ", "Body Movement Sensor", this->body_movement_sensor_);
      LOG_SENSOR("  ", "Distance Sensor", this->distance_sensor_);
      LOG_SENSOR("  ", "Position X Sensor", this->position_x_sensor_);
      LOG_SENSOR("  ", "Position Y Sensor", this->position_y_sensor_);
      LOG_SENSOR("  ", "Position Z Sensor", this->position_z_sensor_);
      LOG_SENSOR("  ", "Heart Rate Sensor", this->heart_rate_sensor_);
      LOG_SENSOR("  ", "Heart Rate Wave 0 Sensor", this->heart_rate_wave_0_sensor_);
      LOG_SENSOR("  ", "Heart Rate Wave 1 Sensor", this->heart_rate_wave_1_sensor_);
      LOG_SENSOR("  ", "Heart Rate Wave 2 Sensor", this->heart_rate_wave_2_sensor_);
      LOG_SENSOR("  ", "Heart Rate Wave 3 Sensor", this->heart_rate_wave_3_sensor_);
      LOG_SENSOR("  ", "Heart Rate Wave 4 Sensor", this->heart_rate_wave_4_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Sensor", this->respiration_rate_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 0 Sensor", this->respiration_rate_wave_0_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 1 Sensor", this->respiration_rate_wave_1_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 2 Sensor", this->respiration_rate_wave_2_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 3 Sensor", this->respiration_rate_wave_3_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 4 Sensor", this->respiration_rate_wave_4_sensor_);
      #endif

      #ifdef USE_TEXT_SENSOR
      LOG_TEXT_SENSOR("  ", "Respiration Info Sensor", this->respiration_info_text_sensor_);
      LOG_TEXT_SENSOR("  ", "Motion Text Sensor", this->motion_info_text_sensor_);
      LOG_TEXT_SENSOR("  ", "Sleep Stage Sensor", this->sleep_stage_text_sensor_);
      LOG_TEXT_SENSOR("  ", "Firmware Version Sensor", this->firmware_version_text_sensor_);
      LOG_TEXT_SENSOR("  ", "Sleep Rating Sensors", this->sleep_rating_text_sensor_);
      #endif
    }

    // Checksum calculation for SENDING commands
    // buffer contains Header, Control, Command, Length, Data
    uint8_t R60ABD1Component::calculate_checksum_(const uint8_t *buffer, size_t length)
    {
      uint32_t sum = 0;
      for (size_t i = 0; i < length; ++i)
      {
        sum += buffer[i];
      }
      return (uint8_t)(sum & 0xFF); // Return lower 8 bits
    }

    int16_t R60ABD1Component::decode_16bit_signed_(uint8_t msb, uint8_t lsb)
    {
      // Protocol specific signed format (Page 11/12 interpretation)
      // 16 bits, first bit is sign (0=positive, 1=negative), remaining 15 bits are value
      uint16_t raw_value = (uint16_t(msb) << 8) | lsb;
      int16_t value = raw_value & 0x7FFF; // Mask out the sign bit to get magnitude
      if (raw_value & 0x8000)
      { // Check if the sign bit is set (negative)
        value = -value;
      }
      return value;
    }

    void R60ABD1Component::handle_byte_(uint8_t byte)
    {
      this->buffer_.push_back(byte); // Add byte to the buffer

      // Basic buffer overflow protection
      if (this->buffer_.size() > MAX_FRAME_LENGTH + 10)
      { // Check against a slightly larger limit
        ESP_LOGW(TAG, "Buffer overflow risk detected (size: %d), clearing buffer.", this->buffer_.size());
        this->buffer_.clear();
        this->parse_state_ = ParseState::WAITING_HEADER_1; // Reset state machine
        return;
      }

      // State machine logic
      switch (this->parse_state_)
      {
      case ParseState::WAITING_HEADER_1:
        // If buffer is not empty here, it means we might be in recovery, clear it first.
        if (this->buffer_.size() > 1)
        {
          this->buffer_.erase(this->buffer_.begin(), this->buffer_.end() - 1); // Keep only the last byte
        }
        if (byte == FRAME_HEADER[0])
        {
          this->parse_state_ = ParseState::WAITING_HEADER_2; // Found first header byte
        }
        else
        {
          // Discard unexpected byte if buffer is growing too large without finding header
          if (this->buffer_.size() > 2)
          {                                             // Allow buffer to hold at least the potential header
            this->buffer_.erase(this->buffer_.begin()); // Remove oldest byte
          }
        }
        break;
      case ParseState::WAITING_HEADER_2:
        if (byte == FRAME_HEADER[1])
        {
          this->parse_state_ = ParseState::READING_CONTROL; // Found second header byte
        }
        else
        {
          ESP_LOGW(TAG, "Invalid header byte 2: 0x%02X, resetting.", byte);
          this->parse_state_ = ParseState::WAITING_HEADER_1; // Reset: sequence broken
          this->buffer_.clear();                             // Discard invalid frame start
        }
        break;
      case ParseState::READING_CONTROL: // Control Word (Byte 3)
        this->parse_state_ = ParseState::READING_COMMAND;
        break;
      case ParseState::READING_COMMAND: // Command Word (Byte 4)
        this->parse_state_ = ParseState::READING_LENGTH_H;
        break;
      case ParseState::READING_LENGTH_H: // Length High Byte (Byte 5)
        this->data_length_ = (uint16_t)byte << 8;
        this->parse_state_ = ParseState::READING_LENGTH_L;
        break;
      case ParseState::READING_LENGTH_L: // Length Low Byte (Byte 6)
        this->data_length_ |= byte;
        // Sanity check data length
        if (this->data_length_ > MAX_FRAME_LENGTH)
        {
          ESP_LOGW(TAG, "Declared data length (0x%04X) too large. Resetting.", this->data_length_);
          this->parse_state_ = ParseState::WAITING_HEADER_1;
          this->buffer_.clear();
          this->data_length_ = 0;
        }
        else if (this->data_length_ == 0)
        {
          this->parse_state_ = ParseState::READING_CHECKSUM; // No data bytes expected
        }
        else
        {
          this->parse_state_ = ParseState::READING_DATA; // Expect data bytes
        }
        break;
      case ParseState::READING_DATA: // Data Bytes (Byte 7 to 6+data_length)
        // Check if we have received all expected data bytes
        // Current buffer size should be Header(2)+Ctrl(1)+Cmd(1)+Len(2)+Data(data_length) = 6 + data_length
        if (this->buffer_.size() == 6 + this->data_length_)
        {
          this->parse_state_ = ParseState::READING_CHECKSUM; // All data bytes received
        }
        // If not all data bytes received yet, stay in this state
        break;
      case ParseState::READING_CHECKSUM: // Checksum Byte (Byte 7+data_length)
        this->expected_checksum_ = byte; // Store the checksum byte from the frame
        this->parse_state_ = ParseState::READING_FOOTER_1;
        break;
      case ParseState::READING_FOOTER_1: // Footer Byte 1 (Byte 8+data_length)
        if (byte == FRAME_FOOTER[0])
        {
          this->parse_state_ = ParseState::READING_FOOTER_2; // Found first footer byte
        }
        else
        {
          ESP_LOGW(TAG, "Invalid footer byte 1: 0x%02X. Frame discarded.", byte);
          this->parse_state_ = ParseState::WAITING_HEADER_1; // Reset: sequence broken
          this->buffer_.clear();                             // Discard invalid frame
        }
        break;
      case ParseState::READING_FOOTER_2: // Footer Byte 2 (Byte 9+data_length)
        if (byte == FRAME_FOOTER[1])
        {
          // Frame received completely, validate checksum
          // Checksum includes bytes from Header up to the end of Data field
          // Total bytes for checksum calculation = Header(2)+Ctrl(1)+Cmd(1)+Len(2)+Data(data_length) = 6 + data_length
          uint8_t calculated_checksum = this->calculate_checksum_(this->buffer_.data(), 6 + this->data_length_);

          if (calculated_checksum == this->expected_checksum_)
          {
            // Checksum matches, process the valid frame
            ESP_LOGV(TAG, "Received valid frame: %s", format_hex_pretty(this->buffer_).c_str());
            this->process_frame_(this->buffer_);
          }
          else
          {
            // Checksum mismatch, log error and discard frame
            ESP_LOGW(TAG, "Checksum mismatch! Expected: 0x%02X, Calculated: 0x%02X. Frame: %s",
                     this->expected_checksum_, calculated_checksum, format_hex_pretty(this->buffer_).c_str());
          }
        }
        else
        {
          // Second footer byte incorrect, discard frame
          ESP_LOGW(TAG, "Invalid footer byte 2: 0x%02X. Frame discarded.", byte);
        }
        // Reset state machine and buffer for the next frame, regardless of validity
        this->parse_state_ = ParseState::WAITING_HEADER_1;
        this->buffer_.clear();
        this->data_length_ = 0;
        break;
      } // end switch
    }

    void R60ABD1Component::process_frame_(const std::vector<uint8_t> &frame)
    {
      // Frame structure: [Header(2)|Ctrl(1)|Cmd(1)|Len(2)|Data(N)|Chk(1)|Footer(2)]
      // We already validated checksum and footer in handle_byte_

      uint8_t control_word = frame[2];
      uint8_t command_word = frame[3];
      uint16_t length = (uint16_t(frame[4]) << 8) | frame[5];
      const uint8_t *data = frame.data() + 6; // Pointer to the start of the data field

      // Double-check length consistency (although should be correct if checksum passed)
      if (length != frame.size() - 9)
      {
        ESP_LOGW(TAG, "Internal length mismatch during processing! Header says %d, actual is %d.",
                 length, frame.size() - 9);
        // This case should ideally not happen if checksum validation is robust
        return;
      }

      // Handle frame based on Control Word
      switch (control_word)
      {
      case CTRL_HEARTBEAT: // 0x01 - 心跳包
        if (command_word == 0x01 && length == 1 && data[0] == 0x0F)
        {
          ESP_LOGD(TAG, "Heartbeat received");
          // Could add logic here to track last heartbeat time for connection status
        }
        else if (command_word == 0x80 && length == 1)
        { // Reply to heartbeat query
          ESP_LOGD(TAG, "Heartbeat query reply: 0x%02X", data[0]);
        }
        break;

      case CTRL_PRODUCT_INFO: // 0x02 - 产品信息
        switch (command_word)
        {
        // Handle responses to queries or active reports
        case CMD_FIRMWARE_VERSION_REPORT: // 0x04 - Active report
        case CMD_FIRMWARE_VERSION_QUERY:  // 0xA4 - Response to query
          if (length > 0 && this->firmware_version_text_sensor_ != nullptr)
          {
            // Convert data bytes to string
            std::string version_str(reinterpret_cast<const char *>(data), length);
            ESP_LOGI(TAG, "Received Firmware version: %s", version_str.c_str());
            // Publish state only if it has changed
            if (this->firmware_version_text_sensor_->state != version_str)
            {
              this->firmware_version_text_sensor_->publish_state(version_str);
            }
          }
          break;
        // Add cases for other product info if sensors are defined for them
        case CMD_PRODUCT_MODEL_QUERY: // 0xA1 - Response to model query
          if (length > 0)
          {
            std::string model_str(reinterpret_cast<const char *>(data), length);
            ESP_LOGI(TAG, "Received Product Model: %s", model_str.c_str());
            // TODO: Add text sensor if needed
          }
          break;
          // ... handle Product ID (0xA2), Hardware Model (0xA3) responses similarly ...
        }
        break;

      case CTRL_WORK_STATUS: // 0x05 - 工作状态
        if (command_word == 0x01 && length == 1 && data[0] == 0x0F)
        {
          ESP_LOGI(TAG, "Radar initialization complete report received.");
        }
        else if (command_word == 0x81 && length == 1)
        { // Reply to initialization status query
          ESP_LOGI(TAG, "Radar initialization status query response: %s", data[0] == 0x01 ? "已完成 (Complete)" : "未完成 (Not Complete)");
        }
        break;

      case CTRL_PRESENCE: // 0x80 - 人体存在
        switch (command_word)
        {
        case CMD_PRESENCE_SWITCH: // 0x00
          if (length == 1 && this->presence_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Presence detection switch confirmation: %s", value ? "Enabled" : "Disabled");
            this->presence_detection_switch_->publish_state(value);
          }
          break;
        case CMD_PRESENCE_REPORT: // 0x01 - 存在信息主动上报
          if (length == 1 && this->presence_binary_sensor_ != nullptr)
          {
            bool state = (data[0] == 0x01); // 01:有人, 00:无人
            ESP_LOGD(TAG, "Presence report: %s", state ? "有人 (Present)" : "无人 (Absent)");
            // Publish state only if it has changed
            if (this->presence_binary_sensor_->state != state)
            {
              this->presence_binary_sensor_->publish_state(state);
            }
          }
          break;
        case CMD_MOTION_REPORT: // 0x02 - 运动信息主动上报
          if (length == 1)
          {
            int motion_code = data[0]; // 0:无, 1:静止, 2:活跃
            ESP_LOGD(TAG, "Motion report code: %d", motion_code);
            // Update numerical motion state sensor
            if (this->motion_sensor_ != nullptr)
            {
              // Publish state only if it has changed
              if (this->motion_sensor_->raw_state != motion_code)
              {
                this->motion_sensor_->publish_state(motion_code);
              }
            }
            // Update text motion state sensor
            if (this->motion_info_text_sensor_ != nullptr)
            {
              std::string state_str = MOTION_INFO_INT_TO_ENUM.at(motion_code);
              // Publish state only if it has changed
              if (this->motion_info_text_sensor_->state != state_str)
              {
                this->motion_info_text_sensor_->publish_state(state_str.c_str());
              }
            }
          }
          break;
        case CMD_BODY_MVMT_REPORT: // 0x03 - 体动参数主动上报
          if (length == 1 && this->body_movement_sensor_ != nullptr)
          {
            float value = data[0]; // 0-100
            ESP_LOGD(TAG, "Body movement parameter: %.0f", value);
            // Publish state only if it has changed
            if (this->body_movement_sensor_->raw_state != value)
            {
              this->body_movement_sensor_->publish_state(value);
            }
          }
          break;
        case CMD_DISTANCE_REPORT: // 0x04 - 人体距离主动上报
          if (length == 2 && this->distance_sensor_ != nullptr)
          {
            uint16_t distance_cm = (uint16_t(data[0]) << 8) | data[1];
            ESP_LOGD(TAG, "Distance report: %d cm", distance_cm);
            float value = NAN; // Default to Not a Number
            if (distance_cm <= 65530)
            { // Assuming 65535 etc. are invalid/out of range
              value = distance_cm;
            }
            // Publish state only if it has changed (handle NAN comparison carefully)
            if (std::isnan(this->distance_sensor_->raw_state) ? !std::isnan(value) : (this->distance_sensor_->raw_state != value))
            {
              this->distance_sensor_->publish_state(value);
            }
          }
          break;
        case CMD_POSITION_REPORT: // 0x05 - 人体方位主动上报
          if (length == 6)
          {
            int16_t x = decode_16bit_signed_(data[0], data[1]);
            int16_t y = decode_16bit_signed_(data[2], data[3]);
            int16_t z = decode_16bit_signed_(data[4], data[5]);
            ESP_LOGD(TAG, "Position report: X=%d cm, Y=%d cm, Z=%d cm", x, y, z);
            // Publish states only if they have changed
            if (this->position_x_sensor_ != nullptr && this->position_x_sensor_->raw_state != x)
              this->position_x_sensor_->publish_state(x);
            if (this->position_y_sensor_ != nullptr && this->position_y_sensor_->raw_state != y)
              this->position_y_sensor_->publish_state(y);
            if (this->position_z_sensor_ != nullptr && this->position_z_sensor_->raw_state != z)
              this->position_z_sensor_->publish_state(z);
          }
          break;
        // Handle query responses if needed (e.g., 0x80, 0x81, 0x82...)
        case CMD_PRESENCE_SWITCH_QUERY: // 0x80 - Response to enable/disable query
          if (length == 1 && this->presence_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Presence detection status query response: %s", value ? "Enabled" : "Disabled");
            this->presence_detection_switch_->publish_state(value);
          }
          break;
        }
        break;

      case CTRL_HEART_RATE: // 0x85 - 心率监测
        switch (command_word)
        {
        case CMD_HEART_RATE_SWITCH: // 0x00
          if (length == 1 && this->heart_rate_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Heart rate detection switch confirmation: %s", value ? "Enabled" : "Disabled");
            this->heart_rate_detection_switch_->publish_state(value);
          }
          break;
        case CMD_HEART_RATE_VALUE_REPORT: // 0x02 - 心率数值上报
          if (length == 1 && this->heart_rate_sensor_ != nullptr)
          {
            uint8_t hr_value = data[0];
            ESP_LOGD(TAG, "Heart rate report: %d bpm", hr_value);
            float value = NAN;
            // Check if value is within plausible range (e.g., 30-200)
            if (hr_value >= 30 && hr_value <= 200)
            {
              value = hr_value;
            }
            // Publish state only if it has changed
            if (std::isnan(this->heart_rate_sensor_->raw_state) ? !std::isnan(value) : (this->heart_rate_sensor_->raw_state != value))
            {
              this->heart_rate_sensor_->publish_state(value);
            }
          }
          break;
        case CMD_HEART_RATE_WAVE_REPORT: // 0x05
          ESP_LOGV(TAG, "Heart rate waveform data received (ignored).");
          if (length == 5)
          {
            ESP_LOGD(TAG, "Heart rate waveform report: %02X %02X %02X %02X %02X",
                     data[0], data[1], data[2], data[3], data[4]);
            if (this->heart_rate_wave_0_sensor_ != nullptr)
            {
              this->heart_rate_wave_0_sensor_->publish_state(data[0]);
            }
            if (this->heart_rate_wave_1_sensor_ != nullptr)
            {
              this->heart_rate_wave_1_sensor_->publish_state(data[1]);
            }
            if (this->heart_rate_wave_2_sensor_ != nullptr)
            {
              this->heart_rate_wave_2_sensor_->publish_state(data[2]);
            }
            if (this->heart_rate_wave_3_sensor_ != nullptr)
            {
              this->heart_rate_wave_3_sensor_->publish_state(data[3]);
            }
            if (this->heart_rate_wave_4_sensor_ != nullptr)
            {
              this->heart_rate_wave_4_sensor_->publish_state(data[4]);
            }
          }
          break;
        case CMD_HEART_RATE_WAVE_SWITCH: // 0x0A
          if (length == 1 && this->heart_rate_waveform_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Heart rate waveform switch confirmation: %s", value ? "Enabled" : "Disabled");
            this->heart_rate_waveform_switch_->publish_state(value);
          }
          break;
        // Handle query responses if needed (e.g., 0x80, 0x8A)
        case CMD_HEART_RATE_SWITCH_QUERY: // 0x80
          if (length == 1 && this->heart_rate_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Heart rate detection status query response: %s", value ? "Enabled" : "Disabled");
            this->heart_rate_detection_switch_->publish_state(value);
          }
          break;
        case CMD_HEART_RATE_WAVE_SWITCH_QUERY: // 0x8A
          if (length == 1 && this->heart_rate_waveform_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Heart rate waveform reporting query response: %s", value ? "Enabled" : "Disabled");
            this->heart_rate_waveform_switch_->publish_state(value);
          }
          break;
        }
        break;

      case CTRL_RESPIRATION: // 0x81 - 呼吸检测
        switch (command_word)
        {
        case CMD_RESPIRATION_SWITCH: // 0x80
          if (length == 1 && this->respiration_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Respiration detection switch confirmation: %s", value ? "Enabled" : "Disabled");
            this->respiration_detection_switch_->publish_state(value);
          }
          break;
        case CMD_RESPIRATION_INFO_REPORT: // 0x01 - 呼吸信息上报
          if (length == 1 && this->respiration_info_text_sensor_ != nullptr)
          {
            std::string info_str = RESPIRATION_INFO_INT_TO_ENUM.at(data[0]);
            ESP_LOGD(TAG, "Respiration info report: %s", info_str.c_str());
            // Publish state only if it has changed
            if (this->respiration_info_text_sensor_->state != info_str)
            {
              this->respiration_info_text_sensor_->publish_state(info_str.c_str());
            }
          }
          break;
        case CMD_RESPIRATION_VALUE_REPORT: // 0x02 - 呼吸数值上报
          if (length == 1 && this->respiration_rate_sensor_ != nullptr)
          {
            uint8_t resp_value = data[0];
            ESP_LOGD(TAG, "Respiration rate report: %d rpm", resp_value);
            float value = NAN;
            // Check if value is within plausible range (e.g., 1-50)
            if (resp_value >= 1 && resp_value <= 50)
            {
              value = resp_value;
            }
            // Publish state only if it has changed
            if (std::isnan(this->respiration_rate_sensor_->raw_state) ? !std::isnan(value) : (this->respiration_rate_sensor_->raw_state != value))
            {
              this->respiration_rate_sensor_->publish_state(value);
            }
          }
          break;
        case CMD_RESPIRATION_WAVE_REPORT: // 0x05
          ESP_LOGV(TAG, "Respiration waveform data received (ignored).");
          if (length == 5)
          {
            ESP_LOGD(TAG, "Respiration waveform report: %02X %02X %02X %02X %02X",
                     data[0], data[1], data[2], data[3], data[4]);
            if (this->respiration_rate_wave_0_sensor_ != nullptr)
            {
              this->respiration_rate_wave_0_sensor_->publish_state(data[0]);
            }
            if (this->respiration_rate_wave_1_sensor_ != nullptr)
            {
              this->respiration_rate_wave_1_sensor_->publish_state(data[1]);
            }
            if (this->respiration_rate_wave_2_sensor_ != nullptr)
            {
              this->respiration_rate_wave_2_sensor_->publish_state(data[2]);
            }
            if (this->respiration_rate_wave_3_sensor_ != nullptr)
            {
              this->respiration_rate_wave_3_sensor_->publish_state(data[3]);
            }
            if (this->respiration_rate_wave_4_sensor_ != nullptr)
            {
              this->respiration_rate_wave_4_sensor_->publish_state(data[4]);
            }
          }
          break;
        case CMD_RESPIRATION_LOW_THRESHOLD_SET: // 0x0B
          if (length == 1 && this->respiration_low_threshold_number_ != nullptr) {
            float value = data[0];
            ESP_LOGD(TAG, "Respiration low threshold set confirmation: %d", data[0]);
            this->respiration_low_threshold_number_->publish_state(value);
          }
          break;
        case CMD_RESPIRATION_WAVE_SWITCH: // 0x0C
          if (length == 1 && this->respiration_waveform_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Respiration waveform reporting switch confirmation: %s", value ? "Enabled" : "Disabled");
            this->respiration_waveform_switch_->publish_state(value);
          }
          break;
        // Handle query responses if needed (e.g., 0x80, 0x8B, 0x8C)
        case CMD_RESPIRATION_SWITCH_QUERY: // 0x80
          if (length == 1 && this->respiration_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Respiration detection status query response: %s", value ? "Enabled" : "Disabled");
            this->respiration_detection_switch_->publish_state(value);
          }
          break;
        case CMD_RESPIRATION_LOW_THRESHOLD_QUERY: // 0x8B
          if (length == 1 && this->respiration_low_threshold_number_ != nullptr) {
            float value = data[0];
            ESP_LOGD(TAG, "Respiration low threshold query response: %d", data[0]);
            this->respiration_low_threshold_number_->publish_state(value);
          }
          break;
        case CMD_RESPIRATION_WAVE_SWITCH_QUERY: // 0x8C
          if (length == 1 && this->respiration_waveform_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Respiration waveform reporting query response: %s", value ? "Enabled" : "Disabled");
            this->respiration_waveform_switch_->publish_state(value);
          }
          break;
        }
        break;

      case CTRL_SLEEP_MONITOR: // 0x84 - 睡眠监测
        switch (command_word)
        {
        // Handle status reports
        case CMD_SLEEP_SWITCH: // 0x00 - Reply to enable/disable sleep monitoring
          if (length == 1 && this->sleep_monitoring_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Sleep monitoring set confirmation: %s", value ? "Enabled" : "Disabled");
            this->sleep_monitoring_switch_->publish_state(value);
          }
          break;
        case CMD_SLEEP_BED_STATUS_REPORT: // 0x01 - 入床/离床状态
          if (length == 1 && this->bed_status_binary_sensor_ != nullptr)
          {
            // 0x00:离床, 0x01:入床, 0x02:无(实时探测模式下显示)
            bool state = (data[0] == 0x01); // Treat 0x00 and 0x02 as 'false' (not in bed)
            ESP_LOGD(TAG, "Bed status report: %s (Raw: 0x%02X)", state ? "在床 (In Bed)" : "离床/无 (Out/None)", data[0]);
            // Publish state only if it has changed
            if (this->bed_status_binary_sensor_->state != state)
            {
              this->bed_status_binary_sensor_->publish_state(state);
            }
          }
          break;
        case CMD_SLEEP_STAGE_REPORT: // 0x02 - 睡眠状态
          if (length == 1 && this->sleep_stage_text_sensor_ != nullptr)
          {
            // 0x00:深睡, 0x01:浅睡, 0x02:清醒, 0x03:无(离床时/实时探测模式下上报)
            std::string stage_str = SLEEP_STAGE_INT_TO_ENUM.at(data[0]);
            ESP_LOGD(TAG, "Sleep stage report: %s", stage_str.c_str());
            // Publish state only if it has changed
            if (this->sleep_stage_text_sensor_->state != stage_str)
            {
              this->sleep_stage_text_sensor_->publish_state(stage_str.c_str());
            }
          }
          break;
        case CMD_SLEEP_SCORE_REPORT: // 0x06 - 睡眠质量评分
          if (length == 1 && this->sleep_score_sensor_ != nullptr)
          {
            float score = data[0]; // 0-100
            ESP_LOGD(TAG, "Sleep score report: %.0f", score);
            // Publish state only if it has changed
            if (this->sleep_score_sensor_->raw_state != score)
            {
              this->sleep_score_sensor_->publish_state(score);
            }
          }
          break;
        // Add handling for composite (0x0C), analysis (0x0D), abnormal (0x0E),
        // quality rating (0x10), struggle (0x11), unattended (0x12) reports if needed.
        // These often contain multiple data points and would require more complex parsing
        // and potentially multiple sensors or attributes.
        case CMD_SLEEP_COMPOSITE_REPORT: // 0x0C
          ESP_LOGD(TAG, "Sleep composite report received (parsing not implemented). Length: %d", length);
          // Example: Parse data[0] (presence), data[1] (sleep state), etc.
          break;
        case CMD_SLEEP_ANALYSIS_REPORT: // 0x0D
          ESP_LOGD(TAG, "Sleep analysis report received (parsing not implemented). Length: %d", length);
          break;

        // Handle command confirmation replies (replies to set commands)
        case CMD_SLEEP_STRUGGLE_SWITCH_SET: // 0x13
          if (length == 1 && this->struggle_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Struggle detection set confirmation: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
            this->struggle_detection_switch_->publish_state(value);
          }
          break;
        case CMD_SLEEP_UNATTENDED_SWITCH_SET: // 0x14
          if (length == 1 && this->unattended_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Unattended detection set confirmation: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
            this->unattended_detection_switch_->publish_state(value);
          }
          break;
        case CMD_SLEEP_UNATTENDED_TIME_SET: // 0x15
          if (length == 1 && this->unattended_time_number_ != nullptr) {
            float value = data[0];
            ESP_LOGD(TAG, "Unattended time set confirmation: %d minutes", data[0]);
            this->unattended_time_number_->publish_state(value);
          }
          break;
        case CMD_SLEEP_END_TIME_SET: // 0x16
          if (length == 1 && this->sleep_end_time_number_ != nullptr) {
            float value = data[0];
            ESP_LOGD(TAG, "Sleep end time set confirmation: %d minutes", data[0]);
            this->sleep_end_time_number_->publish_state(value);
          }
          break;
        case CMD_SLEEP_STRUGGLE_SENSITIVITY_SET: // 0x1A
          if (length == 1 && this->struggle_sensitivity_select_ != nullptr) {
            std::string value = STRUGGLE_SENSITIVITY_INT_TO_ENUM.at(data[0]);
            ESP_LOGD(TAG, "Struggle sensitivity set confirmation: %d (0:Low, 1:Medium, 2:High)", data[0]);
            this->struggle_sensitivity_select_->publish_state(value);
          }
          break;
        // Handle query responses if needed (e.g., 0x80, 0x93, 0x94, 0x95, 0x96, 0x9A)
        case CMD_SLEEP_SWITCH_QUERY: // 0x80
          if (length == 1 && this->sleep_monitoring_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Sleep monitoring status query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
            this->sleep_monitoring_switch_->publish_state(value);
          }
          break;
        case CMD_SLEEP_STRUGGLE_SWITCH_QUERY: // 0x93
          if (length == 1 && this->struggle_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Struggle detection query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
            this->struggle_detection_switch_->publish_state(value);
          }
          break;
        case CMD_SLEEP_UNATTENDED_SWITCH_QUERY: // 0x94
          if (length == 1 && this->unattended_detection_switch_ != nullptr) {
            bool value = data[0] == 0x01;
            ESP_LOGD(TAG, "Unattended detection query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
            this->unattended_detection_switch_->publish_state(value);
          }
          break;
        case CMD_SLEEP_UNATTENDED_TIME_QUERY: // 0x95
          if (length == 1 && this->unattended_time_number_ != nullptr) {
            float value = data[0];
            ESP_LOGD(TAG, "Unattended time query response: %d minutes", data[0]);
            this->unattended_time_number_->publish_state(value);
          }
          break;
        case CMD_SLEEP_END_TIME_QUERY: // 0x96
          if (length == 1 && this->sleep_end_time_number_ != nullptr) {
            float value = data[0];
            ESP_LOGD(TAG, "Sleep end time query response: %d minutes", data[0]);
            this->sleep_end_time_number_->publish_state(value);
          }
          break;
        case CMD_SLEEP_STRUGGLE_SENSITIVITY_QUERY: // 0x9A
          if (length == 1 && this->struggle_sensitivity_select_ != nullptr) {
            std::string value = STRUGGLE_SENSITIVITY_INT_TO_ENUM.at(data[0]);
            ESP_LOGD(TAG, "Struggle sensitivity query response: %d (0:Low, 1:Medium, 2:High)", data[0]);
            this->struggle_sensitivity_select_->publish_state(value);
          }
          break;
        }
        break;

        // Handle other Control Words if necessary
        // case CTRL_RADAR_RANGE: // 0x07
        //    ...
        //    break;

      default:
        ESP_LOGV(TAG, "Ignoring frame with unhandled Control Word: 0x%02X, Command: 0x%02X", control_word, command_word);
        break;
      }
    }

    // --- Send Command Implementation ---
    // Constructs and sends a command frame over UART
    void R60ABD1Component::send_command(uint8_t control_word, uint8_t command_word, const std::vector<uint8_t> &data_payload)
    {
      std::vector<uint8_t> frame;
      size_t payload_size = data_payload.size();
      frame.reserve(9 + payload_size); // Pre-allocate memory for efficiency

      // 1. Frame Header
      frame.push_back(FRAME_HEADER[0]);
      frame.push_back(FRAME_HEADER[1]);
      // 2. Control Word
      frame.push_back(control_word);
      // 3. Command Word
      frame.push_back(command_word);
      // 4. Length (2 bytes, Big Endian)
      frame.push_back((payload_size >> 8) & 0xFF); // Length High Byte
      frame.push_back(payload_size & 0xFF);        // Length Low Byte
      // 5. Data payload
      frame.insert(frame.end(), data_payload.begin(), data_payload.end());

      // 6. Calculate Checksum (on Header through Data)
      uint8_t checksum = this->calculate_checksum_(frame.data(), frame.size());
      frame.push_back(checksum); // Append checksum

      // 7. Frame Footer
      frame.push_back(FRAME_FOOTER[0]);
      frame.push_back(FRAME_FOOTER[1]);

      // Send the constructed frame over UART
      this->write_array(frame);

      // Log the sent command for debugging
      ESP_LOGD(TAG, "Sent frame: Ctrl=0x%02X, Cmd=0x%02X, Data=[%s]",
               control_word, command_word, format_hex_pretty(data_payload).c_str());

      ESP_LOGD(TAG, "Debug frame: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
      frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9]);
      // Optional: Short delay after sending might be needed for some devices
      // delay(50);
    }

    // --- Control Methods Implementations ---
    // These methods are called by the code generated from switch.py, number.py, select.py

    void R60ABD1Component::set_presence_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting presence detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_PRESENCE, CMD_PRESENCE_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void R60ABD1Component::set_heart_rate_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting heart rate detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_HEART_RATE, CMD_HEART_RATE_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void R60ABD1Component::set_respiration_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting respiration detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void R60ABD1Component::set_sleep_monitoring(bool enable)
    {
      ESP_LOGD(TAG, "Setting sleep monitoring: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void R60ABD1Component::set_heart_rate_waveform(bool enable)
    {
      ESP_LOGD(TAG, "Setting heart rate waveform reporting: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_HEART_RATE, CMD_HEART_RATE_WAVE_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void R60ABD1Component::set_respiration_waveform(bool enable)
    {
      ESP_LOGD(TAG, "Setting respiration waveform reporting: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_WAVE_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void R60ABD1Component::set_respiration_low_threshold(float value)
    {
      uint8_t threshold = (uint8_t)value;
      // Clamp value to protocol range [10, 20]
      uint8_t clamped_threshold = std::max((uint8_t)10, std::min(threshold, (uint8_t)20));
      if (clamped_threshold != threshold)
      {
        ESP_LOGW(TAG, "Respiration low threshold %d clamped to %d.", threshold, clamped_threshold);
      }
      ESP_LOGD(TAG, "Setting respiration low threshold: %d rpm", clamped_threshold);
      this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_LOW_THRESHOLD_SET, {clamped_threshold});
    }

    void R60ABD1Component::set_struggle_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting struggle detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_STRUGGLE_SWITCH_SET, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void R60ABD1Component::set_unattended_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting unattended detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_UNATTENDED_SWITCH_SET, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void R60ABD1Component::set_unattended_time(float value)
    {
      uint8_t minutes = (uint8_t)value;
      // Clamp value to protocol range [30, 180]
      uint8_t clamped_minutes = std::max((uint8_t)30, std::min(minutes, (uint8_t)180));
      if (clamped_minutes != minutes)
      {
        ESP_LOGW(TAG, "Unattended time %d min clamped to %d min.", minutes, clamped_minutes);
      }
      // Protocol implies steps of 10 min, but we send the clamped value. Device might round it.
      ESP_LOGD(TAG, "Setting unattended time: %d minutes", clamped_minutes);
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_UNATTENDED_TIME_SET, {clamped_minutes});
    }

    void R60ABD1Component::set_sleep_end_time(float value)
    {
      uint8_t minutes = (uint8_t)value;
      // Clamp value to protocol range [5, 120]
      uint8_t clamped_minutes = std::max((uint8_t)5, std::min(minutes, (uint8_t)120));
      if (clamped_minutes != minutes)
      {
        ESP_LOGW(TAG, "Sleep end time %d min clamped to %d min.", minutes, clamped_minutes);
      }
      ESP_LOGD(TAG, "Setting sleep end time: %d minutes", clamped_minutes);
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_END_TIME_SET, {clamped_minutes});
    }

    void R60ABD1Component::set_struggle_sensitivity(const std::string &value)
    {
      uint8_t level = (uint8_t)STRUGGLE_SENSITIVITY_ENUM_TO_INT.at(value);
      // Clamp value to protocol range [0, 2]
      uint8_t clamped_level = std::min(level, (uint8_t)2);
      if (clamped_level != level)
      {
        ESP_LOGW(TAG, "Struggle sensitivity level %d clamped to %d.", level, clamped_level);
      }
      ESP_LOGD(TAG, "Setting struggle sensitivity level: %d (0:Low, 1:Medium, 2:High)", clamped_level);
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_STRUGGLE_SENSITIVITY_SET, {clamped_level});
    }

  } // namespace r60abd1
} // namespace esphome
