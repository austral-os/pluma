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
#include <horizon/dialogs/FileDialog.hpp>

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

// ── Table Grid Picker ──────────────────────────────────────────────────────
struct TableHoverState {
  int hover_row = -1;
  int hover_col = -1;
};

class TableGridItem : public horizon::Widget {
public:
  TableGridItem(int row, int col, std::shared_ptr<TableHoverState> state,
                std::shared_ptr<std::vector<TableGridItem *>> all_cells)
      : m_row(row), m_col(col), m_state(std::move(state)),
        m_all_cells(std::move(all_cells)) {
    set_focusable(true);

    when_mouse_enter.connect([this](horizon::EventContext &) {
      m_state->hover_row = m_row;
      m_state->hover_col = m_col;
      // Invalidate all siblings so they repaint with new highlight
      for (auto *cell : *m_all_cells)
        cell->invalidate();
    });
  }

  void draw(horizon::GraphicsContext &ctx) override {
    bool highlighted =
        (m_row <= m_state->hover_row && m_col <= m_state->hover_col);

    if (highlighted) {
      // Highlighted cell: filled accent with border
      horizon::Color fill{0.25f, 0.55f, 0.95f, 0.35f};
      horizon::Color border{0.25f, 0.55f, 0.95f, 1.0f};
      ctx.setColor(fill);
      ctx.fillRect(x(), y(), width(), height());
      ctx.setColor(border);
      ctx.drawRect(x(), y(), width(), height());
    } else {
      // Normal cell
      horizon::Color fill{0.5f, 0.5f, 0.5f, 0.08f};
      horizon::Color border{0.5f, 0.5f, 0.5f, 0.4f};
      ctx.setColor(fill);
      ctx.fillRect(x(), y(), width(), height());
      ctx.setColor(border);
      ctx.drawRect(x(), y(), width(), height());
    }
  }

  int preferred_width()  const override { return 22; }
  int preferred_height() const override { return 22; }
  int preferred_height(int /*w*/) const override { return 22; }

  int row() const { return m_row; }
  int col() const { return m_col; }

private:
  int m_row;
  int m_col;
  std::shared_ptr<TableHoverState>          m_state;
  std::shared_ptr<std::vector<TableGridItem*>> m_all_cells;
};
// ── End Table Grid Picker ───────────────────────────────────────────────────

