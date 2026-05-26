#include "PlumaWindow.hpp"
#include <horizon/ScrollArea.hpp>
#include <horizon/Toolbar.hpp>
#include <horizon/Logger.hpp>
#include "MainToolbar.hpp"
#include <filesystem>

namespace pluma_app {

PlumaWindow::PlumaWindow() : horizon::ApplicationWindow("Pluma") {
    set_title("Pluma Rich Text Editor");
    set_size(1024, 768);

    auto main_toolbar = std::make_unique<MainToolbar>();
    auto* tb_ptr = main_toolbar.get();
    toolbar()->add_toolbar_widget(std::move(main_toolbar));

    tb_ptr->when_new_clicked.connect([this](horizon::EventContext&) {
        this->new_file();
    });

    auto tabs = std::make_unique<horizon::TabCollection>();
    m_tabs = tabs.get();
    m_tabs->set_smart_header(true);
    m_tabs->set_closable_tabs(true);

    m_tabs->when_tab_close_requested.connect([this](int index) {
        m_tabs->remove_tab(index);
        if (m_tabs->tab_count() == 0) {
            new_file();
        }
    });

    m_tabs->when_add_tab_clicked.connect([this](horizon::EventContext&) {
        new_file();
    });

    set_content(std::move(tabs));

    setup_events();

    // Create an initial empty tab
    new_file();
}

void PlumaWindow::new_file() {
    create_tab("Sin título");
}

void PlumaWindow::create_tab(const std::string& title, const std::string& path) {
    auto scroll_area = std::make_unique<horizon::ScrollArea>();
    auto pluma_view = std::make_unique<PlumaView>();
    
    if (!path.empty()) {
        pluma_view->load_document(path);
        pluma_view->set_current_path(path);
    }
    
    scroll_area->set_content(std::move(pluma_view));
    m_tabs->add_tab(title, std::move(scroll_area));
    m_tabs->set_current_tab(m_tabs->tab_count() - 1);
}

PlumaView* PlumaWindow::get_current_view() const {
    if (!m_tabs) return nullptr;
    auto* scroll = dynamic_cast<horizon::ScrollArea*>(m_tabs->current_tab_body());
    if (scroll && !scroll->children().empty()) {
        return dynamic_cast<PlumaView*>(scroll->children()[0].get());
    }
    return nullptr;
}

std::string PlumaWindow::current_file_path() const {
    auto* view = get_current_view();
    return view ? view->current_path() : "";
}

void PlumaWindow::setup_events() {
    when_file_opened.connect([this](Window::FileOpenedContext& ctx) {
        std::filesystem::path p(ctx.path);
        create_tab(p.filename().string(), ctx.path);
        LOG_INFO << "Pluma: Opened file in new tab: " << ctx.path;
    });

    when_save.connect([this](Window::FileSaveContext& ctx) {
        auto* view = get_current_view();
        if (view && view->save_document(ctx.path)) {
            view->set_current_path(ctx.path);
            std::filesystem::path p(ctx.path);
            m_tabs->set_tab_title(m_tabs->current_tab_index(), p.filename().string());
            LOG_INFO << "Pluma: Saved file to: " << ctx.path;
        } else {
            LOG_ERROR << "Pluma: Failed to save file to: " << ctx.path;
        }
    });

    when_save_as.connect([this](Window::FileSaveContext& ctx) {
        auto* view = get_current_view();
        if (view && view->save_document(ctx.path)) {
            view->set_current_path(ctx.path);
            std::filesystem::path p(ctx.path);
            m_tabs->set_tab_title(m_tabs->current_tab_index(), p.filename().string());
            LOG_INFO << "Pluma: Saved file as: " << ctx.path;
        } else {
            LOG_ERROR << "Pluma: Failed to save file as: " << ctx.path;
        }
    });
}

} // namespace pluma_app
