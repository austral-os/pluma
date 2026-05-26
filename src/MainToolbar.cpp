#include "MainToolbar.hpp"
#include <Spacer.hpp>
#include <horizon/Application.hpp>
#include <horizon/SearchBox.hpp>

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

  auto btn_cut = std::make_unique<horizon::ToolbarButton>("", "pluma-cut");
  btn_cut->set_fixed_size(40);

  auto btn_copy = std::make_unique<horizon::ToolbarButton>("", "pluma-copy");
  btn_copy->set_fixed_size(40);

  auto btn_paste = std::make_unique<horizon::ToolbarButton>("", "pluma-paste");
  btn_paste->set_fixed_size(40);

  auto btn_undo = std::make_unique<horizon::ToolbarButton>("", "pluma-undo");
  btn_undo->set_fixed_size(40);

  auto btn_redo = std::make_unique<horizon::ToolbarButton>("", "pluma-redo");
  btn_redo->set_fixed_size(40);

  m_btn_new = btn_new.get();
  m_btn_open = btn_open.get();
  m_btn_save = btn_save.get();
  m_btn_cut = btn_cut.get();
  m_btn_copy = btn_copy.get();
  m_btn_paste = btn_paste.get();
  m_btn_undo = btn_undo.get();
  m_btn_redo = btn_redo.get();

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

  add_child(horizon::Spacer(5));
  add_child(std::move(btn_new));
  add_child(std::move(btn_open));
  add_child(std::move(btn_save));
  add_child(horizon::Spacer(10));
  add_child(std::move(btn_cut));
  add_child(std::move(btn_copy));
  add_child(std::move(btn_paste));
  add_child(horizon::Spacer(10));
  add_child(std::move(btn_undo));
  add_child(std::move(btn_redo));

  auto search_container = std::make_unique<horizon::Widget>();
  search_container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  search_container->set_position_type(horizon::FILL);
  search_container->set_fixed_size(200);

  add_child(horizon::Spacer()); // pushes the search box to the right
  auto search = std::make_unique<horizon::SearchBox>();
  search->set_fixed_size(30);
  m_search_box = search.get();

  search_container->add_child(horizon::Spacer(5));
  search_container->add_child(std::move(search));
  search_container->add_child(horizon::Spacer(5));

  add_child(std::move(search_container));
  add_child(horizon::Spacer(10));
}

} // namespace pluma_app
