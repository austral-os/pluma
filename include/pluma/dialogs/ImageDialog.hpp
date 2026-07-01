#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/EventsManager.hpp>
#include <horizon/Notebook.hpp>
#include <horizon/TextBox.hpp>
#include <horizon/TextBoxPolicies.hpp>
#include <horizon/Checkbox.hpp>
#include <string>

namespace pluma_app {
namespace dialogs {

struct ImageSizeEvent : public horizon::EventContext {
    void* sender = nullptr;
    float width_pt;   // in points
    float height_pt;  // in points
};

class ImageDialog : public horizon::WaylandWindow {
public:
    ImageDialog();

    /// Sets the initial size in points (the dialog converts to cm for display)
    void set_initial_size(float width_pt, float height_pt);

    horizon::EventsManager<ImageSizeEvent> when_accepted;

private:
    horizon::Notebook* m_notebook;
    horizon::TextBox<horizon::DoublePolicy>* m_width_box;
    horizon::TextBox<horizon::DoublePolicy>* m_height_box;
    horizon::Checkbox<horizon::AquaObject>* m_aspect_ratio_check;

    float m_orig_width_pt{0};   // original width in points
    float m_orig_height_pt{0};  // original height in points
    float m_aspect_ratio{1};    // width / height

    bool m_updating{false};  // guard against recursive text_changed

    void on_close();
    void on_width_changed();
    void on_height_changed();
    void apply_aspect_ratio(bool width_changed);
};

} // namespace dialogs
} // namespace pluma_app
