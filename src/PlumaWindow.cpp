#include "PlumaWindow.hpp"
#include "MainToolbar.hpp"
#include "Ribbon/HomeSection.hpp"
#include <filesystem>
#include <horizon/Label.hpp>
#include <horizon/Logger.hpp>
#include <horizon/RibbonToolbar.hpp>
#include <horizon/ScrollArea.hpp>
#include <horizon/Toolbar.hpp>
#include <horizon/Widget.hpp>
#include <pluma/Style/StyleProperties.hpp>

namespace pluma_app {

PlumaWindow::PlumaWindow() : horizon::ApplicationWindow("Pluma") {
  set_title("Pluma Rich Text Editor");
  set_size(1024, 768);
  show_status_bar();

  auto main_toolbar = std::make_unique<MainToolbar>();
  auto *tb_ptr = main_toolbar.get();
  toolbar()->add_toolbar_widget(std::move(main_toolbar));

  tb_ptr->when_new_clicked.connect(
      [this](horizon::EventContext &) { this->new_file(); });

  auto tabs = std::make_unique<horizon::TabCollection>();
  m_tabs = tabs.get();
  m_tabs->set_smart_header(true);
  m_tabs->set_closable_tabs(true);

  m_tabs->when_tab_close_requested.connect([this](int index) {
    m_tabs->remove_tab(index);
    if (index >= 0 && index < m_home_sections.size()) {
      m_home_sections.erase(m_home_sections.begin() + index);
    }
    if (m_tabs->tab_count() == 0) {
      new_file();
    }
  });

  m_tabs->when_add_tab_clicked.connect(
      [this](horizon::EventContext &) { new_file(); });

  m_tabs->when_tab_selected.connect([this](int) { this->update_status_bar(); });

  tabs->set_position_type(horizon::FILL);

  set_content(std::move(tabs));

  setup_events();

  // Create an initial empty tab
  new_file();
}

void PlumaWindow::new_file() { create_tab("Sin título"); }

void PlumaWindow::create_tab(const std::string &title,
                             const std::string &path) {
  auto tab_container = std::make_unique<horizon::Widget>();
  tab_container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  tab_container->set_position_type(horizon::FILL);

  auto ribbon = std::make_unique<horizon::RibbonToolbar>();
  ribbon->set_fixed_size(135);

  int t1 = ribbon->add_tab("Home");
  auto home_sec = std::make_unique<HomeSection>(ribbon.get(), t1);
  m_home_sections.push_back(std::move(home_sec));

  tab_container->add_child(std::move(ribbon));

  auto scroll_area = std::make_unique<horizon::ScrollArea>();
  auto pluma_view = std::make_unique<PlumaView>();

  PlumaView* raw_view_ptr = pluma_view.get();

  if (!path.empty()) {
    pluma_view->load_document(path);
    pluma_view->set_current_path(path);
  }

  pluma_view->editor()->setCursorStateCallback(
      [this, view_ptr = raw_view_ptr](const pluma::CursorState &) {
        if (this->get_current_view() == view_ptr) {
          this->update_status_bar();
        }
      });

  scroll_area->set_content(std::move(pluma_view));
  tab_container->add_child(std::move(scroll_area));

  m_tabs->add_tab(title, std::move(tab_container));
  m_tabs->set_current_tab(m_tabs->tab_count() - 1);

  m_home_sections.back()->combo_font_family()->when_item_selected.connect(
      [this, view_ptr = raw_view_ptr](horizon::ComboItemSelectedContext &ctx) {
        LOG_INFO << "Combo item selected! id: " << ctx.item.id;
        if (view_ptr && view_ptr->editor()) {
          auto editor = view_ptr->editor();
          auto selection = editor->getSelectionRange();
          LOG_INFO << "Selection start: " << selection.getStart() 
                   << " length: " << selection.getLength();
          if (!selection.isCollapsed()) {
            editor->applyStyle(selection.getStart(), selection.getLength(),
                               pluma::PropertyId::FontFamily, ctx.item.id);
            view_ptr->calculate_layout();
            view_ptr->invalidate();
            if (view_ptr->parent()) {
              view_ptr->parent()->calculate_layout();
              view_ptr->parent()->invalidate();
            }
            LOG_INFO << "Applied font style!";
          }
        }
      });

  m_home_sections.back()->combo_font_size()->when_item_selected.connect(
      [this, view_ptr = raw_view_ptr](horizon::ComboItemSelectedContext &ctx) {
        LOG_INFO << "Font size combo selected! id: " << ctx.item.id;
        if (view_ptr && view_ptr->editor()) {
          auto editor = view_ptr->editor();
          auto selection = editor->getSelectionRange();
          if (!selection.isCollapsed()) {
            float size = 12.0f;
            try {
              size = std::stof(ctx.item.id);
            } catch (...) {}
            
            editor->applyStyle(selection.getStart(), selection.getLength(),
                               pluma::PropertyId::FontSize, size);
            view_ptr->calculate_layout();
            view_ptr->invalidate();
            if (view_ptr->parent()) {
              view_ptr->parent()->calculate_layout();
              view_ptr->parent()->invalidate();
            }
            LOG_INFO << "Applied font size style!";
          }
        }
      });
}

PlumaView *PlumaWindow::get_current_view() const {
  if (!m_tabs)
    return nullptr;
  auto *tab_container = dynamic_cast<horizon::Widget *>(m_tabs->current_tab_body());
  if (tab_container && tab_container->children().size() >= 2) {
    auto *scroll = dynamic_cast<horizon::ScrollArea *>(tab_container->children()[1].get());
    if (scroll) {
      for (const auto& child : scroll->children()) {
        if (auto* view = dynamic_cast<PlumaView*>(child.get())) {
          return view;
        }
      }
    }
  }
  return nullptr;
}

std::string PlumaWindow::current_file_path() const {
  auto *view = get_current_view();
  return view ? view->current_path() : "";
}

void PlumaWindow::setup_events() {
  when_file_opened.connect([this](Window::FileOpenedContext &ctx) {
    std::filesystem::path p(ctx.path);
    create_tab(p.filename().string(), ctx.path);
    LOG_INFO << "Pluma: Opened file in new tab: " << ctx.path;
  });

  when_save.connect([this](Window::FileSaveContext &ctx) {
    auto *view = get_current_view();
    if (view && view->save_document(ctx.path)) {
      view->set_current_path(ctx.path);
      std::filesystem::path p(ctx.path);
      m_tabs->set_tab_title(m_tabs->current_tab_index(), p.filename().string());
      LOG_INFO << "Pluma: Saved file to: " << ctx.path;
    } else {
      LOG_ERROR << "Pluma: Failed to save file to: " << ctx.path;
    }
  });

  when_save_as.connect([this](Window::FileSaveContext &ctx) {
    auto *view = get_current_view();
    if (view && view->save_document(ctx.path)) {
      view->set_current_path(ctx.path);
      std::filesystem::path p(ctx.path);
      m_tabs->set_tab_title(m_tabs->current_tab_index(), p.filename().string());
      LOG_INFO << "Pluma: Saved file as: " << ctx.path;
    } else {
      LOG_ERROR << "Pluma: Failed to save file as: " << ctx.path;
    }
  });
}

void PlumaWindow::update_status_bar() {
  auto *view = get_current_view();
  if (!view || !view->editor()) {
    set_status_text("Ln 1, Col 1 | Total: 1");
    return;
  }

  auto editor = view->editor();
  std::string text = editor->getText();
  uint32_t offset = editor->getCursorOffset();

  int line = 1;
  int col = 1;
  for (uint32_t i = 0; i < offset && i < text.length(); ++i) {
    if (text[i] == '\n') {
      line++;
      col = 1;
    } else {
      col++;
    }
  }

  int total_lines = 1;
  for (char c : text) {
    if (c == '\n')
      total_lines++;
  }

  char buf[128];
  snprintf(buf, sizeof(buf), "Ln %d, Col %d | Total: %d", line, col,
           total_lines);
  set_status_text(buf);
}

} // namespace pluma_app
