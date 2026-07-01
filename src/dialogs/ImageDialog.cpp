#include "pluma/dialogs/ImageDialog.hpp"
#include <horizon/Button.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Window.hpp>
#include <algorithm>
#include <cmath>

namespace pluma_app {
namespace dialogs {

// Conversion constants
static constexpr double kCmToPt = 72.0 / 2.54;
static constexpr double kPtToCm = 2.54 / 72.0;

ImageDialog::ImageDialog()
    : horizon::WaylandWindow("pluma.dialog.image", 500, 400, false, false) {
    auto window_widget = std::make_unique<horizon::Window>(
        horizon::i18n().tr("pluma-writer.image_dialog.title"));

    auto content = std::make_unique<horizon::Widget>();
    content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    content->set_position_type(horizon::FILL);
    content->set_margin(10);
    content->set_spacing(10);

    auto notebook = std::make_unique<horizon::Notebook>();
    m_notebook = notebook.get();

    // --- Tab 1: Size ---
    auto size_tab = std::make_unique<horizon::Widget>();
    size_tab->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    size_tab->set_position_type(horizon::FILL);
    size_tab->set_margin(10);
    size_tab->set_spacing(10);

    // Dimensions section title
    auto dim_title = std::make_unique<horizon::Label>(
        horizon::i18n().tr("pluma-writer.image_dialog.dimensions"));
    dim_title->set_fixed_size(25);
    dim_title->set_font_weight(horizon::FontWeight::FONT_WEIGHT_BOLD);
    size_tab->add_child(std::move(dim_title));

    // Helper lambda for input rows (same pattern as ParagraphDialog)
    auto create_input_row = [](const std::string& label_text,
                               const std::string& unit_text,
                               horizon::TextBox<horizon::DoublePolicy>** out_box) {
        auto row = std::make_unique<horizon::Widget>();
        row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
        row->set_fixed_size(35);

        auto label = std::make_unique<horizon::Label>(label_text);
        label->set_fixed_size(100);
        row->add_child(std::move(label));

        auto box = std::make_unique<horizon::TextBox<horizon::DoublePolicy>>();
        box->set_fixed_size(120);
        box->config.show_spin_buttons = true;
        box->config.spin_step = 0.1;
        *out_box = box.get();
        row->add_child(std::move(box));

        auto unit = std::make_unique<horizon::Label>(unit_text);
        unit->set_fixed_size(40);
        row->add_child(std::move(unit));

        row->add_child(horizon::Spacer());
        return row;
    };

    // Width row
    size_tab->add_child(create_input_row(
        horizon::i18n().tr("pluma-writer.image_dialog.width"),
        horizon::i18n().tr("pluma-writer.image_dialog.cm"),
        &m_width_box));

    // Height row
    size_tab->add_child(create_input_row(
        horizon::i18n().tr("pluma-writer.image_dialog.height"),
        horizon::i18n().tr("pluma-writer.image_dialog.cm"),
        &m_height_box));

    // Aspect ratio checkbox
    auto check_row = std::make_unique<horizon::Widget>();
    check_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    check_row->set_position_type(horizon::FILL);
    check_row->set_fixed_size(36);

    auto aspect_check = std::make_unique<horizon::Checkbox<horizon::AquaObject>>();
    aspect_check->set_text(horizon::i18n().tr("pluma-writer.image_dialog.aspect_ratio"));
    aspect_check->set_fixed_size(300);
    aspect_check->set_checked(true);
    m_aspect_ratio_check = aspect_check.get();
    check_row->add_child(std::move(aspect_check));
    check_row->add_child(horizon::Spacer());

    size_tab->add_child(std::move(check_row));

    m_notebook->add_tab(horizon::NotebookPage(
        horizon::i18n().tr("pluma-writer.image_dialog.size"),
        std::move(size_tab)));

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
        [this](horizon::EventContext&) { this->on_close(); });

    auto btn_accept = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_accept->set_text(horizon::i18n().tr("pluma-writer.font_dialog.ok"));
    btn_accept->set_fixed_size(120);
    btn_accept->set_accent_color(horizon::WidgetAccentColor::Primary);
    btn_accept->when_click.connect([this](horizon::EventContext&) {
        ImageSizeEvent ev;
        ev.sender = this;
        float w_cm = 0, h_cm = 0;
        try { w_cm = std::stof(m_width_box->text()); } catch (...) {}
        try { h_cm = std::stof(m_height_box->text()); } catch (...) {}
        // Clamp to minimum 0.1 cm
        if (w_cm < 0.1f) w_cm = 0.1f;
        if (h_cm < 0.1f) h_cm = 0.1f;
        ev.width_pt = w_cm * kCmToPt;
        ev.height_pt = h_cm * kCmToPt;
        when_accepted.run(ev);
        this->on_close();
    });

    btn_row->add_child(std::move(btn_cancel));
    btn_row->add_child(std::move(btn_accept));
    content->add_child(std::move(btn_row));

    window_widget->add_child(std::move(content));
    set_root(std::move(window_widget));

    // Connect aspect-ratio updates
    m_width_box->when_text_changed.connect([this](horizon::EventContext&) {
        on_width_changed();
    });
    m_height_box->when_text_changed.connect([this](horizon::EventContext&) {
        on_height_changed();
    });
}

void ImageDialog::set_initial_size(float width_pt, float height_pt) {
    m_orig_width_pt = width_pt;
    m_orig_height_pt = height_pt;
    if (width_pt > 0 && height_pt > 0) {
        m_aspect_ratio = width_pt / height_pt;
    } else {
        m_aspect_ratio = 1.0f;
    }

    m_updating = true;
    m_width_box->set_text(std::to_string(width_pt * kPtToCm));
    m_height_box->set_text(std::to_string(height_pt * kPtToCm));
    m_updating = false;
}

void ImageDialog::on_width_changed() {
    if (m_updating) return;
    if (m_aspect_ratio_check->is_checked()) {
        apply_aspect_ratio(true);
    }
}

void ImageDialog::on_height_changed() {
    if (m_updating) return;
    if (m_aspect_ratio_check->is_checked()) {
        apply_aspect_ratio(false);
    }
}

void ImageDialog::apply_aspect_ratio(bool width_changed) {
    m_updating = true;
    try {
        if (width_changed) {
            float w_cm = std::stof(m_width_box->text());
            if (w_cm < 0.1f) w_cm = 0.1f;
            float h_cm = w_cm / m_aspect_ratio;
            // Keep 2 decimal places for cleaner display
            h_cm = std::round(h_cm * 100.0f) / 100.0f;
            m_height_box->set_text(std::to_string(h_cm));
        } else {
            float h_cm = std::stof(m_height_box->text());
            if (h_cm < 0.1f) h_cm = 0.1f;
            float w_cm = h_cm * m_aspect_ratio;
            w_cm = std::round(w_cm * 100.0f) / 100.0f;
            m_width_box->set_text(std::to_string(w_cm));
        }
    } catch (...) {}
    m_updating = false;
}

void ImageDialog::on_close() {
    this->quit();
}

} // namespace dialogs
} // namespace pluma_app
