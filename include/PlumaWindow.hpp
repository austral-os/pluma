#pragma once

#include <horizon/ApplicationWindow.hpp>
#include <horizon/TabCollection.hpp>
#include "PlumaView.hpp"
#include "Ribbon/HomeSection.hpp"
#include <memory>
#include <string>
#include <vector>
#include <horizon/dialogs/FileDialog.hpp>

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
    std::unique_ptr<horizon::FileDialog> m_file_dialog;
};

} // namespace pluma_app
