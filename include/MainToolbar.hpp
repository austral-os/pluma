#pragma once

#include <horizon/Spacer.hpp>
#include <horizon/ToolbarButton.hpp>
#include <horizon/Widget.hpp>
#include <memory>

#include <horizon/SearchBox.hpp>

namespace pluma_app {

class MainToolbar : public horizon::Widget {
public:
  MainToolbar();
  virtual ~MainToolbar() = default;

  horizon::EventsManager<horizon::EventContext> when_new_clicked;
  horizon::EventsManager<horizon::EventContext> when_cut_clicked;
  horizon::EventsManager<horizon::EventContext> when_copy_clicked;
  horizon::EventsManager<horizon::EventContext> when_paste_clicked;

private:
  horizon::ToolbarButton *m_btn_new = nullptr;
  horizon::ToolbarButton *m_btn_open = nullptr;
  horizon::ToolbarButton *m_btn_save = nullptr;
  horizon::ToolbarButton *m_btn_cut = nullptr;
  horizon::ToolbarButton *m_btn_copy = nullptr;
  horizon::ToolbarButton *m_btn_paste = nullptr;
  horizon::ToolbarButton *m_btn_undo = nullptr;
  horizon::ToolbarButton *m_btn_redo = nullptr;
  horizon::SearchBox *m_search_box = nullptr;
};

} // namespace pluma_app
