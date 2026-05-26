#include "MainToolbar.hpp"
#include <horizon/Application.hpp>

namespace pluma_app {

MainToolbar::MainToolbar() : horizon::Widget() {
  set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  set_position_type(horizon::FILL);
  set_spacing(4);

  auto btn_new = std::make_unique<horizon::ToolbarButton>("", "pluma-filenew");
  btn_new->set_fixed_size(40);

  auto btn_open =
      std::make_unique<horizon::ToolbarButton>("", "pluma-fileopen");
  btn_open->set_fixed_size(40);

  auto btn_save =
      std::make_unique<horizon::ToolbarButton>("", "pluma-filesave");
  btn_save->set_fixed_size(40);

  m_btn_new = btn_new.get();
  m_btn_open = btn_open.get();
  m_btn_save = btn_save.get();

  m_btn_new->when_click.connect(
      [this](horizon::EventContext &ctx) { this->when_new_clicked.run(ctx); });

  m_btn_open->when_click.connect([this](horizon::EventContext &) {
    horizon::SignalContext ctx;
    if (application())
      application()->signal_manager.emit("file.open", ctx);
  });

  m_btn_save->when_click.connect([this](horizon::EventContext &) {
    horizon::SignalContext ctx;
    if (application())
      application()->signal_manager.emit("file.save", ctx);
  });

  add_child(std::move(horizon::Spacer(10)));
  add_child(std::move(btn_new));
  add_child(std::move(btn_open));
  add_child(std::move(btn_save));
}

} // namespace pluma_app
