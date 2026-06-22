#include "pluma/dialogs/FontDialog.hpp"
#include "utils/FontUtils.hpp"
#include <algorithm>
#include <filesystem>
#include <horizon/Button.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/TableColumn.hpp>
#include <horizon/Widget.hpp>
#include <unicode/locid.h>
#include <vector>

namespace pluma_app {
namespace dialogs {

FontDialog::FontDialog()
    : horizon::WaylandWindow("pluma.dialog.font", 800, 750, false, false) {
  auto window_widget = std::make_unique<horizon::Window>(horizon::i18n().tr("pluma-writer.font_dialog.character"));

  auto content = std::make_unique<horizon::Widget>();
  content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  content->set_position_type(horizon::FILL);
  content->set_margin(10);
  // content->set_spacing(10);

  auto notebook = std::make_unique<horizon::Notebook>();
  m_notebook = notebook.get();

  // --- Tab 1: Font ---
  auto font_tab = std::make_unique<horizon::Widget>();
  font_tab->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  font_tab->set_position_type(horizon::FILL);
  font_tab->set_margin(10);
  font_tab->set_spacing(10);

  auto family_label =
      std::make_unique<horizon::Label>(horizon::i18n().tr("pluma-writer.font_dialog.family"));
  family_label->set_fixed_size(25);
  font_tab->add_child(std::move(family_label));

  auto search_box = std::make_unique<horizon::TextBox<horizon::TextPolicy>>();
  search_box->set_fixed_size(35);
  m_search_box = search_box.get();

  font_tab->add_child(std::move(search_box));

  auto font_table = std::make_unique<horizon::TableView<std::string>>();
  font_table->set_width_mode(horizon::TableViewWidthMode::Fill);
  horizon::TableColumn<std::string> col1;
  col1.id = "name";
  col1.title = "Family";
  col1.width = -1;
  col1.cell_factory = [](const std::string &item) {
    return std::make_unique<horizon::Label>(item);
  };
  font_table->add_column(std::move(col1));
  font_table->set_header_visible(false);
  font_table->set_position_type(horizon::FILL);
  m_font_table = font_table.get();
  font_tab->add_child(std::move(font_table));

  auto style_label =
      std::make_unique<horizon::Label>(horizon::i18n().tr("pluma-writer.font_dialog.style"));
  style_label->set_fixed_size(25);
  font_tab->add_child(std::move(style_label));

  auto style_combo = std::make_unique<horizon::Combo>();
  style_combo->set_fixed_size(35);
  m_style_combo = style_combo.get();
  font_tab->add_child(std::move(style_combo));

  auto size_label =
      std::make_unique<horizon::Label>(horizon::i18n().tr("pluma-writer.font_dialog.size"));
  size_label->set_fixed_size(25);
  font_tab->add_child(std::move(size_label));

  auto size_combo = std::make_unique<horizon::Combo>();
  size_combo->set_fixed_size(35);
  m_size_combo = size_combo.get();
  font_tab->add_child(std::move(size_combo));

  auto lang_label =
      std::make_unique<horizon::Label>(horizon::i18n().tr("pluma-writer.font_dialog.language"));
  lang_label->set_fixed_size(25);
  font_tab->add_child(std::move(lang_label));

  auto lang_combo = std::make_unique<horizon::Combo>();
  lang_combo->set_fixed_size(35);
  m_lang_combo = lang_combo.get();
  font_tab->add_child(std::move(lang_combo));

  // Preview
  auto preview = std::make_unique<widgets::FontPreview>();
  preview->set_position_type(horizon::FILL);
  preview->set_fixed_size(120);
  m_preview = preview.get();
  font_tab->add_child(std::move(preview));

  // Add Tabs
  m_notebook->add_tab(
      horizon::NotebookPage(horizon::i18n().tr("pluma-writer.font_dialog.font"), std::move(font_tab)));
  m_notebook->add_tab(horizon::NotebookPage(
      horizon::i18n().tr("pluma-writer.font_dialog.font_effects"), std::make_unique<horizon::Widget>()));
  m_notebook->add_tab(horizon::NotebookPage(
      horizon::i18n().tr("pluma-writer.font_dialog.position"), std::make_unique<horizon::Widget>()));
  m_notebook->add_tab(horizon::NotebookPage(
      horizon::i18n().tr("pluma-writer.font_dialog.highlighting"), std::make_unique<horizon::Widget>()));
  m_notebook->add_tab(horizon::NotebookPage(
      horizon::i18n().tr("pluma-writer.font_dialog.borders"), std::make_unique<horizon::Widget>()));

  content->add_child(std::move(notebook));

  // Buttons
  auto btn_row = std::make_unique<horizon::Widget>();
  btn_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  btn_row->set_fixed_size(33);
  btn_row->set_spacing(10);

  btn_row->add_child(horizon::Spacer());

  auto btn_cancel = std::make_unique<horizon::Button<horizon::AquaObject>>();
  btn_cancel->set_text(horizon::i18n().tr("pluma-writer.font_dialog.cancel"));
  btn_cancel->set_fixed_size(120);
  btn_cancel->when_click.connect(
      [this](horizon::EventContext &) { this->on_close(); });

  auto btn_accept = std::make_unique<horizon::Button<horizon::AquaObject>>();
  btn_accept->set_text(horizon::i18n().tr("pluma-writer.font_dialog.ok"));
  btn_accept->set_fixed_size(120);
  btn_accept->set_accent_color(horizon::WidgetAccentColor::Primary);
  btn_accept->when_click.connect([this](horizon::EventContext &) {
    FontSelectedEvent ev;
    ev.sender = this;
    ev.family = m_search_box->text();

    std::string size_str = "12";
    if (m_size_combo->selected_item()) {
      size_str = m_size_combo->selected_item()->id;
    }
    float size = 12.0f;
    try {
      size = std::stof(size_str);
    } catch (...) {
    }
    ev.size = size;

    bool bold = false;
    bool italic = false;
    if (m_style_combo->selected_item()) {
      std::string style_id = m_style_combo->selected_item()->id;
      if (style_id == "Bold" || style_id == "Bold Italic")
        bold = true;
      if (style_id == "Italic" || style_id == "Bold Italic")
        italic = true;
    }
    ev.bold = bold;
    ev.italic = italic;

    if (m_lang_combo->selected_item()) {
      ev.language = m_lang_combo->selected_item()->id;
    }

    when_accepted.run(ev);
    this->on_close();
  });

  btn_row->add_child(std::move(btn_cancel));
  btn_row->add_child(std::move(btn_accept));

  content->add_child(std::move(btn_row));

  window_widget->add_child(std::move(content));
  set_root(std::move(window_widget));

  populate_fonts();
  populate_styles();
  populate_sizes();
  populate_languages();

  // Hook up search filter
  auto update_preview = [this]() {
    std::string family = m_search_box->text();
    std::string size_str = "12";
    if (m_size_combo->selected_item()) {
      size_str = m_size_combo->selected_item()->id;
    }
    float size = 12.0f;
    try {
      size = std::stof(size_str);
    } catch (...) {
    }

    bool bold = false;
    bool italic = false;
    if (m_style_combo->selected_item()) {
      std::string style_id = m_style_combo->selected_item()->id;
      if (style_id == "Bold" || style_id == "Bold Italic")
        bold = true;
      if (style_id == "Italic" || style_id == "Bold Italic")
        italic = true;
    }

    if (m_preview) {
      m_preview->set_preview_font(family, size, bold, italic);
    }
  };

  m_style_combo->when_item_selected.connect(
      [this, update_preview](horizon::ComboItemSelectedContext &) {
        update_preview();
      });

  m_size_combo->when_item_selected.connect(
      [this, update_preview](horizon::ComboItemSelectedContext &) {
        update_preview();
      });

  m_search_box->when_text_changed.connect(
      [this, update_preview](horizon::EventContext &) {
        std::string query = m_search_box->text();
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);
        auto fonts = utils::FontUtils::get_installed_fonts();
        if (query.empty()) {
          m_font_table->set_data(fonts);
        } else {
          std::vector<std::string> filtered;
          for (const auto &f : fonts) {
            std::string f_lower = f;
            std::transform(f_lower.begin(), f_lower.end(), f_lower.begin(),
                           ::tolower);
            if (f_lower.find(query) != std::string::npos) {
              filtered.push_back(f);
            }
          }
          m_font_table->set_data(filtered);
        }
        update_preview();
      });

