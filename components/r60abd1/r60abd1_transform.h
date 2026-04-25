#pragma once
/**
 * r60abd1_transform.h  —  三维坐标变换 + 房间边界过滤
 *
 * 支持 pitch / roll / yaw 全姿态旋转，适用于非水平安装场景。
 *
 * 坐标系约定（手册 v2.5 勘误后）:
 *   雷达局部坐标系:
 *     Y 轴 — 雷达正前方
 *     X 轴 — 右正左负
 *     Z 轴 — 雷达朝向法线（垂直天线面，向外为正）
 *   编码: bit15=符号(0=正,1=负), bit14-0=15位幅值, 单位 cm
 *
 * 旋转顺序: Rz(yaw) · Rx(pitch) · Ry(roll)
 *   yaw   — 水平朝向，雷达 Y 轴相对房间 Y 轴顺时针为正（度）
 *   pitch — 俯仰，雷达向前倾为正（度）
 *   roll  — 横滚，雷达向右倾为正（度）
 *
 * 无 IMU 时：pitch/roll 手动填写或设为 0（水平安装误差 <5° 可忽略）
 * 有 IMU 时：从传感器实体读取后写入校准参数
 */

#include <cmath>
#include <cstdint>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace r60abd1 {

// ─── 坐标解码 ────────────────────────────────────────────────────────────────

/** 解码 R60ABD1 坐标字节对：bit15=符号, bit14-0=15位幅值（cm） */
inline int16_t decode_coord(uint8_t high, uint8_t low) {
  uint16_t raw   = (static_cast<uint16_t>(high) << 8) | low;
  bool negative  = (raw >> 15) & 1u;
  uint16_t value = raw & 0x7FFFu;
  return negative ? -static_cast<int16_t>(value)
                  :  static_cast<int16_t>(value);
}

// ─── 数据结构 ────────────────────────────────────────────────────────────────

struct Vec2 { float x = 0.f, y = 0.f; };
struct Vec3 { float x = 0.f, y = 0.f, z = 0.f; };

struct CalibrationParams {
  // 安装位置（房间坐标系，cm）
  float radar_x  = 0.f;
  float radar_y  = 0.f;
  float radar_height = 220.f;  // 距地面高度（cm）

  // 安装姿态（度）
  float yaw   = 0.f;   // 水平朝向偏差，顺时针为正
  float pitch = 0.f;   // 俯仰，向前倾为正
  float roll  = 0.f;   // 横滚，向右倾为正

  // 房间边界多边形（房间坐标系，cm；size<3 时不过滤）
  std::vector<Vec2> polygon;
};

struct TransformResult {
  Vec2  room;              // 变换后的房间水平坐标（cm）
  float height_floor_cm;  // 目标距地面高度（cm）
  bool  in_boundary;      // 是否在多边形边界内
};

// ─── 旋转矩阵构建 ────────────────────────────────────────────────────────────

/**
 * 构建 3×3 旋转矩阵  R = Rz(yaw) · Rx(pitch) · Ry(roll)
 *
 * Rz(γ):  绕 Z 轴旋转（水平朝向）
 * Rx(α):  绕 X 轴旋转（俯仰）
 * Ry(β):  绕 Y 轴旋转（横滚）
 *
 *         ┌ cγcβ+sγsαsβ   sγcα   −cγsβ+sγsαcβ ┐
 * R(γαβ) = │−sγcβ+cγsαsβ   cγcα    sγsβ+cγsαcβ │
 *         └ cαsβ          −sα      cαcβ          ┘
 */
struct Mat3 {
  float m[3][3] = {};
  float operator()(int r, int c) const { return m[r][c]; }
};

inline Mat3 build_rotation(float yaw_deg, float pitch_deg, float roll_deg) {
  const float deg2rad = static_cast<float>(M_PI) / 180.f;
  const float γ = yaw_deg   * deg2rad;
  const float α = pitch_deg * deg2rad;
  const float β = roll_deg  * deg2rad;

  const float sγ = sinf(γ), cγ = cosf(γ);
  const float sα = sinf(α), cα = cosf(α);
  const float sβ = sinf(β), cβ = cosf(β);

  Mat3 R;
  R.m[0][0] =  cγ*cβ + sγ*sα*sβ;
  R.m[0][1] =  sγ*cα;
  R.m[0][2] = -cγ*sβ + sγ*sα*cβ;

  R.m[1][0] = -sγ*cβ + cγ*sα*sβ;
  R.m[1][1] =  cγ*cα;
  R.m[1][2] =  sγ*sβ + cγ*sα*cβ;

  R.m[2][0] =  cα*sβ;
  R.m[2][1] = -sα;
  R.m[2][2] =  cα*cβ;
  return R;
}

