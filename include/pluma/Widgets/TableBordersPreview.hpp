#pragma once

#include <horizon/Widget.hpp>
#include <horizon/EventsManager.hpp>

namespace pluma_app {
namespace Widgets {

class TableBordersPreview : public horizon::Widget {
public:
    TableBordersPreview();
    
    // Toggles a specific border
    void set_preset(int preset_index);
    
    // 0: top, 1: bottom, 2: left, 3: right, 4: inner horiz, 5: inner vert
    bool m_active_borders[6];

    // Signals when user manually changes a border by clicking
    horizon::EventsManager<horizon::EventContext> when_user_changed;

protected:
    void draw(horizon::GraphicsContext& ctx) override;
};

} // namespace Widgets
} // namespace pluma_app