  m_font_table->when_row_click.connect([this, update_preview](
                                           horizon::EventContext &ctx) {
    auto &ev =
        static_cast<horizon::TableViewRowMouseClickContext<std::string> &>(ctx);
    m_search_box->set_text(ev.row_data);
    update_preview();
  });

  update_preview();
}

void FontDialog::populate_fonts() {
  auto fonts = utils::FontUtils::get_installed_fonts();
  m_font_table->set_data(fonts);
  int selected_idx = 0;
  for (size_t i = 0; i < fonts.size(); ++i) {
    if (fonts[i] == "Arial" || fonts[i] == "Liberation Serif" ||
        fonts[i] == "Ubuntu") {
      selected_idx = i;
      break;
    }
  }
  if (!fonts.empty()) {
    m_font_table->set_selected_index(selected_idx);
    m_font_table->scroll_to_index(selected_idx);
    m_search_box->set_text(fonts[selected_idx]);
  }
}

void FontDialog::populate_sizes() {
  std::vector<std::string> sizes = {"10", "12", "14", "16", "18",
                                    "24", "36", "48", "72"};
  for (const auto &s : sizes) {
    m_size_combo->add_item(s, s);
  }
  m_size_combo->set_selected_item_index(1); // 12
}

void FontDialog::populate_styles() {
  m_style_combo->add_item("Regular", horizon::i18n().tr("pluma-writer.font_dialog.regular"));
  m_style_combo->add_item("Bold", horizon::i18n().tr("pluma-writer.font_dialog.bold"));
  m_style_combo->add_item("Italic", horizon::i18n().tr("pluma-writer.font_dialog.italic"));
  m_style_combo->add_item("Bold Italic", horizon::i18n().tr("pluma-writer.font_dialog.bold_italic"));
  m_style_combo->set_selected_item_index(0);
}

void FontDialog::populate_languages() {
  struct LanguageItem {
    std::string id;
    std::string display_name;
  };
  std::vector<LanguageItem> all_langs;

  std::vector<std::string> search_paths = {
      "dictionaries/", "build/dictionaries/", "assets/dictionaries/",
      "/usr/share/pluma-writer/dictionaries/",
      "/usr/local/share/pluma-writer/dictionaries/"};
  std::string active_path;
  for (const auto &path : search_paths) {
    if (std::filesystem::exists(path)) {
      active_path = path;
      break;
    }
  }
  if (!active_path.empty()) {
    try {
      for (const auto &entry :
           std::filesystem::directory_iterator(active_path)) {
        if (entry.path().extension() == ".aff") {
          std::string lang = entry.path().stem().string();
          icu::Locale loc(lang.c_str());
          icu::UnicodeString ustr;
          loc.getDisplayName(loc, ustr);
          std::string disp;
          ustr.toUTF8String(disp);
          if (disp.empty())
            disp = lang;
          all_langs.push_back({lang, disp});
        }
      }
    } catch (...) {
    }
  }

  std::sort(all_langs.begin(), all_langs.end(),
            [](const LanguageItem &a, const LanguageItem &b) {
              return a.display_name < b.display_name;
            });

  for (const auto &l : all_langs) {
    m_lang_combo->add_item(l.id, l.display_name);
  }
  if (!all_langs.empty()) {
    m_lang_combo->set_selected_item_index(0);
  }
}

void FontDialog::set_initial_language(const std::string &lang_id) {
  if (!lang_id.empty()) {
    m_lang_combo->set_selected_item_by_id(lang_id);
  }
}

void FontDialog::set_initial_font(const std::string &family, float size,
                                  bool bold, bool italic) {
  if (!family.empty()) {
    m_search_box->set_text(family);
    const auto &items = m_font_table->data();
    for (size_t i = 0; i < items.size(); ++i) {
      if (items[i] == family) {
        m_font_table->set_selected_index(i);
        break;
      }
    }
  }

  std::string size_str = std::to_string((int)size);
  m_size_combo->set_selected_item_by_id(size_str);

  std::string style_id = "Regular";
  if (bold && italic)
    style_id = "Bold Italic";
  else if (bold)
    style_id = "Bold";
  else if (italic)
    style_id = "Italic";
  m_style_combo->set_selected_item_by_id(style_id);

  if (m_preview) {
    m_preview->set_preview_font(family, size, bold, italic);
  }
}

} // namespace dialogs
} // namespace pluma_app
