#include "r60abd1.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h" // Include for delay

namespace esphome
{
  namespace r60abd1
  {

    static const char *const TAG = "r60abd1";

    void R60ABD1::setup()
    {
      ESP_LOGCONFIG(TAG, "Setting up  R60ABD1...");
      this->buffer_.reserve(MAX_FRAME_LENGTH); // Pre-allocate buffer
      // Optional: Send initial query commands after a short delay to get initial states
      // set_timeout(5000, [this]() {
      //   ESP_LOGD(TAG, "Querying initial states...");
      //   this->send_command(CTRL_PRESENCE, CMD_PRESENCE_QUERY, {0x0F});
      //   this->send_command(CTRL_HEART_RATE, CMD_HEART_RATE_VALUE_QUERY, {0x0F});
      //   this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_VALUE_QUERY, {0x0F});
      //   this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_BED_STATUS_REPORT, {0x0F}); // Use query commands 0x81, 0x82 etc.
      // });
    }

    void R60ABD1::loop()
    {
      while (this->available())
      {
        uint8_t byte;
        this->read_byte(&byte);
        this->handle_byte_(byte);
      }
    }

    void R60ABD1::dump_config()
    {
      ESP_LOGCONFIG(TAG, " R60ABD1:");
      LOG_UART_DEVICE(this);
      // Log sensors linked via Python config generation
      LOG_BINARY_SENSOR("  ", "Presence Sensor", this->presence_sensor_);
      LOG_TEXT_SENSOR("  ", "Motion Text Sensor", this->motion_text_sensor_);
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

      LOG_TEXT_SENSOR("  ", "Respiration Info Sensor", this->respiration_info_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Sensor", this->respiration_rate_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 0 Sensor", this->respiration_rate_wave_0_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 1 Sensor", this->respiration_rate_wave_1_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 2 Sensor", this->respiration_rate_wave_2_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 3 Sensor", this->respiration_rate_wave_3_sensor_);
      LOG_SENSOR("  ", "Respiration Rate Wave 4 Sensor", this->respiration_rate_wave_4_sensor_);

      LOG_BINARY_SENSOR("  ", "Bed Status Sensor", this->bed_status_sensor_);
      LOG_TEXT_SENSOR("  ", "Sleep Stage Sensor", this->sleep_stage_sensor_);
    }

    // Checksum calculation for SENDING commands
    // buffer contains Header, Control, Command, Length, Data
    uint8_t R60ABD1::calculate_checksum_(const uint8_t *buffer, size_t length)
    {
      uint32_t sum = 0;
      for (size_t i = 0; i < length; ++i)
      {
        sum += buffer[i];
      }
      return (uint8_t)(sum & 0xFF); // Return lower 8 bits
    }

    int16_t R60ABD1::decode_16bit_signed_(uint8_t msb, uint8_t lsb)
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

    void R60ABD1::handle_byte_(uint8_t byte)
    {
      this->buffer_.push_back(byte);

      // Prevent buffer overflow - slightly safer check
      if (this->buffer_.size() > MAX_FRAME_LENGTH + 5)
      { // Allow some margin
        ESP_LOGW(TAG, "Buffer overflow risk, clearing buffer.");
        this->buffer_.clear();
        this->parse_state_ = ParseState::WAITING_HEADER_1; // Reset state
        return;                                            // Avoid processing the byte that caused the overflow potential
      }

      switch (this->parse_state_)
      {
      case ParseState::WAITING_HEADER_1:
        // If buffer not empty, it means we are in a recovery state, clear it first
        if (!this->buffer_.empty())
          this->buffer_.clear();
        if (byte == FRAME_HEADER[0])
        {
          this->parse_state_ = ParseState::WAITING_HEADER_2;
          this->buffer_.push_back(byte); // Start collecting frame
        }
        // If not header byte 1, just ignore and wait
        break;
      case ParseState::WAITING_HEADER_2:
        if (byte == FRAME_HEADER[1])
        {
          this->parse_state_ = ParseState::READING_CONTROL;
          // buffer_ already contains HEADER[0], now add HEADER[1]
        }
        else
        {
          ESP_LOGW(TAG, "Invalid header byte 2: 0x%02X, resetting.", byte);
          this->parse_state_ = ParseState::WAITING_HEADER_1;
          this->buffer_.clear(); // Discard partial frame
        }
        break;
      case ParseState::READING_CONTROL:
        this->parse_state_ = ParseState::READING_COMMAND;
        break;
      case ParseState::READING_COMMAND:
        this->parse_state_ = ParseState::READING_LENGTH_H;
        break;
      case ParseState::READING_LENGTH_H:
        this->data_length_ = (uint16_t)byte << 8;
        this->parse_state_ = ParseState::READING_LENGTH_L;
        break;
      case ParseState::READING_LENGTH_L:
        this->data_length_ |= byte;
        // Frame size = Header(2)+Ctrl(1)+Cmd(1)+Len(2)+Data(data_length_)+Chk(1)+Foot(2) = 9 + data_length_
        if (this->data_length_ > MAX_FRAME_LENGTH)
        { // Check if data length exceeds reasonable limit
          ESP_LOGW(TAG, "Declared data length too large: %d. Resetting.", this->data_length_);
          this->parse_state_ = ParseState::WAITING_HEADER_1;
          this->buffer_.clear();
          this->data_length_ = 0;
        }
        else if (this->data_length_ == 0)
        {
          this->parse_state_ = ParseState::READING_CHECKSUM; // No data bytes
        }
        else
        {
          this->parse_state_ = ParseState::READING_DATA;
        }
        break;
      case ParseState::READING_DATA:
        // Check if we have received all data bytes based on expected total frame size
        // Expected size = Header(2)+Ctrl(1)+Cmd(1)+Len(2)+Data(data_length_) = 6 + data_length_
        if (this->buffer_.size() == 6 + this->data_length_)
        {
          this->parse_state_ = ParseState::READING_CHECKSUM;
        }
        break;
      case ParseState::READING_CHECKSUM:
        // Checksum byte is now received, total size = 7 + data_length_
        this->expected_checksum_ = byte;
        this->parse_state_ = ParseState::READING_FOOTER_1;
        break;
      case ParseState::READING_FOOTER_1:
        // Footer byte 1 received, total size = 8 + data_length_
        if (byte == FRAME_FOOTER[0])
        {
          this->parse_state_ = ParseState::READING_FOOTER_2;
        }
        else
        {
          ESP_LOGW(TAG, "Invalid footer byte 1: 0x%02X. Frame discarded.", byte);
          this->parse_state_ = ParseState::WAITING_HEADER_1;
          this->buffer_.clear();
        }
        break;
      case ParseState::READING_FOOTER_2:
        // Footer byte 2 received, total size = 9 + data_length_
        if (byte == FRAME_FOOTER[1])
        {
          // Frame received completely, validate checksum
          // Checksum range: Header(2)+Ctrl(1)+Cmd(1)+Len(2)+Data(data_length_) = 6 + data_length_ bytes
          // The checksum byte itself is at index 6 + data_length_
          uint8_t calculated_checksum = this->calculate_checksum_(this->buffer_.data(), 6 + this->data_length_);

          if (calculated_checksum == this->expected_checksum_)
          {
            ESP_LOGV(TAG, "Received valid frame: %s", format_hex_pretty(this->buffer_).c_str());
            this->process_frame_(this->buffer_);
          }
          else
          {
            ESP_LOGW(TAG, "Checksum mismatch! Expected: 0x%02X, Calculated: 0x%02X. Frame: %s",
                     this->expected_checksum_, calculated_checksum, format_hex_pretty(this->buffer_).c_str());
          }
        }
        else
        {
          ESP_LOGW(TAG, "Invalid footer byte 2: 0x%02X. Frame discarded.", byte);
        }
        // Reset for next frame regardless of footer 2 validity or checksum result
        this->parse_state_ = ParseState::WAITING_HEADER_1;
        this->buffer_.clear(); // Clear buffer for the next frame
        this->data_length_ = 0;
        break;
      } // end switch

      // Safety break: If buffer grows too large unexpectedly, reset state.
      // This helps recover from potentially corrupted data streams.
      if (this->buffer_.size() > MAX_FRAME_LENGTH + 10 && this->parse_state_ != ParseState::WAITING_HEADER_1)
      {
        ESP_LOGW(TAG, "Buffer size excessive (%d), forcing reset.", this->buffer_.size());
        this->parse_state_ = ParseState::WAITING_HEADER_1;
        this->buffer_.clear();
      }
    }

    void R60ABD1::process_frame_(const std::vector<uint8_t> &frame)
    {
      // Frame structure: [Header(2)|Ctrl(1)|Cmd(1)|Len(2)|Data(N)|Chk(1)|Footer(2)]
      // Minimum size is 9 (N=0)
      if (frame.size() < 9)
      {
        ESP_LOGW(TAG, "Frame too short to process: %d bytes", frame.size());
        return;
      }

      uint8_t control_word = frame[2];
      uint8_t command_word = frame[3];
      uint16_t length = (uint16_t(frame[4]) << 8) | frame[5];
      const uint8_t *data = frame.data() + 6; // Pointer to the start of the data field

      // Verify actual data length matches header length field
      // Actual data bytes = Total size - Header(2) - Ctrl(1) - Cmd(1) - Len(2) - Chk(1) - Footer(2) = size - 9
      if (length != frame.size() - 9)
      {
        ESP_LOGW(TAG, "Frame data length mismatch! Header says %d, actual is %d. Frame: %s",
                 length, frame.size() - 9, format_hex_pretty(frame).c_str());
        // Decide whether to proceed or discard. Let's try proceeding but be cautious.
        // length = frame.size() - 9; // Use actual length? Or discard? Discarding is safer.
        return;
      }

      switch (control_word)
      {
      case CTRL_HEARTBEAT: // 0x01
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
          if (length > 0 && this->firmware_version_sensor_ != nullptr)
          {
            // Convert data bytes to string
            std::string version_str(reinterpret_cast<const char *>(data), length);
            ESP_LOGI(TAG, "Received Firmware version: %s", version_str.c_str());
            // Publish state only if it has changed
            if (this->firmware_version_sensor_->state != version_str)
            {
              this->firmware_version_sensor_->publish_state(version_str);
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

      case CTRL_WORK_STATUS: // 0x05
        if (command_word == 0x01 && length == 1 && data[0] == 0x0F)
        {
          ESP_LOGI(TAG, "Radar initialization complete.");
        }
        else if (command_word == 0x81 && length == 1)
        { // Reply to query
          ESP_LOGI(TAG, "Radar initialization status: %s", data[0] == 0x01 ? "Complete" : "Not Complete");
        }
        break;

      case CTRL_PRESENCE: // 0x80
        switch (command_word)
        {
        case CMD_PRESENCE_REPORT: // 0x01
          if (length == 1 && this->presence_sensor_ != nullptr)
          {
            bool present = (data[0] == 0x01);
            ESP_LOGD(TAG, "Presence report: %s", present ? "有人 (Present)" : "无人 (Absent)");
            this->presence_sensor_->publish_state(present);
          }
          break;
        case CMD_MOTION_REPORT: // 0x02
          if (length == 1)
          {
            int motion_state = data[0]; // 0: None, 1: Static, 2: Active
            ESP_LOGD(TAG, "Motion report: %d", motion_state);
            if (this->motion_sensor_ != nullptr)
            {
              this->motion_sensor_->publish_state(motion_state);
            }
            if (this->motion_text_sensor_ != nullptr)
            {
              switch (motion_state)
              {
              case 0:
                this->motion_text_sensor_->publish_state("无");
                break;
              case 1:
                this->motion_text_sensor_->publish_state("静止");
                break;
              case 2:
                this->motion_text_sensor_->publish_state("活跃");
                break;
              default:
                this->motion_text_sensor_->publish_state("未知");
                break;
              }
            }
          }
          break;
        case CMD_BODY_MVMT_REPORT: // 0x03
          if (length == 1 && this->body_movement_sensor_ != nullptr)
          {
            ESP_LOGD(TAG, "Body movement parameter: %d", data[0]);
            this->body_movement_sensor_->publish_state(data[0]);
          }
          break;
        case CMD_DISTANCE_REPORT: // 0x04
          if (length == 2 && this->distance_sensor_ != nullptr)
          {
            uint16_t distance = (uint16_t(data[0]) << 8) | data[1];
            ESP_LOGD(TAG, "Distance report: %d cm", distance);
            if (distance <= 65530)
            { // Assuming 65535 is invalid/max range
              this->distance_sensor_->publish_state(distance);
            }
            else
            {
              this->distance_sensor_->publish_state(NAN);
            }
          }
          break;
        case CMD_POSITION_REPORT: // 0x05
          if (length == 6)
          {
            int16_t x = decode_16bit_signed_(data[0], data[1]);
            int16_t y = decode_16bit_signed_(data[2], data[3]);
            int16_t z = decode_16bit_signed_(data[4], data[5]);
            ESP_LOGD(TAG, "Position report: X=%d cm, Y=%d cm, Z=%d cm", x, y, z);
            if (this->position_x_sensor_ != nullptr)
              this->position_x_sensor_->publish_state(x);
            if (this->position_y_sensor_ != nullptr)
              this->position_y_sensor_->publish_state(y);
            if (this->position_z_sensor_ != nullptr)
              this->position_z_sensor_->publish_state(z);
          }
          break;
        // Handle query responses if needed (e.g., 0x80, 0x81, 0x82...)
        case CMD_PRESENCE_SWITCH_QUERY: // 0x80 - Response to enable/disable query
          if (length == 1)
            ESP_LOGD(TAG, "Presence detection status query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          // Could potentially update an internal state variable here
          break;
        }
        break;

      case CTRL_HEART_RATE: // 0x85
        switch (command_word)
        {
        case CMD_HEART_RATE_VALUE_REPORT: // 0x02
          if (length == 1 && this->heart_rate_sensor_ != nullptr)
          {
            ESP_LOGD(TAG, "Heart rate report: %d bpm", data[0]);
            if (data[0] > 0 && data[0] < 200)
            { // More realistic check
              this->heart_rate_sensor_->publish_state(data[0]);
            }
            else
            {
              this->heart_rate_sensor_->publish_state(NAN);
            }
          }
          break;
        // Handle query responses if needed (e.g., 0x80, 0x8A)
        case CMD_HEART_RATE_SWITCH_QUERY: // 0x80
          if (length == 1)
            ESP_LOGD(TAG, "Heart rate detection status query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          break;
        case CMD_HEART_RATE_WAVE_SWITCH_QUERY: // 0x8A
          if (length == 1)
            ESP_LOGD(TAG, "Heart rate waveform reporting query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          break;
        case CMD_HEART_RATE_WAVEFORM_REPORT: // 0x05
          if (length == 6)
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
        }
        break;

      case CTRL_RESPIRATION: // 0x81
        switch (command_word)
        {
        case CMD_RESPIRATION_INFO_REPORT: // 0x01
          if (length == 1 && this->respiration_info_sensor_ != nullptr)
          {
            std::string info_str = "未知";
            switch (data[0])
            {
            case 0x01:
              info_str = "正常";
              break;
            case 0x02:
              info_str = "呼吸过高";
              break;
            case 0x03:
              info_str = "呼吸过低";
              break;
            case 0x04:
              info_str = "无";
              break;
            }
            ESP_LOGD(TAG, "Respiration info report: %s", info_str.c_str());
            this->respiration_info_sensor_->publish_state(info_str);
          }
          break;
        case CMD_RESPIRATION_VALUE_REPORT: // 0x02
          if (length == 1 && this->respiration_rate_sensor_ != nullptr)
          {
            ESP_LOGD(TAG, "Respiration rate report: %d rpm", data[0]);
            if (data[0] > 0 && data[0] < 50)
            { // More realistic check
              this->respiration_rate_sensor_->publish_state(data[0]);
            }
            else
            {
              this->respiration_rate_sensor_->publish_state(NAN);
            }
          }
          break;
        // Handle query responses if needed (e.g., 0x80, 0x8B, 0x8C)
        case CMD_RESPIRATION_SWITCH_QUERY: // 0x80
          if (length == 1)
            ESP_LOGD(TAG, "Respiration detection status query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          break;
        case CMD_RESPIRATION_LOW_THRESHOLD_QUERY: // 0x8B
          if (length == 1)
            ESP_LOGD(TAG, "Respiration low threshold query response: %d", data[0]);
          // Could potentially update number entity state if implemented
          break;
        case CMD_RESPIRATION_WAVE_SWITCH_QUERY: // 0x8C
          if (length == 1)
            ESP_LOGD(TAG, "Respiration waveform reporting query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          break;
        // Handle command confirmation replies
        case CMD_RESPIRATION_LOW_THRESHOLD_SET: // 0x0B
          if (length == 1)
            ESP_LOGD(TAG, "Respiration low threshold set confirmation: %d", data[0]);
          // Could potentially update number entity state if implemented
          break;
        case CMD_RESPIRATION_WAVEFORM_REPORT: // 0x05
          if (length == 6)
          {
            ESP_LOGD(TAG, "Respiration waveform report: %02X %02X %02X %02X %02X",
                     data[0], data[1], data[2], data[3], data[4]);
            if (this->respiration_wave_0_sensor_ != nullptr)
            {
              this->respiration_wave_0_sensor_->publish_state(data[0]);
            }
            if (this->respiration_wave_1_sensor_ != nullptr)
            {
              this->respiration_wave_1_sensor_->publish_state(data[1]);
            }
            if (this->respiration_wave_2_sensor_ != nullptr)
            {
              this->respiration_wave_2_sensor_->publish_state(data[2]);
            }
            if (this->respiration_wave_3_sensor_ != nullptr)
            {
              this->respiration_wave_3_sensor_->publish_state(data[3]);
            }
            if (this->respiration_wave_4_sensor_ != nullptr)
            {
              this->respiration_wave_4_sensor_->publish_state(data[4]);
            }
          }
        }
        break;

      case CTRL_SLEEP_MONITOR: // 0x84
        switch (command_word)
        {
        case CMD_SLEEP_BED_STATUS_REPORT: // 0x01
          if (length == 1 && this->bed_status_sensor_ != nullptr)
          {
            bool in_bed = (data[0] == 0x01);
            ESP_LOGD(TAG, "Bed status report: %s", in_bed ? "在床 (In Bed)" : (data[0] == 0x00 ? "离床 (Out of Bed)" : "无 (None)"));
            if (data[0] == 0x00 || data[0] == 0x01)
            {
              this->bed_status_sensor_->publish_state(in_bed);
            }
            else
            {
              this->bed_status_sensor_->publish_state(false); // Treat 'None' as out of bed
            }
          }
          break;
        case CMD_SLEEP_STAGE_REPORT: // 0x02
          if (length == 1 && this->sleep_stage_sensor_ != nullptr)
          {
            std::string stage_str = "未知";
            switch (data[0])
            {
            case 0x00:
              stage_str = "深睡";
              break;
            case 0x01:
              stage_str = "浅睡";
              break;
            case 0x02:
              stage_str = "清醒";
              break;
            case 0x03:
              stage_str = "无";
              break;
            }
            ESP_LOGD(TAG, "Sleep stage report: %s", stage_str.c_str());
            this->sleep_stage_sensor_->publish_state(stage_str);
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
        case CMD_SLEEP_SWITCH: // 0x00 - Reply to enable/disable sleep monitoring
          if (length == 1)
            ESP_LOGD(TAG, "Sleep monitoring set confirmation: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          break;
        case CMD_SLEEP_STRUGGLE_SWITCH_SET: // 0x13
          if (length == 1)
            ESP_LOGD(TAG, "Struggle detection set confirmation: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          break;
        case CMD_SLEEP_UNATTENDED_SWITCH_SET: // 0x14
          if (length == 1)
            ESP_LOGD(TAG, "Unattended detection set confirmation: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          break;
        case CMD_SLEEP_UNATTENDED_TIME_SET: // 0x15
          if (length == 1)
            ESP_LOGD(TAG, "Unattended time set confirmation: %d minutes", data[0]);
          // Could update number entity state if implemented
          break;
        case CMD_SLEEP_END_TIME_SET: // 0x16
          if (length == 1)
            ESP_LOGD(TAG, "Sleep end time set confirmation: %d minutes", data[0]);
          // Could update number entity state if implemented
          break;
        case CMD_SLEEP_STRUGGLE_SENSITIVITY_SET: // 0x1A
          if (length == 1)
            ESP_LOGD(TAG, "Struggle sensitivity set confirmation: %d (0:Low, 1:Medium, 2:High)", data[0]);
          // Could update select entity state if implemented
          break;
        // Handle query responses if needed (e.g., 0x80, 0x93, 0x94, 0x95, 0x96, 0x9A)
        case CMD_SLEEP_SWITCH_QUERY: // 0x80
          if (length == 1)
            ESP_LOGD(TAG, "Sleep monitoring status query response: %s", data[0] == 0x01 ? "Enabled" : "Disabled");
          break;
        case CMD_SLEEP_UNATTENDED_TIME_QUERY: // 0x95
          if (length == 1)
            ESP_LOGD(TAG, "Unattended time query response: %d minutes", data[0]);
          // Could update number entity state if implemented
          break;
        case CMD_SLEEP_STRUGGLE_SENSITIVITY_QUERY: // 0x9A
          if (length == 1)
            ESP_LOGD(TAG, "Struggle sensitivity query response: %d (0:Low, 1:Medium, 2:High)", data[0]);
          // Could update select entity state if implemented
          break;
        }
        break;

        // Add cases for other control words (Product Info, OTA, Radar Range) if needed

      default:
        ESP_LOGV(TAG, "Unhandled frame - Control: 0x%02X, Command: 0x%02X", control_word, command_word);
        break;
      }
    }

    // --- Send Command Implementation ---
    // Constructs and sends a command frame over UART
    void MicRadarR60ABD1::send_command(uint8_t control_word, uint8_t command_word, const std::vector<uint8_t> &data_payload)
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

      // Optional: Short delay after sending might be needed for some devices
      // delay(50);
    }

    // --- Control Methods Implementations ---
    // These methods are called by the code generated from switch.py, number.py, select.py

    void MicRadarR60ABD1::set_presence_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting presence detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_PRESENCE, CMD_PRESENCE_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void MicRadarR60ABD1::set_heart_rate_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting heart rate detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_HEART_RATE, CMD_HEART_RATE_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void MicRadarR60ABD1::set_respiration_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting respiration detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void MicRadarR60ABD1::set_sleep_monitoring(bool enable)
    {
      ESP_LOGD(TAG, "Setting sleep monitoring: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void MicRadarR60ABD1::set_heart_rate_waveform_reporting(bool enable)
    {
      ESP_LOGD(TAG, "Setting heart rate waveform reporting: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_HEART_RATE, CMD_HEART_RATE_WAVE_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void MicRadarR60ABD1::set_respiration_waveform_reporting(bool enable)
    {
      ESP_LOGD(TAG, "Setting respiration waveform reporting: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_WAVE_SWITCH, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void MicRadarR60ABD1::set_respiration_low_threshold(uint8_t threshold)
    {
      // Clamp value to protocol range [10, 20]
      uint8_t clamped_threshold = std::max((uint8_t)10, std::min(threshold, (uint8_t)20));
      if (clamped_threshold != threshold)
      {
        ESP_LOGW(TAG, "Respiration low threshold %d clamped to %d.", threshold, clamped_threshold);
      }
      ESP_LOGD(TAG, "Setting respiration low threshold: %d rpm", clamped_threshold);
      this->send_command(CTRL_RESPIRATION, CMD_RESPIRATION_LOW_THRESHOLD_SET, {clamped_threshold});
    }

    void MicRadarR60ABD1::set_struggle_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting struggle detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_STRUGGLE_SWITCH_SET, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void MicRadarR60ABD1::set_unattended_detection(bool enable)
    {
      ESP_LOGD(TAG, "Setting unattended detection: %s", enable ? "ON" : "OFF");
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_UNATTENDED_SWITCH_SET, {enable ? (uint8_t)0x01 : (uint8_t)0x00});
    }

    void MicRadarR60ABD1::set_unattended_time(uint8_t minutes)
    {
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

    void MicRadarR60ABD1::set_sleep_end_time(uint8_t minutes)
    {
      // Clamp value to protocol range [5, 120]
      uint8_t clamped_minutes = std::max((uint8_t)5, std::min(minutes, (uint8_t)120));
      if (clamped_minutes != minutes)
      {
        ESP_LOGW(TAG, "Sleep end time %d min clamped to %d min.", minutes, clamped_minutes);
      }
      ESP_LOGD(TAG, "Setting sleep end time: %d minutes", clamped_minutes);
      this->send_command(CTRL_SLEEP_MONITOR, CMD_SLEEP_END_TIME_SET, {clamped_minutes});
    }

    void MicRadarR60ABD1::set_struggle_sensitivity(uint8_t level)
    {
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
