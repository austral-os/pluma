#pragma once

#include "PlumaColorSchemePreferences.hpp"

#include <horizon/Combo.hpp>
#include <horizon/ConfigSection.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/ThemeManager.hpp>
#include <horizon/Widget.hpp>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>

namespace pluma_app {

class PlumaGeneralSection : public horizon::Widget, public horizon::ConfigSection {
public:
    explicit PlumaGeneralSection(std::function<void()> on_change)
        : horizon::Widget(), m_on_change(std::move(on_change)) {
        set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
        set_margin(24);
        set_spacing(15);

        auto label = std::make_unique<horizon::Label>(
            horizon::i18n().tr("pluma-writer.preferences.color_scheme"));
        label->set_font_weight(horizon::FONT_WEIGHT_BOLD);
        label->set_fixed_size(22);
        add_child(std::move(label));

        auto row = std::make_unique<horizon::Widget>();
        row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
        row->set_fixed_size(30);

        auto combo = std::make_unique<horizon::Combo>();
        combo->set_width(250);
        combo->set_fixed_size(250);
        combo->add_item("default", horizon::i18n().tr("pluma-writer.preferences.color_scheme_default"));

        for (const auto &variant : horizon::ThemeManager::instance().app_color_scheme_variants("pluma-writer")) {
            if (variant == "default") {
                continue;
            }
            combo->add_item(variant, variant == "office-2007"
                                         ? horizon::i18n().tr("pluma-writer.preferences.color_scheme_office_2007")
                                         : variant);
        }

        m_color_scheme_combo = combo.get();
        m_color_scheme_combo->when_item_selected.connect([this](const horizon::ComboItemSelectedContext &ctx) {
            if (m_loading) {
                return;
            }

            std::string variant = ctx.item.id;
            if (!horizon::ThemeManager::instance().set_app_color_scheme_variant("pluma-writer", variant)) {
                variant = "default";
                horizon::ThemeManager::instance().set_app_color_scheme_variant("pluma-writer", variant);
                m_loading = true;
                m_color_scheme_combo->set_selected_item_by_id(variant);
                m_loading = false;
            }

            if (m_on_change) {
                m_on_change();
            }
        });

        row->add_child(std::move(combo));
        row->add_child(horizon::Spacer());
        add_child(std::move(row));
        add_child(horizon::Spacer());
    }

    void from_json(const nlohmann::json &j) override {
        m_loading = true;

        std::string variant = color_scheme_preferences::variant_from_json(
            j, horizon::ThemeManager::instance().app_color_scheme_variants("pluma-writer"));

        if (!horizon::ThemeManager::instance().set_app_color_scheme_variant("pluma-writer", variant)) {
            variant = color_scheme_preferences::kDefaultVariant;
            horizon::ThemeManager::instance().set_app_color_scheme_variant("pluma-writer", variant);
        }
        m_color_scheme_combo->set_selected_item_by_id(variant);

        m_loading = false;
    }

    nlohmann::json to_json() const override {
        nlohmann::json j;
        if (auto selected = m_color_scheme_combo->selected_item()) {
            j = color_scheme_preferences::variant_to_json(
                selected->id, horizon::ThemeManager::instance().app_color_scheme_variants("pluma-writer"));
        } else {
            j = color_scheme_preferences::variant_to_json(
                color_scheme_preferences::kDefaultVariant,
                horizon::ThemeManager::instance().app_color_scheme_variants("pluma-writer"));
        }
        return j;
    }

private:
    horizon::Combo *m_color_scheme_combo = nullptr;
    bool m_loading = false;
    std::function<void()> m_on_change;
};

} // namespace pluma_app
