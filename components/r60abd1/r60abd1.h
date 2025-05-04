#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"
#include <vector>

namespace esphome
{
  namespace r60abd1
  {

    // Define constants based on the protocol document
    const uint8_t FRAME_HEADER[] = {0x53, 0x59};
    const uint8_t FRAME_FOOTER[] = {0x54, 0x43};
    const uint8_t MAX_FRAME_LENGTH = 64; // Adjust based on longest possible frame + overhead

    // Control Words (协议文档 Page 8)
    const uint8_t CTRL_HEARTBEAT = 0x01;     // 心跳包标识
    const uint8_t CTRL_PRODUCT_INFO = 0x02;  // 产品信息
    const uint8_t CTRL_OTA = 0x03;           // OTA升级
    const uint8_t CTRL_WORK_STATUS = 0x05;   // 工作状态
    const uint8_t CTRL_RADAR_RANGE = 0x07;   // 雷达探测范围信息 (文档中仅列出位置越界，暂不常用)
    const uint8_t CTRL_PRESENCE = 0x80;      // 人体存在
    const uint8_t CTRL_RESPIRATION = 0x81;   // 呼吸检测
    const uint8_t CTRL_SLEEP_MONITOR = 0x84; // 睡眠监测
    const uint8_t CTRL_HEART_RATE = 0x85;    // 心率监测

    // Command Words for Product Info (0x02) (协议文档 Page 9-10)
    const uint8_t CMD_PRODUCT_MODEL_REPORT = 0x01;    // 产品型号上报
    const uint8_t CMD_PRODUCT_ID_REPORT = 0x02;       // 产品ID上报
    const uint8_t CMD_HARDWARE_MODEL_REPORT = 0x03;   // 硬件型号上报
    const uint8_t CMD_FIRMWARE_VERSION_REPORT = 0x04; // 固件版本上报 (主动上报)
    const uint8_t CMD_PRODUCT_MODEL_QUERY = 0xA1;     // 产品型号查询
    const uint8_t CMD_PRODUCT_ID_QUERY = 0xA2;        // 产品ID查询
    const uint8_t CMD_HARDWARE_MODEL_QUERY = 0xA3;    // 硬件型号查询
    const uint8_t CMD_FIRMWARE_VERSION_QUERY = 0xA4;  // 固件版本查询 (查询命令 & 回复命令)

    // Command Words for Presence (0x80) (协议文档 Page 10-11)
    const uint8_t CMD_PRESENCE_SWITCH = 0x00;       // 开关人体存在功能
    const uint8_t CMD_PRESENCE_REPORT = 0x01;       // 存在信息主动上报 (00:无人, 01:有人)
    const uint8_t CMD_MOTION_REPORT = 0x02;         // 运动信息主动上报 (00:无, 01:静止, 02:活跃)
    const uint8_t CMD_BODY_MVMT_REPORT = 0x03;      // 体动参数主动上报 (0-100)
    const uint8_t CMD_DISTANCE_REPORT = 0x04;       // 人体距离主动上报 (0-65535 cm)
    const uint8_t CMD_POSITION_REPORT = 0x05;       // 人体方位主动上报 (X, Y, Z)
    const uint8_t CMD_PRESENCE_SWITCH_QUERY = 0x80; // 查询人体存在开关状态
    const uint8_t CMD_PRESENCE_QUERY = 0x81;        // 查询存在信息
    const uint8_t CMD_MOTION_QUERY = 0x82;          // 查询运动信息
    const uint8_t CMD_BODY_MVMT_QUERY = 0x83;       // 查询体动参数
    const uint8_t CMD_DISTANCE_QUERY = 0x84;        // 查询人体距离
    const uint8_t CMD_POSITION_QUERY = 0x85;        // 查询人体方位信息

    // Command Words for Heart Rate (0x85) (协议文档 Page 12-13)
    const uint8_t CMD_HEART_RATE_SWITCH = 0x00;            // 开关心率监测功能
    const uint8_t CMD_HEART_RATE_VALUE_REPORT = 0x02;      // 心率数值上报 (60-120 bpm)
    const uint8_t CMD_HEART_RATE_WAVE_REPORT = 0x05;       // 心率波形上报 (5 bytes)
    const uint8_t CMD_HEART_RATE_WAVE_SWITCH = 0x0A;       // 心率波形上报开关设置
    const uint8_t CMD_HEART_RATE_SWITCH_QUERY = 0x80;      // 查询心率监测开关状态
    const uint8_t CMD_HEART_RATE_VALUE_QUERY = 0x82;       // 查询心率数值
    const uint8_t CMD_HEART_RATE_WAVE_QUERY = 0x85;        // 查询心率波形
    const uint8_t CMD_HEART_RATE_WAVE_SWITCH_QUERY = 0x8A; // 查询心率波形上报开关状态

