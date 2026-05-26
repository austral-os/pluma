#pragma once

#include <horizon/Spacer.hpp>
#include <horizon/ToolbarButton.hpp>
#include <horizon/Widget.hpp>
#include <memory>

namespace pluma_app {

class MainToolbar : public horizon::Widget {
public:
  MainToolbar();
  virtual ~MainToolbar() = default;

  horizon::EventsManager<horizon::EventContext> when_new_clicked;

private:
  horizon::ToolbarButton *m_btn_new = nullptr;
  horizon::ToolbarButton *m_btn_open = nullptr;
  horizon::ToolbarButton *m_btn_save = nullptr;
};

} // namespace pluma_app
