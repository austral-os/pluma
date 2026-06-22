#include "pluma/dialogs/ParagraphDialog.hpp"
#include <horizon/Button.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Window.hpp>
#include <pluma/Widgets/ParagraphPreview.hpp>

namespace pluma_app {
namespace dialogs {

ParagraphDialog::ParagraphDialog()
    : horizon::WaylandWindow("pluma.dialog.paragraph", 700, 600, false, false) {
  auto window_widget = std::make_unique<horizon::Window>(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.title"));

  auto content = std::make_unique<horizon::Widget>();
  content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  content->set_position_type(horizon::FILL);
  content->set_margin(10);

  auto notebook = std::make_unique<horizon::Notebook>();
  m_notebook = notebook.get();

  // --- Tab 1: Indents & Spacing ---
  auto indents_tab = std::make_unique<horizon::Widget>();
  indents_tab->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  indents_tab->set_position_type(horizon::FILL);
  indents_tab->set_margin(10);
  indents_tab->set_spacing(20);

  auto left_pane = std::make_unique<horizon::Widget>();
  left_pane->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  left_pane->set_position_type(horizon::FILL);
  left_pane->set_spacing(10);

  // Helper lambda to create a row with a label and a numeric textbox
  auto create_input_row = [](const std::string &label_text,
                             horizon::TextBox<horizon::DoublePolicy> **out_box,
                             double step = 0.1) {
    auto row = std::make_unique<horizon::Widget>();
    row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    row->set_fixed_size(35);

    auto label = std::make_unique<horizon::Label>(label_text);
    label->set_fixed_size(150);
    row->add_child(std::move(label));

    auto box = std::make_unique<horizon::TextBox<horizon::DoublePolicy>>();
    box->set_fixed_size(120);
    box->config.show_spin_buttons = true;
    box->config.spin_step = step;
    *out_box = box.get();
    row->add_child(std::move(box));

    return row;
  };

  // Indent section
  auto indent_title = std::make_unique<horizon::Label>(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.indent"));
  indent_title->set_fixed_size(25);
  indent_title->set_font_weight(horizon::FontWeight::FONT_WEIGHT_BOLD);
  left_pane->add_child(std::move(indent_title));

  left_pane->add_child(create_input_row(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.before_text"),
      &m_indent_before_box));
  left_pane->add_child(create_input_row(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.after_text"),
      &m_indent_after_box));
  left_pane->add_child(create_input_row(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.first_line"),
      &m_indent_first_box));

  // Spacing section
  auto spacing_title = std::make_unique<horizon::Label>(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.spacing"));
  spacing_title->set_font_weight(horizon::FontWeight::FONT_WEIGHT_BOLD);
  spacing_title->set_fixed_size(25);
  left_pane->add_child(std::move(spacing_title));

  left_pane->add_child(create_input_row(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.above_paragraph"),
      &m_spacing_above_box));
  left_pane->add_child(create_input_row(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.below_paragraph"),
      &m_spacing_below_box));

  // Line spacing section
  auto ls_title = std::make_unique<horizon::Label>(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.line_spacing"));
  ls_title->set_fixed_size(25);
  ls_title->set_font_weight(horizon::FontWeight::FONT_WEIGHT_BOLD);
  left_pane->add_child(std::move(ls_title));

  auto ls_row = std::make_unique<horizon::Widget>();
  ls_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  ls_row->set_fixed_size(35);
  ls_row->set_spacing(10);

  auto ls_combo = std::make_unique<horizon::Combo>();
  ls_combo->set_fixed_size(150);
  m_line_spacing_combo = ls_combo.get();
  ls_row->add_child(std::move(ls_combo));

  auto ls_box = std::make_unique<horizon::TextBox<horizon::DoublePolicy>>();
  ls_box->set_fixed_size(120);
  ls_box->config.show_spin_buttons = true;
  ls_box->config.spin_step = 0.1;
  m_line_spacing_value_box = ls_box.get();
  ls_row->add_child(std::move(ls_box));

  left_pane->add_child(std::move(ls_row));

  auto right_pane = std::make_unique<horizon::Widget>();
  right_pane->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  right_pane->set_fixed_size(250);

  auto preview = std::make_unique<widgets::ParagraphPreview>();
  preview->set_position_type(horizon::FILL);
  m_preview = preview.get();
  right_pane->add_child(std::move(preview));

  indents_tab->add_child(std::move(left_pane));
  indents_tab->add_child(std::move(right_pane));

  m_notebook->add_tab(horizon::NotebookPage(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.indents_spacing"),
      std::move(indents_tab)));
  m_notebook->add_tab(horizon::NotebookPage(
      horizon::i18n().tr("pluma-writer.paragraph_dialog.alignment"),
      std::make_unique<horizon::Widget>()));

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
    ParagraphSelectedEvent ev;
    ev.sender = this;
    try {
      ev.indent_before = std::stof(m_indent_before_box->text());
    } catch (...) {
    }
    try {
      ev.indent_after = std::stof(m_indent_after_box->text());
    } catch (...) {
    }
    try {
      ev.indent_first_line = std::stof(m_indent_first_box->text());
    } catch (...) {
    }
    try {
      ev.spacing_above = std::stof(m_spacing_above_box->text());
    } catch (...) {
    }
    try {
      ev.spacing_below = std::stof(m_spacing_below_box->text());
    } catch (...) {
    }

    // For line spacing, parse combo or value box.
    // Usually combo ID is "single" (1.0), "1.5" (1.5), "double" (2.0)
    if (m_line_spacing_combo->selected_item()) {
      std::string ls_id = m_line_spacing_combo->selected_item()->id;
      if (ls_id == "Single")
        ev.line_spacing = 1.0f;
      else if (ls_id == "1.5 lines")
        ev.line_spacing = 1.5f;
      else if (ls_id == "Double")
        ev.line_spacing = 2.0f;
      else {
        try {
          ev.line_spacing = std::stof(m_line_spacing_value_box->text());
        } catch (...) {
        }
      }
    }

    when_accepted.run(ev);
    this->on_close();
  });

  btn_row->add_child(std::move(btn_cancel));
  btn_row->add_child(std::move(btn_accept));
  content->add_child(std::move(btn_row));

  window_widget->add_child(std::move(content));
  set_root(std::move(window_widget));

  populate_combos();

  auto update_cb = [this](auto &) { update_preview(); };
  m_indent_before_box->when_text_changed.connect(update_cb);
  m_indent_after_box->when_text_changed.connect(update_cb);
  m_indent_first_box->when_text_changed.connect(update_cb);
  m_spacing_above_box->when_text_changed.connect(update_cb);
  m_spacing_below_box->when_text_changed.connect(update_cb);
  m_line_spacing_combo->when_item_selected.connect([this](auto &) {
    std::string ls_id = m_line_spacing_combo->selected_item()->id;
    if (ls_id == "Single" || ls_id == "1.5 lines" || ls_id == "Double") {
      m_line_spacing_value_box->set_text(""); // Clear value
    }
    update_preview();
  });
  m_line_spacing_value_box->when_text_changed.connect(update_cb);
}

void ParagraphDialog::populate_combos() {
  m_line_spacing_combo->add_item(
      "Single", horizon::i18n().tr("pluma-writer.paragraph_dialog.ls_single"));
  m_line_spacing_combo->add_item(
      "1.5 lines", horizon::i18n().tr("pluma-writer.paragraph_dialog.ls_15"));
  m_line_spacing_combo->add_item(
      "Double", horizon::i18n().tr("pluma-writer.paragraph_dialog.ls_double"));
  m_line_spacing_combo->add_item(
      "Proportional",
      horizon::i18n().tr("pluma-writer.paragraph_dialog.ls_proportional"));
  m_line_spacing_combo->set_selected_item_index(0);
}

void ParagraphDialog::update_preview() {
  float before = 0, after = 0, first = 0, above = 0, below = 0, ls = 1.0f;
  try {
    before = std::stof(m_indent_before_box->text());
  } catch (...) {
  }
  try {
    after = std::stof(m_indent_after_box->text());
  } catch (...) {
  }
  try {
    first = std::stof(m_indent_first_box->text());
  } catch (...) {
  }
  try {
    above = std::stof(m_spacing_above_box->text());
  } catch (...) {
  }
  try {
    below = std::stof(m_spacing_below_box->text());
  } catch (...) {
  }

  if (m_line_spacing_combo->selected_item()) {
    std::string ls_id = m_line_spacing_combo->selected_item()->id;
    if (ls_id == "Single")
      ls = 1.0f;
    else if (ls_id == "1.5 lines")
      ls = 1.5f;
    else if (ls_id == "Double")
      ls = 2.0f;
    else {
      try {
        ls = std::stof(m_line_spacing_value_box->text());
      } catch (...) {
      }
    }
  }

  m_preview->set_indent_before(before);
  m_preview->set_indent_after(after);
  m_preview->set_indent_first_line(first);
  m_preview->set_spacing_above(above);
  m_preview->set_spacing_below(below);
  m_preview->set_line_spacing(ls);
}

void ParagraphDialog::set_initial_paragraph(
    float indent_before, float indent_after, float indent_first,
    float spacing_above, float spacing_below, float line_spacing) {
  m_indent_before_box->set_text(std::to_string(indent_before));
  m_indent_after_box->set_text(std::to_string(indent_after));
  m_indent_first_box->set_text(std::to_string(indent_first));
  m_spacing_above_box->set_text(std::to_string(spacing_above));
  m_spacing_below_box->set_text(std::to_string(spacing_below));

  if (line_spacing == 1.0f) {
    m_line_spacing_combo->set_selected_item_by_id("Single");
  } else if (line_spacing == 1.5f) {
    m_line_spacing_combo->set_selected_item_by_id("1.5 lines");
  } else if (line_spacing == 2.0f) {
    m_line_spacing_combo->set_selected_item_by_id("Double");
  } else {
    m_line_spacing_combo->set_selected_item_by_id("Proportional");
    m_line_spacing_value_box->set_text(std::to_string(line_spacing));
  }

  update_preview();
}

void ParagraphDialog::on_close() { this->quit(); }

} // namespace dialogs
} // namespace pluma_app