    // Command Words for Respiration (0x81) (协议文档 Page 13-14)
    const uint8_t CMD_RESPIRATION_SWITCH = 0x00;              // 开关呼吸监测功能
    const uint8_t CMD_RESPIRATION_INFO_REPORT = 0x01;         // 呼吸信息上报 (01:正常, 02:过高, 03:过低, 04:无)
    const uint8_t CMD_RESPIRATION_VALUE_REPORT = 0x02;        // 呼吸数值上报 (0-35 rpm)
    const uint8_t CMD_RESPIRATION_WAVE_REPORT = 0x05;         // 呼吸波形上报 (5 bytes)
    const uint8_t CMD_RESPIRATION_LOW_THRESHOLD_SET = 0x0B;   // 低缓呼吸判读设置 (10-20)
    const uint8_t CMD_RESPIRATION_WAVE_SWITCH = 0x0C;         // 呼吸波形上报开关设置
    const uint8_t CMD_RESPIRATION_SWITCH_QUERY = 0x80;        // 查询呼吸监测开关状态
    const uint8_t CMD_RESPIRATION_INFO_QUERY = 0x81;          // 查询呼吸信息
    const uint8_t CMD_RESPIRATION_VALUE_QUERY = 0x82;         // 查询呼吸数值
    const uint8_t CMD_RESPIRATION_WAVE_QUERY = 0x85;          // 查询呼吸波形
    const uint8_t CMD_RESPIRATION_LOW_THRESHOLD_QUERY = 0x8B; // 查询低缓呼吸判读设置
    const uint8_t CMD_RESPIRATION_WAVE_SWITCH_QUERY = 0x8C;   // 查询呼吸波形上报开关状态

    // Command Words for Sleep Monitor (0x84) (协议文档 Page 14-18)
    const uint8_t CMD_SLEEP_SWITCH = 0x00;                // 开关睡眠监测功能
    const uint8_t CMD_SLEEP_BED_STATUS_REPORT = 0x01;     // 入床/离床状态上报 (00:离床, 01:入床, 02:无)
    const uint8_t CMD_SLEEP_STAGE_REPORT = 0x02;          // 睡眠状态上报 (00:深睡, 01:浅睡, 02:清醒, 03:无)
    const uint8_t CMD_SLEEP_AWAKE_DURATION_REPORT = 0x03; // 清醒时长 (2B, 分钟) - 由综合/分析报告提供
    const uint8_t CMD_SLEEP_LIGHT_DURATION_REPORT = 0x04; // 浅睡时长 (2B, 分钟) - 由综合/分析报告提供
    const uint8_t CMD_SLEEP_DEEP_DURATION_REPORT = 0x05;  // 深睡时长 (2B, 分钟) - 由综合/分析报告提供
    const uint8_t CMD_SLEEP_SCORE_REPORT = 0x06;          // 睡眠质量评分上报 (0-100)
    const uint8_t CMD_SLEEP_COMPOSITE_REPORT = 0x0C;      // 睡眠综合状态上报 (10分钟一次)
    const uint8_t CMD_SLEEP_ANALYSIS_REPORT = 0x0D;       // 睡眠质量分析上报 (结束时)
    const uint8_t CMD_SLEEP_ABNORMAL_REPORT = 0x0E;       // 睡眠异常上报 (不足/过长/异常无人)
    const uint8_t CMD_SLEEP_QUALITY_RATING_REPORT = 0x10; // 睡眠质量评级上报 (良好/一般/较差)
    const uint8_t CMD_SLEEP_STRUGGLE_REPORT = 0x11;       // 异常挣扎状态上报 (00:无, 01:正常, 02:异常)
    const uint8_t CMD_SLEEP_UNATTENDED_REPORT = 0x12;     // 无人计时状态上报 (00:无, 01:正常, 02:异常)
    // Control commands for Sleep Monitor
    const uint8_t CMD_SLEEP_STRUGGLE_SWITCH_SET = 0x13;      // 异常挣扎状态开关设置
    const uint8_t CMD_SLEEP_UNATTENDED_SWITCH_SET = 0x14;    // 无人计时功能开关设置
    const uint8_t CMD_SLEEP_UNATTENDED_TIME_SET = 0x15;      // 无人计时时长设置 (1B, 30-180 min)
    const uint8_t CMD_SLEEP_END_TIME_SET = 0x16;             // 睡眠截止时长设置 (1B, 5-120 min)
    const uint8_t CMD_SLEEP_STRUGGLE_SENSITIVITY_SET = 0x1A; // 挣扎状态判读设置 (1B, 0:Low, 1:Medium, 2:High)
    // Query commands for Sleep Monitor
    const uint8_t CMD_SLEEP_SWITCH_QUERY = 0x80;     // 查询睡眠监测开关
    const uint8_t CMD_SLEEP_BED_STATUS_QUERY = 0x81; // 查询入床/离床状态
    const uint8_t CMD_SLEEP_STAGE_QUERY = 0x82;      // 查询睡眠状态
    // ... (add other query commands if needed)
    const uint8_t CMD_SLEEP_STRUGGLE_SWITCH_QUERY = 0x93;      // 查询异常挣扎状态开关
    const uint8_t CMD_SLEEP_UNATTENDED_SWITCH_QUERY = 0x94;    // 查询无人计时功能开关
    const uint8_t CMD_SLEEP_UNATTENDED_TIME_QUERY = 0x95;      // 查询无人计时时长
    const uint8_t CMD_SLEEP_END_TIME_QUERY = 0x96;             // 查询睡眠截止时间
    const uint8_t CMD_SLEEP_STRUGGLE_SENSITIVITY_QUERY = 0x9A; // 查询挣扎状态判读


