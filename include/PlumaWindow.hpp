#pragma once

#include <horizon/ApplicationWindow.hpp>
#include <horizon/TabCollection.hpp>
#include "PlumaView.hpp"
#include <memory>
#include <string>

namespace pluma_app {

class PlumaWindow : public horizon::ApplicationWindow {
public:
    PlumaWindow();
    virtual ~PlumaWindow() = default;

    uint32_t file_capabilities() const override { return horizon::FileAll; }
    std::string current_file_path() const override;

private:
    void setup_events();
    void create_tab(const std::string& title, const std::string& path = "");
    void new_file();
    PlumaView* get_current_view() const;

    horizon::TabCollection* m_tabs = nullptr;
};

} // namespace pluma_app
