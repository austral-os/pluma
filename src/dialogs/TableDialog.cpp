#include "pluma/dialogs/TableDialog.hpp"
#include <Ribbon/MultiToggleGroupButton.hpp>
#include <horizon/Button.hpp>
#include <horizon/ColorSelector.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Icon.hpp>
#include <horizon/Label.hpp>
#include <horizon/Notification.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Window.hpp>
#include <horizon/ThemeManager.hpp>
#include <horizon/dialogs/FileDialog.hpp>

// Forward declaration of TableBordersPreview since it will be in its own file
#include "pluma/Widgets/TableBordersPreview.hpp"

namespace pluma_app {
namespace dialogs {

TableDialog::TableDialog()
    : horizon::WaylandWindow("pluma.dialog.table", 700, 490, false, false) {

  auto create_input_row = [](const std::string &label_text,
                             std::unique_ptr<horizon::Widget> widget,
                             float width = 200) {
    auto row = std::make_unique<horizon::Widget>();
    row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    row->set_fixed_size(38);

    auto label = std::make_unique<horizon::Label>(label_text);
    label->set_fixed_size(150);
    row->add_child(std::move(label));

    if (width > 0) {
      widget->set_fixed_size(width);
    }
    row->add_child(std::move(widget));
    return row;
  };

  auto window_widget = std::make_unique<horizon::Window>(
      horizon::i18n().tr("pluma-writer.table_dialog.title"));

  auto main_container = std::make_unique<horizon::Widget>();
  main_container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  main_container->set_margin(10);
  main_container->set_position_type(horizon::FILL);

  // Notebook
  auto notebook = std::make_unique<horizon::Notebook>();
  m_notebook = notebook.get();

  // ----------------------------------------------------
  // Tab: Borders
  // ----------------------------------------------------
  auto borders_tab = std::make_unique<horizon::Widget>();
  borders_tab->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  borders_tab->set_position_type(horizon::FILL);
  borders_tab->set_margin(10);
  borders_tab->set_spacing(20);

  auto left_pane = std::make_unique<horizon::Widget>();
  left_pane->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  left_pane->set_position_type(horizon::FILL);
  left_pane->set_spacing(20);

  auto lbl_arrangement = std::make_unique<horizon::Label>(
      horizon::i18n().tr("pluma-writer.table_dialog.line_arrangement"));
  lbl_arrangement->set_font_weight(horizon::FontWeight::FONT_WEIGHT_BOLD);
  lbl_arrangement->set_fixed_size(25);
  left_pane->add_child(std::move(lbl_arrangement));

  auto btn_presets = std::make_unique<MultiToggleGroupButton>();
  // Icons for each preset
  auto add_preset = [&](const std::string &icon_name) {
    auto icon = std::make_unique<horizon::Icon>();
    icon->set_icon_name(icon_name);
    icon->set_icon_size(
        24); // SVG might be 16x16 natively, scaling up to 24 is safe.
    icon->set_use_theme_colors(false);
    btn_presets->add_item(std::move(icon), 40);
  };

  add_preset("pluma-tbl-brd-none");
  add_preset("pluma-tbl-brd-outer");
  add_preset("pluma-tbl-brd-outer-h");
  add_preset("pluma-tbl-brd-outer-all");
  add_preset("pluma-tbl-brd-outer-wocil");

  auto set_tooltip = [](horizon::Widget *w, const std::string &msg) {
    auto t = std::make_unique<horizon::Notification>();
    t->set_message(msg);
    w->set_tooltip(std::move(t));
  };

  if (btn_presets->children().size() >= 5) {
    set_tooltip(btn_presets->children()[0].get(),
                horizon::i18n().tr("pluma-writer.table_dialog.preset_none"));
    set_tooltip(btn_presets->children()[1].get(),
                horizon::i18n().tr("pluma-writer.table_dialog.preset_outer"));
    set_tooltip(
        btn_presets->children()[2].get(),
        horizon::i18n().tr("pluma-writer.table_dialog.preset_outer_horiz"));
    set_tooltip(btn_presets->children()[3].get(),
                horizon::i18n().tr("pluma-writer.table_dialog.preset_all"));
    set_tooltip(
        btn_presets->children()[4].get(),
        horizon::i18n().tr("pluma-writer.table_dialog.preset_outer_no_inner"));
  }
  m_presets_group = btn_presets.get();

  left_pane->add_child(
      create_input_row(horizon::i18n().tr("pluma-writer.table_dialog.presets"),
                       std::move(btn_presets), 210));

  auto lbl_line = std::make_unique<horizon::Label>(
      horizon::i18n().tr("pluma-writer.table_dialog.line"));
  lbl_line->set_font_weight(horizon::FontWeight::FONT_WEIGHT_BOLD);
  lbl_line->set_fixed_size(25);
  left_pane->add_child(std::move(lbl_line));

  auto cb_style = std::make_unique<horizon::Combo>();
  m_style_combo = cb_style.get();
  left_pane->add_child(
      create_input_row(horizon::i18n().tr("pluma-writer.table_dialog.style"),
                       std::move(cb_style)));

  auto cs_color = std::make_unique<horizon::ColorSelector>();
  horizon::Color default_color(0.0f, 0.0f, 0.0f, 1.0f);
  if (auto tm = horizon::theme_manager()) {
      default_color = tm->get_color("textbox_fg");
  }
  cs_color->set_color(default_color);
  m_color_selector = cs_color.get();
  left_pane->add_child(
      create_input_row(horizon::i18n().tr("pluma-writer.table_dialog.color"),
                       std::move(cs_color)));

  auto cb_thickness = std::make_unique<horizon::Combo>();
  m_thickness_combo = cb_thickness.get();
  left_pane->add_child(create_input_row(
      horizon::i18n().tr("pluma-writer.table_dialog.thickness"),
      std::move(cb_thickness)));

  // Right pane: User-defined preview
  auto right_pane = std::make_unique<horizon::Widget>();
  right_pane->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  right_pane->set_fixed_size(250);
  right_pane->set_spacing(10);

  auto lbl_user_defined = std::make_unique<horizon::Label>(
      horizon::i18n().tr("pluma-writer.table_dialog.user_defined"));
  lbl_user_defined->set_font_weight(horizon::FontWeight::FONT_WEIGHT_BOLD);
  lbl_user_defined->set_fixed_size(25);
  right_pane->add_child(std::move(lbl_user_defined));

  auto preview = std::make_unique<Widgets::TableBordersPreview>();
  preview->set_position_type(horizon::FILL);
  m_preview = preview.get();
  right_pane->add_child(std::move(preview));
  right_pane->add_child(horizon::Spacer(100));

  m_style_combo->when_item_selected.connect([this](horizon::ComboItemSelectedContext &) {
    if (m_preview) m_preview->set_line_style(m_style_combo->selected_item_index());
  });

  m_color_selector->when_color_changed.connect([this](horizon::ColorPickerDialogAcceptedContext &) {
    if (m_preview) m_preview->set_line_color(m_color_selector->color());
  });

  m_thickness_combo->when_item_selected.connect([this](horizon::ComboItemSelectedContext &) {
    if (m_preview) {
        float thick = 1.0f;
        int idx = m_thickness_combo->selected_item_index();
        if (idx == 0) thick = 0.5f;
        else if (idx == 1) thick = 1.0f;
        else if (idx == 2) thick = 1.5f;
        else if (idx == 3) thick = 2.0f;
        else if (idx == 4) thick = 2.5f;
        else if (idx == 5) thick = 3.0f;
        else if (idx == 6) thick = 4.0f;
        else if (idx == 7) thick = 5.0f;
        m_preview->set_line_thickness(thick);
    }
  });

  borders_tab->add_child(std::move(left_pane));
  borders_tab->add_child(std::move(right_pane));

  static_cast<horizon::GroupButton *>(m_presets_group)
      ->when_button_clicked.connect([this](horizon::GroupButtonClickEvent &ev) {
        for (int i = 0; i < 5; ++i) {
          static_cast<MultiToggleGroupButton *>(m_presets_group)
              ->set_item_active(i, i == ev.button_index);
        }
        if (m_preview)
          m_preview->set_preset(ev.button_index);
      });

  m_preview->when_user_changed.connect([this](horizon::EventContext &) {
    // Deselect all presets if possible, or just ignore (MultiToggleGroupButton
    // doesn't have an easy public API to reset all but we can add one or leave
    // it)
    for (int i = 0; i < 5; ++i) {
      static_cast<MultiToggleGroupButton *>(m_presets_group)
          ->set_item_active(i, false);
    }
  });

  m_notebook->add_tab(horizon::NotebookPage(
      horizon::i18n().tr("pluma-writer.table_dialog.borders"),
      std::move(borders_tab)));

  main_container->add_child(std::move(notebook));

  // Action buttons (Ok / Cancel)
  auto actions_container = std::make_unique<horizon::Widget>();
  actions_container->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  actions_container->set_fixed_size(33);
  actions_container->set_spacing(10);
  actions_container->add_child(horizon::Spacer()); // Align buttons to the right

  auto btn_cancel = std::make_unique<horizon::Button<horizon::AquaObject>>();
  btn_cancel->set_text(horizon::i18n().tr("pluma-writer.table_dialog.cancel"));
  btn_cancel->set_fixed_size(120);
  btn_cancel->when_click.connect(
      [this](horizon::EventContext &) { on_close(); });

  auto btn_ok = std::make_unique<horizon::Button<horizon::AquaObject>>();
  btn_ok->set_text(horizon::i18n().tr("pluma-writer.table_dialog.ok"));
  btn_ok->set_fixed_size(120);
  btn_ok->set_accent_color(horizon::WidgetAccentColor::Primary);
  btn_ok->when_click.connect([this](horizon::EventContext &) {
    TableBordersEvent ev;
    ev.sender = this;
    when_accepted.run(ev);
    on_close();
  });

  actions_container->add_child(std::move(btn_cancel));
  actions_container->add_child(std::move(btn_ok));

  main_container->add_child(std::move(actions_container));

  window_widget->add_child(std::move(main_container));
  set_root(std::move(window_widget));

  populate_combos();
}

void TableDialog::populate_combos() {
  // Populate Styles
  m_style_combo->add_item(
      "solid", horizon::i18n().tr("pluma-writer.table_dialog.style_solid"));
  m_style_combo->add_item(
      "dashed", horizon::i18n().tr("pluma-writer.table_dialog.style_dashed"));
  m_style_combo->add_item(
      "dotted", horizon::i18n().tr("pluma-writer.table_dialog.style_dotted"));
  m_style_combo->add_item(
      "dash_dot",
      horizon::i18n().tr("pluma-writer.table_dialog.style_dash_dot"));
  m_style_combo->add_item(
      "dash_dot_dot",
      horizon::i18n().tr("pluma-writer.table_dialog.style_dash_dot_dot"));
  m_style_combo->add_item(
      "double", horizon::i18n().tr("pluma-writer.table_dialog.style_double"));
  m_style_combo->set_selected_item_index(0);

  // Populate Thickness
  m_thickness_combo->add_item(
      "0.5pt", horizon::i18n().tr("pluma-writer.table_dialog.thick_05"));
  m_thickness_combo->add_item(
      "1pt", horizon::i18n().tr("pluma-writer.table_dialog.thick_10"));
  m_thickness_combo->add_item(
      "2pt", horizon::i18n().tr("pluma-writer.table_dialog.thick_20"));
  m_thickness_combo->add_item(
      "3pt", horizon::i18n().tr("pluma-writer.table_dialog.thick_30"));
  m_thickness_combo->add_item(
      "4pt", horizon::i18n().tr("pluma-writer.table_dialog.thick_40"));
  m_thickness_combo->set_selected_item_index(0);
}

void TableDialog::on_close() { this->quit(); }

} // namespace dialogs
} // namespace pluma_app
