#include <Ribbon/MultiToggleGroupButton.hpp>
#include <horizon/Button.hpp>
#include <horizon/Icon.hpp>
#include <horizon/SolidObject.hpp>
#include <iostream>

namespace pluma_app {

MultiToggleGroupButton::MultiToggleGroupButton() : horizon::GroupButton() {}

MultiToggleGroupButton::~MultiToggleGroupButton() {}

void MultiToggleGroupButton::set_item_active(int index, bool active) {
  if (index >= 0 && index < (int)m_active_states.size()) {
    std::cout << "[MultiToggleGroupButton] set_item_active index=" << index
              << " active=" << active << std::endl;
    m_active_states[index] = active;
    configure();
    invalidate();
  }
}

bool MultiToggleGroupButton::is_item_active(int index) const {
  if (index >= 0 && index < (int)m_active_states.size()) {
    return m_active_states[index];
  }
  return false;
}

void MultiToggleGroupButton::configure() {
  horizon::GroupButton::configure();

  int index = 0;
  for (const auto &item : children()) {
    if (auto button =
            dynamic_cast<horizon::Button<horizon::SolidObject> *>(item.get())) {
      if (index < (int)m_active_states.size() && m_active_states[index]) {
        button->set_draw_state(horizon::WidgetDrawState::PRESSED);
      } else {
        button->set_draw_state(horizon::WidgetDrawState::NORMAL);
      }
      index++;
    }
  }
}

void MultiToggleGroupButton::add_item(std::string text, int width) {
  int index = children().size();
  horizon::GroupButton::add_item(text, width);
  m_active_states.push_back(false);

  auto &btn_ptr = children().back();
  if (auto button = dynamic_cast<horizon::Button<horizon::SolidObject> *>(
          btn_ptr.get())) {
    auto override_state = [this, button, index](horizon::EventContext &ev) {
      std::cout << "[MultiToggleGroupButton] override_state index=" << index
                << " active=" << is_item_active(index) << std::endl;
      if (is_item_active(index)) {
        button->set_draw_state(horizon::WidgetDrawState::PRESSED);
        button->invalidate();
      }
    };

    button->when_mouse_enter.connect(override_state);
    button->when_mouse_leave.connect(override_state);
    button->when_mouse_release.connect(
        [this, override_state](horizon::MouseButtonEventContext &ev) {
          if (this->application()) {
            horizon::EventContext ev_copy = ev;
            this->application()->post_task([override_state, ev_copy]() mutable {
              override_state(ev_copy);
            });
          } else {
            override_state(ev);
          }
        });
  }
}

void MultiToggleGroupButton::add_item(std::unique_ptr<horizon::Icon> icon,
                                      int width) {
  int index = children().size();
  horizon::GroupButton::add_item(std::move(icon), width);
  m_active_states.push_back(false);

  auto &btn_ptr = children().back();
  if (auto button = dynamic_cast<horizon::Button<horizon::SolidObject> *>(
          btn_ptr.get())) {
    auto override_state = [this, button, index](horizon::EventContext &ev) {
      std::cout << "[MultiToggleGroupButton] override_state index=" << index
                << " active=" << is_item_active(index) << std::endl;
      if (is_item_active(index)) {
        button->set_draw_state(horizon::WidgetDrawState::PRESSED);
        button->invalidate();
      }
    };

    button->when_mouse_enter.connect(override_state);
    button->when_mouse_leave.connect(override_state);
    button->when_mouse_release.connect(
        [this, override_state](horizon::MouseButtonEventContext &ev) {
          if (this->application()) {
            horizon::EventContext ev_copy = ev;
            this->application()->post_task([override_state, ev_copy]() mutable {
              override_state(ev_copy);
            });
          } else {
            override_state(ev);
          }
        });
  }
}

void MultiToggleGroupButton::add_item(std::string text, std::string icon,
                                      int width, int button_width) {
  int index = children().size();

  auto icon_obj = std::make_unique<horizon::Icon>();
  icon_obj->set_icon_name(icon);
  icon_obj->set_icon_size(width);
  icon_obj->set_use_theme_colors(true);

  horizon::GroupButton::add_item(std::move(icon_obj), button_width);
  m_active_states.push_back(false);

  auto &btn_ptr = children().back();
  if (auto button = dynamic_cast<horizon::Button<horizon::SolidObject> *>(
          btn_ptr.get())) {
    auto override_state = [this, button, index](horizon::EventContext &ev) {
      std::cout << "[MultiToggleGroupButton] override_state index=" << index
                << " active=" << is_item_active(index) << std::endl;
      if (is_item_active(index)) {
        button->set_draw_state(horizon::WidgetDrawState::PRESSED);
        button->invalidate();
      }
    };

    button->when_mouse_enter.connect(override_state);
    button->when_mouse_leave.connect(override_state);
    button->when_mouse_release.connect(
        [this, override_state](horizon::MouseButtonEventContext &ev) {
          if (this->application()) {
            horizon::EventContext ev_copy = ev;
            this->application()->post_task([override_state, ev_copy]() mutable {
              override_state(ev_copy);
            });
          } else {
            override_state(ev);
          }
        });
  }
}

} // namespace pluma_app
