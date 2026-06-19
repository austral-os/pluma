#pragma once

#include <horizon/Widget.hpp>
#include <pluma/PlumaEditor.hpp>
#include <horizon/print/Models.h>
#include <memory>

namespace pluma_app {

class PlumaView : public horizon::Widget {
public:
    PlumaView();
    virtual ~PlumaView();

    void draw(horizon::GraphicsContext& ctx) override;
    void calculate_layout() override;

    int preferred_width() const override;
    int preferred_height() const override;
    int preferred_height(int width) const override;

    // We can also implement mouse/keyboard events if needed
    // bool on_mouse_press(double x, double y, horizon::MouseButton button, uint32_t modifiers) override;
    
    bool load_document(const std::string& path);
    bool save_document(const std::string& path);

    // Clipboard Support
    bool supports_clipboard() const override { return true; }
    bool can_perform(horizon::ClipboardAction action) const override;
    void perform(horizon::ClipboardAction action) override;
    void provide_clipboard_data(const std::string &mime, horizon::DataSink &sink) override;
    void on_clipboard_data_received(const std::string &mime, const std::vector<uint8_t> &data) override;
    std::vector<std::string> provided_mime_types() const override { return {"text/plain"}; }
    std::vector<std::string> accepted_mime_types() const override { return {"text/plain"}; }

    // Undo/Redo Support
    bool supports_undo() const override { return true; }

    // Print Support
    bool supports_printing() const override { return true; }
    horizon::print::PrintDocument generate_print_document(const horizon::print::PrintConfig& config) override;

    const std::string& current_path() const { return m_current_path; }
    void set_current_path(const std::string& path) { m_current_path = path; }

    std::shared_ptr<pluma::PlumaEditor> editor() { return m_editor; }

    void set_application_recursive(horizon::WaylandWindow *app) override;

private:
    std::shared_ptr<pluma::PlumaEditor> m_editor;
    std::string m_current_path;
    std::string m_clipboard_buffer;
    size_t m_blink_timer_id{0};
};

} // namespace pluma_app