// ─── 边界过滤 ────────────────────────────────────────────────────────────────

/** 射线法：点 (px,py) 是否在多边形内；顶点不足 3 个时始终返回 true */
inline bool point_in_polygon(float px, float py, const std::vector<Vec2>& poly) {
  const size_t n = poly.size();
  if (n < 3) return true;
  bool inside = false;
  for (size_t i = 0, j = n - 1; i < n; j = i++) {
    const float xi = poly[i].x, yi = poly[i].y;
    const float xj = poly[j].x, yj = poly[j].y;
    if (((yi > py) != (yj > py)) &&
        (px < (xj - xi) * (py - yi) / (yj - yi) + xi))
      inside = !inside;
  }
  return inside;
}

// ─── 主变换函数 ──────────────────────────────────────────────────────────────

/**
 * 将雷达局部坐标 (rx, ry, rz) 变换到房间坐标系
 *
 * 完整流程:
 *   1. 构建旋转矩阵 R = Rz(yaw)·Rx(pitch)·Ry(roll)
 *   2. world_vec = R * [rx, ry, rz]ᵀ
 *   3. room_x = radar_x + world_vec.x
 *      room_y = radar_y + world_vec.y
 *      room_z = radar_height - world_vec.z   （转为距地高度）
 *   4. 射线法判断是否在边界内
 *
 * 精度说明（R60ABD1 角度分辨率约 ±5°）:
 *   水平安装（pitch≈roll≈0）时，3m 处横向误差天花板约 ±25cm
 *   非水平安装时，正确填写 pitch/roll 可消除系统性几何误差
 */
inline TransformResult apply(float rx_cm, float ry_cm, float rz_cm,
                             const CalibrationParams& cal) {
  const Mat3 R = build_rotation(cal.yaw, cal.pitch, cal.roll);

  // 旋转到世界坐标（相对于雷达安装点）
  const float wx = R.m[0][0]*rx_cm + R.m[0][1]*ry_cm + R.m[0][2]*rz_cm;
  const float wy = R.m[1][0]*rx_cm + R.m[1][1]*ry_cm + R.m[1][2]*rz_cm;
  const float wz = R.m[2][0]*rx_cm + R.m[2][1]*ry_cm + R.m[2][2]*rz_cm;

  TransformResult res;
  res.room.x         = cal.radar_x + wx;
  res.room.y         = cal.radar_y + wy;
  res.height_floor_cm = cal.radar_height - wz;   // wz 向下为正时减去
  res.in_boundary    = point_in_polygon(res.room.x, res.room.y, cal.polygon);
  return res;
}

// ─── 校准工具函数 ────────────────────────────────────────────────────────────

/** 两点几何法计算偏航角（度），用于校准流程 */
inline float compute_yaw_from_two_points(
    Vec2 map_a, Vec2 map_b,   // 参考点在房间坐标系的实际位置
    Vec2 det_a, Vec2 det_b)   // 雷达检测到的原始坐标（未变换）
{
  const float am = atan2f(map_b.y - map_a.y, map_b.x - map_a.x);
  const float ad = atan2f(det_b.y - det_a.y, det_b.x - det_a.x);
  float yaw = (am - ad) * 180.f / static_cast<float>(M_PI);
  while (yaw >  180.f) yaw -= 360.f;
  while (yaw < -180.f) yaw += 360.f;
  return yaw;
}

/** 计算两点校准残差（cm），< 5cm 良好，< 15cm 可接受 */
inline float calibration_residual(
    Vec2 map_a, Vec2 map_b, Vec2 det_a, Vec2 det_b,
    const CalibrationParams& cal)
{
  const auto ta = apply(det_a.x, det_a.y, 0.f, cal);
  const auto tb = apply(det_b.x, det_b.y, 0.f, cal);
  const float ra = hypotf(ta.room.x - map_a.x, ta.room.y - map_a.y);
  const float rb = hypotf(tb.room.x - map_b.x, tb.room.y - map_b.y);
  return (ra + rb) / 2.f;
}

} // namespace r60abd1