#pragma once

#include <horizon/ApplicationWindow.hpp>
#include <horizon/TabCollection.hpp>
#include "PlumaView.hpp"
#include "Ribbon/HomeSection.hpp"
#include "Ribbon/PageLayoutSection.hpp"
#include "Ribbon/ImageFormatSection.hpp"
#include "Ribbon/TableLayoutSection.hpp"
#include "Ribbon/InsertSection.hpp"
#include <memory>
#include <string>
#include <vector>
#include <horizon/dialogs/FileDialog.hpp>
#include <horizon/Label.hpp>
#include <horizon/Slider.hpp>

namespace pluma_app {

class PlumaWindow : public horizon::ApplicationWindow {
public:
    PlumaWindow(const std::string& initial_file = "");
    virtual ~PlumaWindow() = default;

    uint32_t file_capabilities() const override { return horizon::FileAll; }
    std::string current_file_path() const override;

private:
    void setup_events();
    void create_tab(const std::string& title, const std::string& path = "");
    void new_file();
    PlumaView* get_current_view() const;
    void update_status_bar();
    void update_ribbon_state(PlumaView* view, HomeSection* home_sec);

    horizon::TabCollection* m_tabs = nullptr;
    std::vector<std::unique_ptr<HomeSection>> m_home_sections;
    std::vector<std::unique_ptr<InsertSection>> m_insert_sections;
    std::vector<std::unique_ptr<PageLayoutSection>> m_page_layout_sections;
    std::vector<std::unique_ptr<ImageFormatSection>> m_image_format_sections;
    std::vector<std::unique_ptr<TableLayoutSection>> m_table_layout_sections;
    std::unique_ptr<horizon::FileDialog> m_file_dialog;
    horizon::Label* m_lang_label = nullptr;
    horizon::Label* m_status_label = nullptr;
    horizon::Slider* m_zoom_slider = nullptr;
    horizon::Label* m_zoom_label = nullptr;
};

} // namespace pluma_app
