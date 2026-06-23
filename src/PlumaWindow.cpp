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
#include <horizon/I18n.hpp>
#include <pluma/Style/StyleProperties.hpp>
#include <horizon/dialogs/FileDialog.hpp>

#include <horizon/SearchBox.hpp>
#include <horizon/TableView.hpp>
#include <horizon/TableColumn.hpp>
#include <horizon/Panel.hpp>
#include <unicode/locid.h>

struct LanguageItem {
    std::string id;
    std::string display_name;
};


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

PlumaWindow::PlumaWindow(const std::string& initial_file) : horizon::ApplicationWindow(horizon::i18n().tr("pluma-writer.title")) {
  set_size(1024, 768);

  show_status_bar();
  m_status_label = new horizon::Label("Ready");
  m_status_label->set_position_type(horizon::FILL);
  m_lang_label = new horizon::Label("es"); // default
  m_lang_label->set_fixed_size(150);
  // removed FREE position
  
  // Custom draw for label to make it look like a button
  m_lang_label->set_focusable(true);

  if (statusbar()) {
      statusbar()->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
      statusbar()->add_child(std::unique_ptr<horizon::Widget>(m_status_label));
      
      statusbar()->add_child(horizon::Spacer());
      
      statusbar()->add_child(std::unique_ptr<horizon::Widget>(m_lang_label));

      m_zoom_slider = new horizon::Slider();
      m_zoom_slider->set_fixed_size(200);
      m_zoom_slider->set_thumb_shape(horizon::ThumbShape::Circle);
      m_zoom_slider->set_min(0.5f);
      m_zoom_slider->set_max(2.0f);
      m_zoom_slider->set_value(1.0f);
      m_zoom_slider->when_value_changed.connect([this](horizon::EventContext& ctx) {
          if (!m_zoom_slider) return;
          float zoom = m_zoom_slider->value();
          auto* view = get_current_view();
          if (view) {
              view->set_zoom(zoom);
          }
          if (m_zoom_label) {
              char buf[32];
              snprintf(buf, sizeof(buf), "%d%%", (int)(zoom * 100));
              m_zoom_label->set_text(buf);
              m_zoom_label->invalidate();
          }
      });
      statusbar()->add_child(std::unique_ptr<horizon::Widget>(m_zoom_slider));

      m_zoom_label = new horizon::Label("100%");
      m_zoom_label->set_fixed_size(60);
      statusbar()->add_child(std::unique_ptr<horizon::Widget>(m_zoom_label));
      
      // Hook up the vault creation on click
      m_lang_label->when_mouse_press.connect([this](horizon::MouseButtonEventContext &) {
          auto vault = std::make_unique<horizon::Vault>();
          auto content = std::make_unique<horizon::Widget>();
          content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
          content->set_spacing(6);
          
          auto title = std::make_unique<horizon::Label>(horizon::i18n().tr("Language"));
          title->set_fixed_size(24);
          content->add_child(std::move(title));
          
          auto search = std::make_unique<horizon::SearchBox>();
          search->set_placeholder("Search...");
          auto search_ptr = search.get();
          content->add_child(std::move(search));
          
          auto table = std::make_unique<horizon::TableView<LanguageItem>>();
          table->set_width_mode(horizon::TableViewWidthMode::Fill);
          
          horizon::TableColumn<LanguageItem> col1;
          col1.id = "name";
          col1.title = "Language";
          col1.width = -1;
          col1.cell_factory = [](const LanguageItem &item) {
              return std::make_unique<horizon::Label>(item.display_name);
          };
          table->add_column(std::move(col1));
          table->set_header_visible(false);
          
          std::vector<LanguageItem> all_langs;
          
          // scan dictionaries
          std::vector<std::string> search_paths = {
              "dictionaries/",
              "build/dictionaries/",
              "assets/dictionaries/",
              "/usr/share/pluma-writer/dictionaries/",
              "/usr/local/share/pluma-writer/dictionaries/"
          };
          std::string active_path;
          for (const auto& path : search_paths) {
              if (std::filesystem::exists(path)) {
                  active_path = path; break;
              }
          }
          if (!active_path.empty()) {
              try {
                  for (const auto& entry : std::filesystem::directory_iterator(active_path)) {
                      if (entry.path().extension() == ".aff") {
                          std::string lang = entry.path().stem().string();
                          icu::Locale loc(lang.c_str());
                          icu::UnicodeString ustr;
                          loc.getDisplayName(loc, ustr);
                          std::string disp;
                          ustr.toUTF8String(disp);
                          if (disp.empty()) disp = lang;
                          all_langs.push_back({lang, disp});
                      }
                  }
              } catch (...) {}
          }
          
          std::sort(all_langs.begin(), all_langs.end(), [](const LanguageItem& a, const LanguageItem& b) {
              return a.display_name < b.display_name;
          });
          
          std::cout << "POPULATING TABLE WITH " << all_langs.size() << " ITEMS\n"; table->set_data(all_langs);
          
          auto table_ptr = table.get();
          
          search_ptr->when_key_press.connect([this, table_ptr](horizon::KeyEventContext& ev) {
              if (table_ptr->data().empty()) return;
              int current_idx = table_ptr->selected_index();
              int new_idx = current_idx;

              if (ev.keysym == 0xff52) { // Up arrow
                  new_idx = (current_idx <= 0) ? 0 : current_idx - 1;
              } else if (ev.keysym == 0xff54) { // Down arrow
                  new_idx = (current_idx < 0) ? 0 : std::min((int)table_ptr->data().size() - 1, current_idx + 1);
              } else {
                  return; // Let TextBox handle typing
              }

              if (new_idx != current_idx && new_idx >= 0 && new_idx < (int)table_ptr->data().size()) {
                  table_ptr->set_selected_index(new_idx);
                  table_ptr->scroll_to_index(new_idx);
                  ev.stop_propagation = true;
              }
          });

          search_ptr->when_submit.connect([table_ptr](horizon::KeyEventContext&) {
              int current_idx = table_ptr->selected_index();
              if (current_idx >= 0 && current_idx < (int)table_ptr->data().size()) {
                  horizon::TableViewRowMouseClickContext<LanguageItem> click_ctx;
                  click_ctx.row_index = current_idx;
                  click_ctx.row_data = table_ptr->data()[current_idx];
                  table_ptr->when_row_click.run(click_ctx);
              }
          });

          search_ptr->when_text_changed.connect([table_ptr, all_langs](horizon::EventContext&) {
              std::string query = ((horizon::TextBox<horizon::TextPolicy>*)table_ptr->parent()->children()[1].get())->text();
              std::transform(query.begin(), query.end(), query.begin(), ::tolower);
              if (query.empty()) {
                  table_ptr->set_data(all_langs);
              } else {
                  std::vector<LanguageItem> filtered;
                  for (const auto& lang : all_langs) {
                      std::string name_lower = lang.display_name;
                      std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                      if (name_lower.find(query) != std::string::npos) {
                          filtered.push_back(lang);
                      }
                  }
                  table_ptr->set_data(filtered);
                  if (!filtered.empty()) {
                      table_ptr->set_selected_index(0);
                  }
              }
          });
          
          table_ptr->when_row_click.connect([this](horizon::EventContext& ctx) {
              auto& ev = static_cast<horizon::TableViewRowMouseClickContext<LanguageItem>&>(ctx);
              auto* view = this->get_current_view();
              if (view && view->editor()) {
                  auto editor = view->editor();
                  auto sel = editor->getSelectionRange();
                  if (!sel.isCollapsed()) {
                      editor->applyStyle(sel.getStart(), sel.getLength(), pluma::PropertyId::Language, ev.row_data.id);
                      view->calculate_layout();
                      view->invalidate();
                      if (view->parent()) view->parent()->invalidate();
                      view->triggerAnalysis();
                  }
              }
              application()->close_vault();
              if (view) view->set_focus(true);
          });
          
          table->set_position_type(horizon::FILL);
          content->add_child(std::move(table));
          
          content->set_size(300, 400);
          vault->set_content(std::move(content));
          application()->show_vault(vault.release(), -1, -1, 0, m_lang_label);
          search_ptr->set_focus(true);
      });
  }



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
      m_insert_sections.erase(m_insert_sections.begin() + index);
      m_page_layout_sections.erase(m_page_layout_sections.begin() + index);
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

void PlumaWindow::new_file() { create_tab(horizon::i18n().tr("pluma-writer.status.untitled")); }

void PlumaWindow::create_tab(const std::string &title,
                             const std::string &path) {
  auto tab_container = std::make_unique<horizon::Widget>();
  tab_container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  tab_container->set_position_type(horizon::FILL);

  auto ribbon = std::make_unique<horizon::RibbonToolbar>();
  ribbon->set_fixed_size(135);

  int t1 = ribbon->add_tab(horizon::i18n().tr("pluma-writer.tabs.home"));
  auto home_sec = std::make_unique<HomeSection>(ribbon.get(), t1);
  m_home_sections.push_back(std::move(home_sec));

  int t2 = ribbon->add_tab(horizon::i18n().tr("pluma-writer.tabs.page_layout"));
  auto layout_sec = std::make_unique<PageLayoutSection>(ribbon.get(), t2);
  m_page_layout_sections.push_back(std::move(layout_sec));

  int t3 = ribbon->add_tab(horizon::i18n().tr("pluma-writer.tabs.image_format"));
  auto image_sec = std::make_unique<ImageFormatSection>(ribbon.get(), t3);
  m_image_format_sections.push_back(std::move(image_sec));
  ribbon->set_tab_visible(t3, false);

  int t4 = ribbon->add_tab("Table Layout");
  auto table_sec = std::make_unique<TableLayoutSection>(ribbon.get(), t4);
  m_table_layout_sections.push_back(std::move(table_sec));
  ribbon->set_tab_visible(t4, false);

  int t_insert = ribbon->add_tab(horizon::i18n().tr("pluma-writer.tabs.insert"));
  auto insert_sec = std::make_unique<InsertSection>(ribbon.get(), t_insert);
  m_insert_sections.push_back(std::move(insert_sec));

  tab_container->add_child(std::move(ribbon));

  auto scroll_area = std::make_unique<horizon::ScrollArea>();
  auto pluma_view = std::make_unique<PlumaView>();

  PlumaView *raw_view_ptr = pluma_view.get();

  m_page_layout_sections.back()->when_margin_selected.connect(
      [this, view_ptr = raw_view_ptr](pluma::PageMargins &margins) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->setMargins(margins);
          view_ptr->calculate_layout();
          view_ptr->invalidate();
          if (view_ptr->parent()) {
            view_ptr->parent()->calculate_layout();
            view_ptr->parent()->invalidate();
          }
        }
      });

  m_page_layout_sections.back()->when_orientation_selected.connect(
      [this, view_ptr = raw_view_ptr](bool &landscape) {
        if (view_ptr && view_ptr->editor()) {
          auto size = view_ptr->editor()->getPageSize();
          bool currently_landscape = size.width.getValue() > size.height.getValue();
          if (landscape != currently_landscape) {
             view_ptr->editor()->setPageSize({size.height, size.width});
             view_ptr->calculate_layout();
             view_ptr->invalidate();
             if (view_ptr->parent()) {
               view_ptr->parent()->calculate_layout();
               view_ptr->parent()->invalidate();
             }
          }
        }
      });

  m_page_layout_sections.back()->when_size_selected.connect(
      [this, view_ptr = raw_view_ptr](pluma::PageSize &new_size) {
        if (view_ptr && view_ptr->editor()) {
          auto current_size = view_ptr->editor()->getPageSize();
          bool currently_landscape = current_size.width.getValue() > current_size.height.getValue();
          
          pluma::PageSize final_size = new_size;
          if (currently_landscape) {
             final_size = {new_size.height, new_size.width}; // Maintain landscape orientation
          }

          view_ptr->editor()->setPageSize(final_size);
          view_ptr->calculate_layout();
          view_ptr->invalidate();
          if (view_ptr->parent()) {
            view_ptr->parent()->calculate_layout();
            view_ptr->parent()->invalidate();
          }
        }
      });

  m_image_format_sections.back()->when_wrap_mode_selected.connect(
      [this, view_ptr = raw_view_ptr](pluma::TextWrapMode mode) {
        if (view_ptr && view_ptr->editor()) {
          uint32_t offset = view_ptr->editor()->getCursorOffset();
          view_ptr->editor()->applyStyle(offset, 1, pluma::PropertyId::ImageWrapMode, mode);
        }
      });

  m_table_layout_sections.back()->when_insert_above_clicked.connect(
      [this, view_ptr = raw_view_ptr](int&) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->insertTableRowAbove();
          view_ptr->calculate_layout();
        }
      });
      
  m_table_layout_sections.back()->when_insert_below_clicked.connect(
      [this, view_ptr = raw_view_ptr](int&) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->insertTableRowBelow();
          view_ptr->calculate_layout();
        }
      });

  m_table_layout_sections.back()->when_insert_left_clicked.connect(
      [this, view_ptr = raw_view_ptr](int&) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->insertTableColumnLeft();
          view_ptr->calculate_layout();
        }
      });

  m_table_layout_sections.back()->when_insert_right_clicked.connect(
      [this, view_ptr = raw_view_ptr](int&) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->insertTableColumnRight();
          view_ptr->calculate_layout();
        }
      });

  m_table_layout_sections.back()->when_merge_cells_clicked.connect(
      [this, view_ptr = raw_view_ptr](int&) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->mergeTableCells();
          view_ptr->calculate_layout();
        }
      });

  m_table_layout_sections.back()->when_split_cells_horizontally_clicked.connect(
      [this, view_ptr = raw_view_ptr](int&) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->splitTableCells(true);
          view_ptr->calculate_layout();
        }
      });

  m_table_layout_sections.back()->when_split_cells_vertically_clicked.connect(
      [this, view_ptr = raw_view_ptr](int&) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->splitTableCells(false);
          view_ptr->calculate_layout();
        }
      });

  m_table_layout_sections.back()->when_split_table_clicked.connect(
      [this, view_ptr = raw_view_ptr](int&) {
        if (view_ptr && view_ptr->editor()) {
          view_ptr->editor()->splitTable();
          view_ptr->calculate_layout();
        }
      });

  if (!path.empty()) {
    pluma_view->load_document(path);
    pluma_view->set_current_path(path);
  }

  auto raw_home_ptr = m_home_sections.back().get();
  auto raw_img_sec_ptr = m_image_format_sections.back().get();
  auto raw_tbl_sec_ptr = m_table_layout_sections.back().get();
  horizon::RibbonToolbar* ribbon_ptr = static_cast<horizon::RibbonToolbar*>(tab_container->children()[0].get());
  
  pluma_view->editor()->setCursorStateCallback(
      [this, view_ptr = raw_view_ptr, home_ptr = raw_home_ptr, img_sec_ptr = raw_img_sec_ptr, tbl_sec_ptr = raw_tbl_sec_ptr, rbb_ptr = ribbon_ptr, img_tab = t3, tbl_tab = t4, last_was_image = std::make_shared<bool>(false), last_was_table = std::make_shared<bool>(false)](const pluma::CursorState &state) {
        if (this->get_current_view() == view_ptr) {
          this->update_status_bar();
          this->update_ribbon_state(view_ptr, home_ptr);
          
          bool is_image = (state.object_type == pluma::CursorObjectType::Image);
          bool is_table = (state.object_type == pluma::CursorObjectType::Table || 
                           state.object_type == pluma::CursorObjectType::TableCell ||
                           state.object_type == pluma::CursorObjectType::TableRow ||
                           state.object_type == pluma::CursorObjectType::TableColumn);

          if (rbb_ptr) {
              rbb_ptr->set_tab_visible(img_tab, is_image);
              if (is_image && !(*last_was_image)) {
                  rbb_ptr->set_active_tab(img_tab);
              }
              *last_was_image = is_image;

              rbb_ptr->set_tab_visible(tbl_tab, is_table);
              if (is_table && !(*last_was_table)) {
                  rbb_ptr->set_active_tab(tbl_tab);
              }
              *last_was_table = is_table;
          }
          if (is_image && img_sec_ptr) {
              auto bag = view_ptr->editor()->getFormatRegistry().getStyleAt(state.logical_offset);
              auto wrap_val = bag.get(pluma::PropertyId::ImageWrapMode);
              pluma::TextWrapMode mode = pluma::TextWrapMode::InLine; // Default
              if (wrap_val) mode = std::get<pluma::TextWrapMode>(*wrap_val);
              img_sec_ptr->update_active_mode(mode);
          }
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

  auto bind_image_button = [this, view_ptr = raw_view_ptr](horizon::RibbonButton* btn) {
    btn->when_mouse_press.connect(
        [this, view_ptr](horizon::MouseButtonEventContext &ctx) {
          LOG_INFO << "Image button pressed!";
          if (view_ptr && view_ptr->editor()) {
            m_file_dialog = std::make_unique<horizon::FileDialog>(horizon::FileDialogMode::Open, horizon::i18n().tr("pluma-writer.dialogs.insert_image_title"));
            m_file_dialog->when_accepted.connect([this, view_ptr](horizon::FileDialogAcceptedContext& ctx_acc) {
              auto editor = view_ptr->editor();
              std::string img_tag = "\n|IMAGE:InLine:" + ctx_acc.selected_path + "|\n";
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
  };

  bind_image_button(m_home_sections.back()->btn_image());
  bind_image_button(m_insert_sections.back()->btn_image());

  // ── Insert Table vault ──────────────────────────────────────────────────
  auto bind_table_vault = [this, view_ptr = raw_view_ptr](horizon::RibbonButton* btn) {
    btn->when_mouse_press.connect(
        [this, view_ptr, btn](horizon::MouseButtonEventContext &) {
        constexpr int ROWS = 8;
        constexpr int COLS = 10;

        auto vault   = std::make_unique<horizon::Vault>();
        auto content = std::make_unique<horizon::Widget>();
        content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
        content->set_spacing(6);

        // Title label
        auto title = std::make_unique<horizon::Label>(horizon::i18n().tr("pluma-writer.ribbon.table"));
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
                                  btn);
      });
  };

  bind_table_vault(m_home_sections.back()->btn_table());
  bind_table_vault(m_insert_sections.back()->btn_table());
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

          auto title = std::make_unique<horizon::Label>(horizon::i18n().tr("pluma-writer.ribbon.text-color"));
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

          auto title = std::make_unique<horizon::Label>(horizon::i18n().tr("pluma-writer.ribbon.bg-color"));
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
  when_key_press.connect([this](horizon::KeyEventContext &ctx) {
      if ((ctx.modifiers & horizon::WaylandWindow::Modifier::CTRL) &&
          (ctx.modifiers & horizon::WaylandWindow::Modifier::SHIFT) &&
          (ctx.keysym == 'l' || ctx.keysym == 'L')) {
          if (m_lang_label) {
              horizon::MouseButtonEventContext mock_ctx;
              m_lang_label->when_mouse_press.run(mock_ctx);
          }
          ctx.stop_propagation = true;
      }
  });

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
    std::string text = "Pages: 1 of 1, Words: 0";
    set_status_text(text); if (m_status_label) { m_status_label->set_text(text); m_status_label->invalidate(); }
    return;
  }

  auto editor = view->editor();
  std::string text = editor->getText();
  uint32_t offset = editor->getCursorOffset();

  uint32_t current_page = editor->getCurrentPageNumber();
  size_t total_pages = editor->getPageCount();
  if (total_pages == 0) total_pages = 1;
  if (current_page > total_pages) current_page = total_pages;

  int word_count = 0;
  bool in_word = false;
  for (char c : text) {
      if (std::isspace(static_cast<unsigned char>(c))) {
          in_word = false;
      } else if (!in_word) {
          in_word = true;
          word_count++;
      }
  }

  char buf[256];
  snprintf(buf, sizeof(buf), "Pages: %u of %zu, Words: %d", current_page, total_pages, word_count);
  set_status_text(buf); if (m_status_label) { m_status_label->set_text(buf); m_status_label->invalidate(); }

  if (m_lang_label) {
      auto style_opt = editor->getFormatRegistry().getStyleAt(offset).get(pluma::PropertyId::Language);
      if (style_opt) {
          m_lang_label->set_text(std::get<std::string>(*style_opt));
      } else {
          m_lang_label->set_text("es"); // fallback
      }
      m_lang_label->invalidate();
  }
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
