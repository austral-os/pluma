#include "PlumaWindow.hpp"
#include <horizon/Application.hpp>

using namespace pluma_app;

int main(int argc, char **argv) {
  horizon::Application app("org.horizon.pluma", 1024, 768);
  app.set_name("Pluma");
  app.set_icon_name("pluma-writer");

  auto &about = app.about_manager();
  about.set_app_title("Pluma Writer");
  about.set_app_description(
      "A rich text editor for the Horizon desktop environment.");
  about.set_app_version(APP_VERSION);
  about.set_app_icon("pluma-writer");

  std::string file_to_open;
  if (argc > 1) {
    file_to_open = argv[1];
  }
  auto window = std::make_unique<PlumaWindow>(file_to_open);
  app.set_root(std::move(window));

  app.run();
  return 0;
}
