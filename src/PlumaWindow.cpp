#include "PlumaWindow.hpp"
#include <horizon/ScrollArea.hpp>
#include <horizon/Toolbar.hpp>
#include <horizon/Logger.hpp>
#include "MainToolbar.hpp"

namespace pluma_app {

PlumaWindow::PlumaWindow() : horizon::ApplicationWindow("Pluma") {
    set_title("Pluma Rich Text Editor");
    set_size(1024, 768);

    auto main_toolbar = std::make_unique<MainToolbar>();
    toolbar()->add_toolbar_widget(std::move(main_toolbar));

    auto scroll_area = std::make_unique<horizon::ScrollArea>();
    auto pluma_view = std::make_unique<PlumaView>();
    m_pluma_view = pluma_view.get();
    
    scroll_area->set_content(std::move(pluma_view));
    set_content(std::move(scroll_area));

    setup_events();
}

std::string PlumaWindow::current_file_path() const {
    return m_current_path;
}

void PlumaWindow::setup_events() {
    when_file_opened.connect([this](Window::FileOpenedContext& ctx) {
        if (m_pluma_view->load_document(ctx.path)) {
            m_current_path = ctx.path;
            LOG_INFO << "Pluma: Opened file: " << ctx.path;
        } else {
            LOG_ERROR << "Pluma: Failed to open file: " << ctx.path;
        }
    });

    when_save.connect([this](Window::FileSaveContext& ctx) {
        if (m_pluma_view->save_document(ctx.path)) {
            LOG_INFO << "Pluma: Saved file to: " << ctx.path;
        } else {
            LOG_ERROR << "Pluma: Failed to save file to: " << ctx.path;
        }
    });

    when_save_as.connect([this](Window::FileSaveContext& ctx) {
        if (m_pluma_view->save_document(ctx.path)) {
            m_current_path = ctx.path;
            LOG_INFO << "Pluma: Saved file as: " << ctx.path;
        } else {
            LOG_ERROR << "Pluma: Failed to save file as: " << ctx.path;
        }
    });
}

} // namespace pluma_app
