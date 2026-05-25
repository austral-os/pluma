#pragma once

#include <horizon/ApplicationWindow.hpp>
#include "PlumaView.hpp"
#include <memory>

namespace pluma_app {

class PlumaWindow : public horizon::ApplicationWindow {
public:
    PlumaWindow();
    virtual ~PlumaWindow() = default;

private:
    PlumaView* m_pluma_view = nullptr;
};

} // namespace pluma_app