PlumaWindow::PlumaWindow(const std::string& initial_file) : horizon::ApplicationWindow("Pluma") {
  set_title("Pluma Writer");
  set_size(1024, 768);
  show_status_bar();

  auto main_toolbar = std::make_unique<MainToolbar>();
  auto *tb_ptr = main_toolbar.get();
  toolbar()->add_toolbar_widget(std::move(main_toolbar));

  tb_ptr->when_new_clicked.connect(
      [this](horizon::EventContext &) { this->new_file(); });

  tb_ptr->when_cut_clicked.connect([this](horizon::EventContext &) {
    auto *view = get_current_view();
    if (view) {
      view->perform(horizon::ClipboardAction::Cut);
      view->calculate_layout();
      view->invalidate();
    }
  });

  tb_ptr->when_copy_clicked.connect([this](horizon::EventContext &) {
    auto *view = get_current_view();
    if (view) {
      view->perform(horizon::ClipboardAction::Copy);
    }
  });

  tb_ptr->when_paste_clicked.connect([this](horizon::EventContext &) {
    auto *view = get_current_view();
    if (view) {
      view->perform(horizon::ClipboardAction::Paste);
      view->calculate_layout();
      view->invalidate();
    }
  });


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
      auto* view = this->get_current_view();
      this->update_ribbon_state(view, this->m_home_sections[index].get());
      if (view) {
        view->set_focus(true);
      }
    }
  });

  tabs->set_position_type(horizon::FILL);

  set_content(std::move(tabs));

  setup_events();

  if (!initial_file.empty()) {
    std::filesystem::path p(initial_file);
    create_tab(p.filename().string(), initial_file);
  } else {
    // Create an initial empty tab
    new_file();
  }
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

  m_home_sections.back()->btn_image()->when_mouse_press.connect(
      [this, view_ptr = raw_view_ptr](horizon::MouseButtonEventContext &ctx) {
        LOG_INFO << "Image button pressed!";
        if (view_ptr && view_ptr->editor()) {
          m_file_dialog = std::make_unique<horizon::FileDialog>(horizon::FileDialogMode::Open, "Insert Image");
          m_file_dialog->when_accepted.connect([this, view_ptr](horizon::FileDialogAcceptedContext& ctx_acc) {
            auto editor = view_ptr->editor();
            std::string img_tag = "\n|IMAGE:" + ctx_acc.selected_path + "|\n";
            editor->insertTextAtCursor(img_tag);
            view_ptr->calculate_layout();
            view_ptr->invalidate();
            if (view_ptr->parent()) {
              view_ptr->parent()->calculate_layout();
              view_ptr->parent()->invalidate();
            }
            m_file_dialog->quit();
          });
          m_file_dialog->when_cancelled.connect([this](horizon::FileDialogCancelledContext&) {
            m_file_dialog->quit();
          });
          
          m_file_dialog->run();
          m_file_dialog.reset();
        }
      });

  // ── Insert Table vault ──────────────────────────────────────────────────
  m_home_sections.back()->btn_table()->when_mouse_press.connect(
      [this, view_ptr = raw_view_ptr,
       home_ptr = raw_home_ptr](horizon::MouseButtonEventContext &) {
        constexpr int ROWS = 8;
        constexpr int COLS = 10;

        auto vault   = std::make_unique<horizon::Vault>();
        auto content = std::make_unique<horizon::Widget>();
        content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
        content->set_spacing(6);

        // Title label
        auto title = std::make_unique<horizon::Label>("Insertar tabla");
        title->set_fixed_size(24);
        content->add_child(std::move(title));

        // Shared hover state
        auto hover_state = std::make_shared<TableHoverState>();
        auto all_cells   = std::make_shared<std::vector<TableGridItem *>>();

        // Grid container
        auto grid = std::make_unique<horizon::Widget>();
        grid->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
        grid->set_spacing(2);

        // Leave grid resets highlight
        grid->when_mouse_leave.connect([hover_state, all_cells](horizon::EventContext &) {
          hover_state->hover_row = -1;
          hover_state->hover_col = -1;
          for (auto *cell : *all_cells)
            cell->invalidate();
        });

        for (int r = 0; r < ROWS; ++r) {
          auto row_widget = std::make_unique<horizon::Widget>();
          row_widget->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
          row_widget->set_spacing(2);

          for (int c = 0; c < COLS; ++c) {
            auto cell =
                std::make_unique<TableGridItem>(r, c, hover_state, all_cells);

            // Click: insert table
            cell->when_mouse_press.connect(
                [this, view_ptr, r, c](horizon::MouseButtonEventContext &) {
                  if (view_ptr && view_ptr->editor()) {
                    int num_rows = r + 1;
                    int num_cols = c + 1;

                    // Build the libpluma table markup:
                    //   \n|TBL:cols=N|\n
                    //   |ROW|\n|CEL|\n...\n
                    //   ...
                    //   |ENDTBL|\n
                    std::string tbl = "\n|TBL:cols=";
                    tbl += std::to_string(num_cols);
                    tbl += "|\n";
                    for (int ri = 0; ri < num_rows; ++ri) {
                      tbl += "|ROW|\n";
                      for (int ci = 0; ci < num_cols; ++ci) {
                        tbl += "|CEL|\n\n";
                      }
                    }
                    tbl += "|ENDTBL|\n";

                    auto editor = view_ptr->editor();
                    editor->insertTextAtCursor(tbl);
                    view_ptr->calculate_layout();
                    view_ptr->invalidate();
                    if (view_ptr->parent()) {
                      view_ptr->parent()->calculate_layout();
                      view_ptr->parent()->invalidate();
                    }
                  }
                  application()->close_vault();
                });

            all_cells->push_back(cell.get());
            row_widget->add_child(std::move(cell));
          }
          grid->add_child(std::move(row_widget));
        }

        // Hint label (updates on hover)
        auto hint = std::make_unique<horizon::Label>("0 x 0");
        hint->set_fixed_size(20);
        // We keep a raw ptr to update text — the label lives inside content
        // which owns it, so it's valid for the vault lifetime.
        // (No easy way to update dynamically without a tick; we leave a static hint)
        content->add_child(std::move(grid));
        content->add_child(std::move(hint));

        // Size the vault: COLS * (cell+spacing) + padding
        int vault_w = COLS * 24 + 20;
        int vault_h = ROWS * 24 + 60;
        content->set_size(vault_w, vault_h);
        vault->set_content(std::move(content));
        application()->show_vault(vault.release(), -1, -1, 0,
                                  home_ptr->btn_table());
      });
  // ── End Insert Table vault ─────────────────────────────────────────────

  m_home_sections.back()->group_lists()->when_button_clicked.connect(
      [this, view_ptr = raw_view_ptr, home_ptr = raw_home_ptr](horizon::GroupButtonClickEvent &ctx) {
        if (view_ptr && view_ptr->editor()) {
          auto editor = view_ptr->editor();
          auto selection = editor->getSelectionRange();
          uint32_t para_start = selection.getStart();
          std::string text = editor->getText();
          while (para_start > 0 && text[para_start - 1] != '\n') para_start--;
          
          uint32_t para_end = selection.getEnd();
          while (para_end < text.length() && text[para_end] != '\n') para_end++;

          std::string para_text = text.substr(para_start, para_end - para_start);
          
          size_t search_start = 0;
          if (para_text.substr(0, 8) == "|INDENT:") {
              size_t end_tag = para_text.find("|", 8);
              if (end_tag != std::string::npos) {
                  search_start = end_tag + 1;
              }
          }
          
          size_t end_list_tag = std::string::npos;
          if (para_text.substr(search_start, 4) == "|UL:" || para_text.substr(search_start, 4) == "|OL:") {
              end_list_tag = para_text.find("|", search_start + 4);
          }
          
          std::string new_tag = "";
          if (ctx.button_index == 0) { // Bullet
              if (para_text.substr(search_start, 4) == "|UL:") {
                  // toggle off
              } else {
                  new_tag = "|UL:1:disc|";
              }
          } else if (ctx.button_index == 1) { // Number
              if (para_text.substr(search_start, 4) == "|OL:") {
                  // toggle off
              } else {
                  new_tag = "|OL:1:1|";
              }
          }

          if (end_list_tag != std::string::npos) {
              editor->setSelection(para_start + search_start, para_start + end_list_tag + 1);
              editor->deleteSelection();
          }
          
          if (!new_tag.empty()) {
              editor->setSelection(para_start + search_start, para_start + search_start);
              editor->insertTextAtCursor(new_tag);
          }
          
          view_ptr->calculate_layout();
          view_ptr->invalidate();
          if (view_ptr->parent()) {
            view_ptr->parent()->calculate_layout();
            view_ptr->parent()->invalidate();
          }
          this->update_ribbon_state(view_ptr, home_ptr);
        }
      });

  m_home_sections.back()->group_indent()->when_button_clicked.connect(
      [this, view_ptr = raw_view_ptr, home_ptr = raw_home_ptr](horizon::GroupButtonClickEvent &ctx) {
        if (view_ptr && view_ptr->editor()) {
          auto editor = view_ptr->editor();
          auto selection = editor->getSelectionRange();
          uint32_t para_start = selection.getStart();
          std::string text = editor->getText();
          while (para_start > 0 && text[para_start - 1] != '\n') para_start--;
          
          uint32_t para_end = selection.getEnd();
          while (para_end < text.length() && text[para_end] != '\n') para_end++;

          std::string para_text = text.substr(para_start, para_end - para_start);
          
          size_t search_start = 0;
          if (para_text.length() >= 8 && para_text.substr(0, 8) == "|INDENT:") {
              size_t end_tag = para_text.find("|", 8);
              if (end_tag != std::string::npos) search_start = end_tag + 1;
          }
          
          bool has_list = false;
          std::string list_prefix = "";
          int list_level = 1;
          std::string list_suffix = "";
          size_t list_end = std::string::npos;
          
          if (para_text.length() >= search_start + 4 && 
              (para_text.substr(search_start, 4) == "|UL:" || para_text.substr(search_start, 4) == "|OL:")) {
              has_list = true;
              list_end = para_text.find("|", search_start + 4);
              if (list_end != std::string::npos) {
                  list_prefix = para_text.substr(search_start, 4);
                  std::string content = para_text.substr(search_start + 4, list_end - (search_start + 4));
                  size_t colon = content.find(":");
                  if (colon != std::string::npos) {
                      try { list_level = std::stoi(content.substr(0, colon)); } catch(...) {}
                      list_suffix = content.substr(colon);
                  } else {
                      try { list_level = std::stoi(content); } catch(...) {}
                  }
              }
          }
          
          if (has_list) {
              if (ctx.button_index == 0) list_level--;
              else if (ctx.button_index == 1) list_level++;
              
              if (list_level < 1) {
                  editor->setSelection(para_start + search_start, para_start + list_end + 1);
                  editor->deleteSelection();
              } else {
                  std::string new_tag = list_prefix + std::to_string(list_level) + list_suffix + "|";
                  editor->setSelection(para_start + search_start, para_start + list_end + 1);
                  editor->deleteSelection();
                  editor->setSelection(para_start + search_start, para_start + search_start);
                  editor->insertTextAtCursor(new_tag);
              }
          } else {
              float left = 0.0f, right = 0.0f, first = 0.0f;
              size_t end_tag = std::string::npos;
              
              if (para_text.substr(0, 8) == "|INDENT:") {
                  end_tag = para_text.find("|", 8);
                  if (end_tag != std::string::npos) {
                      std::string content = para_text.substr(8, end_tag - 8);
                      size_t c1 = content.find(":");
                      size_t c2 = (c1 != std::string::npos) ? content.find(":", c1 + 1) : std::string::npos;
                      try {
                          if (c1 != std::string::npos && c2 != std::string::npos) {
                              left = std::stof(content.substr(0, c1));
                              right = std::stof(content.substr(c1 + 1, c2 - c1 - 1));
                              first = std::stof(content.substr(c2 + 1));
                          }
                      } catch(...) {}
                  }
              }
              
              if (ctx.button_index == 0) { // Decrease
                  left -= 0.5f;
                  if (left < 0.0f) left = 0.0f;
              } else if (ctx.button_index == 1) { // Increase
                  left += 0.5f;
              }
              
              if (end_tag != std::string::npos) {
                  editor->setSelection(para_start, para_start + end_tag + 1);
                  editor->deleteSelection();
              }
              
              if (left > 0.01f || right > 0.01f || first > 0.01f) {
                  char buf[128];
                  snprintf(buf, sizeof(buf), "|INDENT:%.2f:%.2f:%.2f|", left, right, first);
                  editor->setSelection(para_start, para_start);
                  editor->insertTextAtCursor(buf);
              }
          }
          
          view_ptr->calculate_layout();
          view_ptr->invalidate();
          if (view_ptr->parent()) {
            view_ptr->parent()->calculate_layout();
            view_ptr->parent()->invalidate();
          }
          this->update_ribbon_state(view_ptr, home_ptr);
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

    uint32_t para_start = offset;
    std::string text = editor->getText();
    while (para_start > 0 && text[para_start - 1] != '\n') para_start--;
    
    uint32_t para_end = offset;
    while (para_end < text.length() && text[para_end] != '\n') para_end++;

    std::string para_text = text.substr(para_start, para_end - para_start);
    size_t search_start = 0;
    if (para_text.substr(0, 8) == "|INDENT:") {
        size_t end_tag = para_text.find("|", 8);
        if (end_tag != std::string::npos) search_start = end_tag + 1;
    }
    
    bool is_ul = (para_text.substr(search_start, 4) == "|UL:");
    bool is_ol = (para_text.substr(search_start, 4) == "|OL:");
    
    home_sec->group_lists()->set_item_active(0, is_ul);
    home_sec->group_lists()->set_item_active(1, is_ol);
}

} // namespace pluma_app
