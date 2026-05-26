#pragma once

#include <horizon/ApplicationWindow.hpp>
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

    PlumaView* m_pluma_view = nullptr;
    std::string m_current_path;
};

} // namespace pluma_app
