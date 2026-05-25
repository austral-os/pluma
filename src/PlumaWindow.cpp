#include "PlumaWindow.hpp"
#include <horizon/ScrollArea.hpp>

namespace pluma_app {

PlumaWindow::PlumaWindow() : horizon::ApplicationWindow("Pluma") {
    set_title("Pluma Rich Text Editor");
    set_size(1024, 768);

    auto scroll_area = std::make_unique<horizon::ScrollArea>();
    auto pluma_view = std::make_unique<PlumaView>();
    m_pluma_view = pluma_view.get();
    
    scroll_area->set_content(std::move(pluma_view));
    set_content(std::move(scroll_area));
}

} // namespace pluma_app
