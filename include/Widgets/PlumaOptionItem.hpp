#pragma once

#include <horizon/Widget.hpp>
#include <horizon/SolidObject.hpp>

namespace pluma_app {

class PlumaOptionItem : public horizon::Widget {
public:
  PlumaOptionItem();
  ~PlumaOptionItem() override = default;

protected:
  void draw(horizon::GraphicsContext& gc) override;

  bool m_hovered = false;
  horizon::SolidObject m_bg;
};

} // namespace pluma_app
