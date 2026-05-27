#include "PlumaWindow.hpp"
#include "MainToolbar.hpp"
#include "Ribbon/HomeSection.hpp"
#include <filesystem>
#include <horizon/GraphicsContext.hpp>
#include <horizon/Label.hpp>
#include <horizon/Logger.hpp>
#include <horizon/RibbonToolbar.hpp>
#include <horizon/ScrollArea.hpp>
#include <horizon/Toolbar.hpp>
#include <horizon/Vault.hpp>
#include <horizon/Widget.hpp>
#include <pluma/Style/StyleProperties.hpp>

namespace pluma_app {

class ColorPaletteItem : public horizon::Widget {
public:
  ColorPaletteItem(horizon::Color c) : m_color(c) { set_focusable(true); }

  void draw(horizon::GraphicsContext &ctx) override {
    ctx.setColor(m_color);
    ctx.fillRect(x(), y(), width(), height());
  }

  int preferred_width() const override { return 24; }
  int preferred_height() const override { return 24; }
  int preferred_height(int /*width*/) const override { return 24; }

  horizon::Color color() const { return m_color; }

private:
  horizon::Color m_color;
};

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

  m_tabs->when_tab_selected.connect([this](int index) { 
    this->update_status_bar(); 
    if (index >= 0 && index < (int)this->m_home_sections.size()) {
      this->update_ribbon_state(this->get_current_view(), this->m_home_sections[index].get());
    }
  });

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

  PlumaView *raw_view_ptr = pluma_view.get();

  if (!path.empty()) {
    pluma_view->load_document(path);
    pluma_view->set_current_path(path);
  }

