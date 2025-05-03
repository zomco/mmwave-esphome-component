#pragma once

#include "esphome/components/switch/switch.h"
#include "../r60abd1.h"

namespace esphome {
namespace r60abd1 {

class UnattendedDetectionSwitch : public switch_::Switch, public Parented<R60ABD1Component> {
 public:
  UnattendedDetectionSwitch() = default;

 protected:
  void write_state(bool state) override;
};

}  // namespace r60abd1
}  // namespace esphome