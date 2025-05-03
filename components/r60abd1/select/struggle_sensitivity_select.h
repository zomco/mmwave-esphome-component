#pragma once

#include "esphome/components/select/select.h"
#include "../r60abd1.h"

namespace esphome {
namespace r60abd1 {

class StruggleSensitivitySelect : public select::Select, public Parented<R60ABD1Component> {
 public:
StruggleSensitivitySelect() = default;

 protected:
  void control(const std::string &value) override;
};

}  // namespace r60abd1
}  // namespace esphome