  auto raw_home_ptr = m_home_sections.back().get();
  pluma_view->editor()->setCursorStateCallback(
      [this, view_ptr = raw_view_ptr, home_ptr = raw_home_ptr](const pluma::CursorState &) {
        if (this->get_current_view() == view_ptr) {
          this->update_status_bar();
          this->update_ribbon_state(view_ptr, home_ptr);
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
            } catch (...) {
            }

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

  m_home_sections.back()->group_styles()->when_button_clicked.connect(
      [this, view_ptr = raw_view_ptr, home_ptr = raw_home_ptr](horizon::GroupButtonClickEvent &ctx) {
        if (view_ptr && view_ptr->editor()) {
          auto editor = view_ptr->editor();
          auto selection = editor->getSelectionRange();
          if (!selection.isCollapsed()) {
            auto current_style =
                editor->getFormatRegistry().getStyleAt(selection.getStart());

            if (ctx.button_index == 0) { // Bold (B)
              bool is_bold = false;
              if (auto fw = current_style.get(pluma::PropertyId::FontWeight)) {
                is_bold = (std::get<uint16_t>(*fw) >= 700);
              }
              editor->applyStyle(selection.getStart(), selection.getLength(),
                                 pluma::PropertyId::FontWeight,
                                 static_cast<uint16_t>(is_bold ? 400 : 700));
            } else if (ctx.button_index == 1) { // Italic (I)
              bool is_italic = false;
              if (auto fi =
                      current_style.get(pluma::PropertyId::FontStyleItalic)) {
                is_italic = std::get<bool>(*fi);
              }
              editor->applyStyle(selection.getStart(), selection.getLength(),
                                 pluma::PropertyId::FontStyleItalic,
                                 !is_italic);
            } else if (ctx.button_index == 2) { // Underline (U)
              pluma::TextDecoration dec = pluma::TextDecoration::None;
              if (auto d = current_style.get(pluma::PropertyId::Decoration)) {
                if (std::get<pluma::TextDecoration>(*d) ==
                    pluma::TextDecoration::None) {
                  dec = pluma::TextDecoration::Underline;
                }
              } else {
                dec = pluma::TextDecoration::Underline;
              }
              editor->applyStyle(selection.getStart(), selection.getLength(),
                                 pluma::PropertyId::Decoration, dec);
            } else if (ctx.button_index == 3) { // Superscript (x²)
              pluma::VerticalAlign va = pluma::VerticalAlign::Baseline;
              if (auto v =
                      current_style.get(pluma::PropertyId::VerticalAlignment)) {
                if (std::get<pluma::VerticalAlign>(*v) !=
                    pluma::VerticalAlign::Superscript) {
                  va = pluma::VerticalAlign::Superscript;
                } else {
                  va = pluma::VerticalAlign::Baseline;
                }
              } else {
                va = pluma::VerticalAlign::Superscript;
              }
              editor->applyStyle(selection.getStart(), selection.getLength(),
                                 pluma::PropertyId::VerticalAlignment, va);
            } else if (ctx.button_index == 4) { // Subscript (x₂)
              pluma::VerticalAlign va = pluma::VerticalAlign::Baseline;
              if (auto v =
                      current_style.get(pluma::PropertyId::VerticalAlignment)) {
                if (std::get<pluma::VerticalAlign>(*v) !=
                    pluma::VerticalAlign::Subscript) {
                  va = pluma::VerticalAlign::Subscript;
                } else {
                  va = pluma::VerticalAlign::Baseline;
                }
              } else {
                va = pluma::VerticalAlign::Subscript;
              }
              editor->applyStyle(selection.getStart(), selection.getLength(),
                                 pluma::PropertyId::VerticalAlignment, va);
            }

            view_ptr->calculate_layout();
            view_ptr->invalidate();
            if (view_ptr->parent()) {
              view_ptr->parent()->calculate_layout();
              view_ptr->parent()->invalidate();
            }
            this->update_ribbon_state(view_ptr, home_ptr);
          }
        }
      });

  m_home_sections.back()->group_alignment()->when_button_clicked.connect(
      [this, view_ptr = raw_view_ptr, home_ptr = raw_home_ptr](horizon::GroupButtonClickEvent &ctx) {
        if (view_ptr && view_ptr->editor()) {
          auto editor = view_ptr->editor();
          auto selection = editor->getSelectionRange();
          
          uint32_t para_start = selection.getStart();
          std::string text = editor->getText();
          while (para_start > 0 && text[para_start - 1] != '\n') {
              para_start--;
          }
          
          uint32_t para_end = selection.getEnd();
          while (para_end < text.length() && text[para_end] != '\n') {
              para_end++;
          }
          
          uint32_t length = para_end - para_start;
          if (length == 0) length = 1; // Ensure the style sticks

          pluma::TextAlign align = pluma::TextAlign::Left;
          if (ctx.button_index == 1) align = pluma::TextAlign::Center;
          else if (ctx.button_index == 2) align = pluma::TextAlign::Right;
          else if (ctx.button_index == 3) align = pluma::TextAlign::Justify;

          editor->applyStyle(para_start, length,
                             pluma::PropertyId::TextAlignment, align);

          view_ptr->calculate_layout();
          view_ptr->invalidate();
          if (view_ptr->parent()) {
            view_ptr->parent()->calculate_layout();
            view_ptr->parent()->invalidate();
          }
          this->update_ribbon_state(view_ptr, home_ptr);
        }
      });

  m_home_sections.back()->group_font_size()->when_button_clicked.connect(
      [this, view_ptr = raw_view_ptr](horizon::GroupButtonClickEvent &ctx) {
        if (view_ptr && view_ptr->editor()) {
          auto editor = view_ptr->editor();
          auto selection = editor->getSelectionRange();
          if (!selection.isCollapsed()) {
            auto current_style =
                editor->getFormatRegistry().getStyleAt(selection.getStart());

            float current_size = 12.0f;
            if (auto fs = current_style.get(pluma::PropertyId::FontSize)) {
              current_size = std::get<float>(*fs);
            }

            if (ctx.button_index == 0) { // A+
              current_size += 2.0f;
            } else if (ctx.button_index == 1) { // A-
              current_size -= 2.0f;
              if (current_size < 4.0f)
                current_size = 4.0f; // Minimum size limit
            }

            editor->applyStyle(selection.getStart(), selection.getLength(),
                               pluma::PropertyId::FontSize, current_size);

            view_ptr->calculate_layout();
            view_ptr->invalidate();
            if (view_ptr->parent()) {
              view_ptr->parent()->calculate_layout();
              view_ptr->parent()->invalidate();
            }
          }
        }
      });

  m_home_sections.back()->group_colors()->when_button_clicked.connect(
      [this, view_ptr = raw_view_ptr](horizon::GroupButtonClickEvent &ctx) {
        if (ctx.button_index == 0) { // Text Color
          auto vault = std::make_unique<horizon::Vault>();
          auto content = std::make_unique<horizon::Widget>();
          content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
          content->set_spacing(8);

          auto title = std::make_unique<horizon::Label>("Text Color");
          title->set_fixed_size(30);
          content->add_child(std::move(title));

          auto grid = std::make_unique<horizon::Widget>();
          grid->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
          grid->set_spacing(4);

          std::vector<horizon::Color> base_colors = {
              {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f},
              {0.8f, 0.8f, 0.8f, 1.0f}, {0.1f, 0.2f, 0.4f, 1.0f},
              {0.2f, 0.4f, 0.8f, 1.0f}, {0.9f, 0.4f, 0.1f, 1.0f},
              {0.6f, 0.6f, 0.6f, 1.0f}, {0.9f, 0.7f, 0.1f, 1.0f},
              {0.3f, 0.6f, 0.9f, 1.0f}, {0.4f, 0.7f, 0.3f, 1.0f}};

          for (size_t row_idx = 0; row_idx < 6; ++row_idx) {
            auto row = std::make_unique<horizon::Widget>();
            row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
            row->set_spacing(2);

            for (size_t col_idx = 0; col_idx < 10; ++col_idx) {
              horizon::Color c = base_colors[col_idx];
              if (row_idx > 0) {
                float factor = 1.0f - (row_idx * 0.15f); // darken
                if (col_idx == 0 ||
                    col_idx == 2) { // lighten for black/dark colors
                  factor = 1.0f - ((5 - row_idx) * 0.15f);
                }
                c.r *= factor;
                c.g *= factor;
                c.b *= factor;
              }

              auto btn = std::make_unique<ColorPaletteItem>(c);
              btn->when_mouse_press.connect([this, view_ptr, c](auto &) {
                if (view_ptr && view_ptr->editor()) {
                  auto editor = view_ptr->editor();
                  auto selection = editor->getSelectionRange();
                  if (!selection.isCollapsed()) {
                    uint32_t argb = (255 << 24) | ((int)(c.r * 255) << 16) |
                                    ((int)(c.g * 255) << 8) |
                                    ((int)(c.b * 255));
                    editor->applyStyle(selection.getStart(),
                                       selection.getLength(),
                                       pluma::PropertyId::TextColor, argb);
                    view_ptr->calculate_layout();
                    view_ptr->invalidate();
                    if (view_ptr->parent()) {
                      view_ptr->parent()->calculate_layout();
                      view_ptr->parent()->invalidate();
                    }
                  }
                }
                application()->close_vault();
              });
              row->add_child(std::move(btn));
            }
            grid->add_child(std::move(row));
          }

          content->add_child(std::move(grid));
          content->set_size(280, 200);
          vault->set_content(std::move(content));
          application()->show_vault(vault.release(), -1, -1, 0,
                                    m_home_sections.back()->group_colors());
        } else if (ctx.button_index == 1) { // Background Color
          auto vault = std::make_unique<horizon::Vault>();
          auto content = std::make_unique<horizon::Widget>();
          content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
          content->set_spacing(8);

          auto title = std::make_unique<horizon::Label>("Background Color");
          title->set_fixed_size(30);
          content->add_child(std::move(title));

          auto grid = std::make_unique<horizon::Widget>();
          grid->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
          grid->set_spacing(4);

          std::vector<horizon::Color> base_colors = {
              {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f},
              {0.8f, 0.8f, 0.8f, 1.0f}, {0.1f, 0.2f, 0.4f, 1.0f},
              {0.2f, 0.4f, 0.8f, 1.0f}, {0.9f, 0.4f, 0.1f, 1.0f},
              {0.6f, 0.6f, 0.6f, 1.0f}, {0.9f, 0.7f, 0.1f, 1.0f},
              {0.3f, 0.6f, 0.9f, 1.0f}, {0.4f, 0.7f, 0.3f, 1.0f}};

          for (size_t row_idx = 0; row_idx < 6; ++row_idx) {
            auto row = std::make_unique<horizon::Widget>();
            row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
            row->set_spacing(2);

            for (size_t col_idx = 0; col_idx < 10; ++col_idx) {
              horizon::Color c = base_colors[col_idx];
              if (row_idx > 0) {
                float factor = 1.0f - (row_idx * 0.15f); // darken
                if (col_idx == 0 ||
                    col_idx == 2) { // lighten for black/dark colors
                  factor = 1.0f - ((5 - row_idx) * 0.15f);
                }
                c.r *= factor;
                c.g *= factor;
                c.b *= factor;
              }

              auto btn = std::make_unique<ColorPaletteItem>(c);
              btn->when_mouse_press.connect([this, view_ptr, c](auto &) {
                if (view_ptr && view_ptr->editor()) {
                  auto editor = view_ptr->editor();
                  auto selection = editor->getSelectionRange();
                  if (!selection.isCollapsed()) {
                    uint32_t argb = (255 << 24) | ((int)(c.r * 255) << 16) |
                                    ((int)(c.g * 255) << 8) |
                                    ((int)(c.b * 255));
                    editor->applyStyle(selection.getStart(),
                                       selection.getLength(),
                                       pluma::PropertyId::BackgroundColor, argb);
                    view_ptr->calculate_layout();
                    view_ptr->invalidate();
                    if (view_ptr->parent()) {
                      view_ptr->parent()->calculate_layout();
                      view_ptr->parent()->invalidate();
                    }
                  }
                }
                application()->close_vault();
              });
              row->add_child(std::move(btn));
            }
            grid->add_child(std::move(row));
          }

          content->add_child(std::move(grid));
          content->set_size(280, 200);
          vault->set_content(std::move(content));
          application()->show_vault(vault.release(), -1, -1, 0,
                                    m_home_sections.back()->group_colors());
        }
      });
}

PlumaView *PlumaWindow::get_current_view() const {
  if (!m_tabs)
    return nullptr;
  auto *tab_container =
      dynamic_cast<horizon::Widget *>(m_tabs->current_tab_body());
  if (tab_container && tab_container->children().size() >= 2) {
    auto *scroll =
        dynamic_cast<horizon::ScrollArea *>(tab_container->children()[1].get());
    if (scroll) {
      for (const auto &child : scroll->children()) {
        if (auto *view = dynamic_cast<PlumaView *>(child.get())) {
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

void PlumaWindow::update_ribbon_state(PlumaView* view, HomeSection* home_sec) {
    if (!view || !view->editor() || !home_sec) return;
    
    auto editor = view->editor();
    uint32_t offset = editor->getCursorOffset();
    if (!editor->getSelectionRange().isCollapsed()) {
        offset = editor->getSelectionRange().getStart();
    }
    
    pluma::PropertyBag style = editor->getFormatRegistry().getStyleAt(offset);
    
    if (auto ff = style.get(pluma::PropertyId::FontFamily)) {
        std::string family = std::get<std::string>(*ff);
        home_sec->combo_font_family()->set_selected_item_by_id(family);
    }
    
    if (auto fs = style.get(pluma::PropertyId::FontSize)) {
        float size = std::get<float>(*fs);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", size);
        home_sec->combo_font_size()->set_selected_item_by_id(buf);
    }
    
    bool is_bold = false;
    if (auto fw = style.get(pluma::PropertyId::FontWeight)) {
        is_bold = std::get<uint16_t>(*fw) >= 700;
    }
    std::cout << "[PlumaWindow] update_ribbon_state is_bold=" << is_bold << std::endl;
    home_sec->group_styles()->set_item_active(0, is_bold);

    bool is_italic = false;
    if (auto it = style.get(pluma::PropertyId::FontStyleItalic)) {
        is_italic = std::get<bool>(*it);
    }
    home_sec->group_styles()->set_item_active(1, is_italic);

    bool is_underline = false;
    if (auto td = style.get(pluma::PropertyId::Decoration)) {
        is_underline = std::get<pluma::TextDecoration>(*td) == pluma::TextDecoration::Underline;
    }
    home_sec->group_styles()->set_item_active(2, is_underline);

    bool is_super = false;
    bool is_sub = false;
    if (auto va = style.get(pluma::PropertyId::VerticalAlignment)) {
        auto val = std::get<pluma::VerticalAlign>(*va);
        is_super = (val == pluma::VerticalAlign::Superscript);
        is_sub = (val == pluma::VerticalAlign::Subscript);
    }
    home_sec->group_styles()->set_item_active(3, is_super);
    home_sec->group_styles()->set_item_active(4, is_sub);

    pluma::TextAlign align = pluma::TextAlign::Left;
    if (auto ta = style.get(pluma::PropertyId::TextAlignment)) {
        align = std::get<pluma::TextAlign>(*ta);
    }
    home_sec->group_alignment()->set_item_active(0, align == pluma::TextAlign::Left);
    home_sec->group_alignment()->set_item_active(1, align == pluma::TextAlign::Center);
    home_sec->group_alignment()->set_item_active(2, align == pluma::TextAlign::Right);
    home_sec->group_alignment()->set_item_active(3, align == pluma::TextAlign::Justify);
}

} // namespace pluma_app
