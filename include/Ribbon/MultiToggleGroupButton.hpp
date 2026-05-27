#pragma once

#include <horizon/GroupButton.hpp>
#include <vector>

namespace pluma_app {

class MultiToggleGroupButton : public horizon::GroupButton {
public:
    MultiToggleGroupButton();
    ~MultiToggleGroupButton() override;

    void set_item_active(int index, bool active);
    bool is_item_active(int index) const;

    void add_item(std::string text, int width = -1) override;
    void add_item(std::unique_ptr<horizon::Icon> icon, int width = -1) override;

protected:
    void configure() override;

private:
    std::vector<bool> m_active_states;
};

} // namespace pluma_app