    // Main Hub Component
    class R60ABD1Component : public Component, public uart::UARTDevice
    {
    public:

      #ifdef USE_BINARY_SENSOR
      SUB_BINARY_SENSOR(presence);
      SUB_BINARY_SENSOR(bed_status);
      #endif

      #ifdef USE_TEXT_SENSOR
      SUB_TEXT_SENSOR(respiration_info);
      SUB_TEXT_SENSOR(motion_text);
      SUB_TEXT_SENSOR(sleep_stage);
      SUB_TEXT_SENSOR(firmware_version);
      SUB_TEXT_SENSOR(sleep_rating);
      #endif
      
      #ifdef USE_SENSOR
      SUB_SENSOR(motion);
      SUB_SENSOR(distance);
      SUB_SENSOR(body_movement);
      SUB_SENSOR(heart_rate);
      SUB_SENSOR(respiration_rate);
      SUB_SENSOR(sleep_score);
      SUB_SENSOR(position_x);
      SUB_SENSOR(position_y);
      SUB_SENSOR(position_z);
      SUB_SENSOR(heart_rate_wave_0);
      SUB_SENSOR(heart_rate_wave_1);
      SUB_SENSOR(heart_rate_wave_2);
      SUB_SENSOR(heart_rate_wave_3);
      SUB_SENSOR(heart_rate_wave_4);
      SUB_SENSOR(respiration_rate_wave_0);
      SUB_SENSOR(respiration_rate_wave_1);
      SUB_SENSOR(respiration_rate_wave_2);
      SUB_SENSOR(respiration_rate_wave_3);
      SUB_SENSOR(respiration_rate_wave_4);
      #endif

      // --- Component Overrides ---
      R60ABD1Component();
      void setup() override;
      void loop() override;
      void dump_config() override;
      float get_setup_priority() const override { return setup_priority::LATE; }

      // --- Public Methods for Control (called by switch/number/etc.) ---
      void send_command(uint8_t control_word, uint8_t command_word, const std::vector<uint8_t> &data_payload);

      // Control Methods Implementations (declared here, defined in .cpp)
      void set_presence_detection(bool enable);
      void set_heart_rate_detection(bool enable);
      void set_respiration_detection(bool enable);
      void set_sleep_monitoring(bool enable);
      void set_heart_rate_waveform_reporting(bool enable);
      void set_respiration_waveform_reporting(bool enable);
      void set_respiration_low_threshold(float threshold); // 10-20
      void set_struggle_detection(bool enable);
      void set_unattended_detection(bool enable);
      void set_unattended_time(float minutes);    // 30-180 min
      void set_sleep_end_time(float minutes);     // 5-120 min
      void set_struggle_sensitivity(uint8_t level); // 0:Low, 1:Medium, 2:High

    protected:
      // --- Data Processing ---
      void handle_byte_(uint8_t byte);
      void process_frame_(const std::vector<uint8_t> &frame);
      uint8_t calculate_checksum_(const uint8_t *buffer, size_t length); // Checksum for sending buffer
      int16_t decode_16bit_signed_(uint8_t msb, uint8_t lsb);            // Decode signed position data

      // --- Internal State ---
      std::vector<uint8_t> buffer_;
      enum class ParseState
      {
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
    };

  } // namespace r60abd1
} // namespace esphome
