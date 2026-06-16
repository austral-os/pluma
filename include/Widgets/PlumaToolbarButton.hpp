#pragma once

#include <horizon/ToolbarButton.hpp>
#include <horizon/SolidObject.hpp>

namespace pluma_app {

class PlumaToolbarButton : public horizon::ToolbarButton {
public:
  PlumaToolbarButton(const std::string& text, const std::string& icon_name);
  ~PlumaToolbarButton() override = default;

  void set_active(bool active);
  bool is_active() const;

protected:
  void draw(horizon::GraphicsContext& gc) override;

private:
  bool m_hovered = false;
  bool m_active = false;
  horizon::SolidObject m_bg;
};

} // namespace pluma_app
