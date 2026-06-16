#pragma once

#include <horizon/Widget.hpp>
#include <horizon/SolidObject.hpp>
#include <pluma/Layout/PageSize.hpp>
#include <string>

namespace pluma_app {

class MarginButton : public horizon::Widget {
public:
  MarginButton(const std::string& name, const std::string& dims, int t, int b, int l, int r);
  ~MarginButton() override = default;

  pluma::PageMargins get_margins() const;

protected:
  void draw(horizon::GraphicsContext& gc) override;

private:
  pluma::PageMargins m_margins;
  bool m_hovered = false;
  horizon::SolidObject m_bg;
};

} // namespace pluma_app
