#pragma once

#include "esphome/components/select/select.h"
#include "../r60abd1.h"

namespace esphome {
namespace r60abd1 {

class UnattendedTimeNumber : public number::Number, public Parented<R60ABD1Component> {
 public:
  UnattendedTimeNumber() = default;

 protected:
  void control(float value) override;
};

}  // namespace r60abd1
}  // namespace esphome