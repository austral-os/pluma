#include "PlumaWindow.hpp"
#include <horizon/Application.hpp>
#include <horizon/I18n.hpp>

using namespace pluma_app;

int main(int argc, char **argv) {
  horizon::Application app("org.horizon.pluma-writer", 1024, 768);
  horizon::i18n().load_app_locales("pluma-writer");
  app.set_name(horizon::i18n().tr("pluma-writer.title"));
  app.set_icon_name("pluma-writer");

  auto &about = app.about_manager();
  about.set_app_title(horizon::i18n().tr("pluma-writer.title"));
  about.set_app_description(
      horizon::i18n().tr("pluma-writer.description"));
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
