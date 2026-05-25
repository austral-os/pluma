#include <horizon/Application.hpp>
#include <horizon/ApplicationWindow.hpp>
#include <horizon/ScrollArea.hpp>
#include <horizon/Widget.hpp>
#include <iostream>

class MyWidget : public horizon::Widget {
public:
    MyWidget() { set_size(1000, 2000); }
    void draw(horizon::GraphicsContext& ctx) override {
        std::cout << "Draw called!" << std::endl;
    }
};

int main(int argc, char** argv) {
    horizon::Application app(argc, argv, "test");
    auto win = std::make_unique<horizon::ApplicationWindow>("test");
    auto sa = std::make_unique<horizon::ScrollArea>();
    sa->set_content(std::make_unique<MyWidget>());
    win->set_content(std::move(sa));
    win->show();
    return app.run();
}
