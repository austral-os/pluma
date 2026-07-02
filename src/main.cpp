#include "PlumaColorSchemePreferences.hpp"
#include "PlumaGeneralSection.hpp"
#include "PlumaWindow.hpp"
#include <cstdlib>
#include <horizon/Application.hpp>
#include <horizon/ConfigManager.hpp>
#include <horizon/I18n.hpp>
#include <horizon/ThemeManager.hpp>
#include <horizon/dialogs/PreferencesContent.hpp>
#include <iostream>

using namespace pluma_app;

int main(int argc, char **argv) {
  horizon::Application app("pluma-writer", 1024, 768);
  horizon::i18n().load_app_locales("pluma-writer");

  if (!horizon::theme_manager()->load_app_color_scheme("pluma-writer")) {
    std::cerr << "Warning: failed to load Pluma app color scheme; using system "
                 "theme fallback\n";
  }
  if (!horizon::theme_manager()->activate_app_color_scheme("pluma-writer")) {
    std::cerr << "Warning: failed to activate Pluma app color scheme; using "
                 "system theme fallback\n";
  }

  char *home = std::getenv("HOME");
  std::string config_path =
      home ? std::string(home) + "/.config/horizon/pluma-writer.json"
           : "pluma-writer.json";

  horizon::ConfigManager config(config_path);
  config.load();
  std::string variant = color_scheme_preferences::variant_from_json(
      config.get_section(color_scheme_preferences::kSectionName),
      horizon::ThemeManager::instance().app_color_scheme_variants(
          "pluma-writer"));
  if (!horizon::ThemeManager::instance().set_app_color_scheme_variant(
          "pluma-writer", variant)) {
    horizon::ThemeManager::instance().set_app_color_scheme_variant(
        "pluma-writer", color_scheme_preferences::kDefaultVariant);
  }

  app.set_name(horizon::i18n().tr("pluma-writer.title"));
  app.set_icon_name("pluma-writer");

  auto &about = app.about_manager();
  about.set_app_title(horizon::i18n().tr("pluma-writer.title"));
  about.set_app_description(horizon::i18n().tr("pluma-writer.description"));
  about.set_app_version(APP_VERSION);
  about.set_app_icon("pluma-writer");

  app.set_preferences_content(
      [config_path]() {
        auto content =
            std::make_unique<horizon::PreferencesContent>(config_path);
        auto *content_ptr = content.get();
        auto on_change = [content_ptr]() { content_ptr->save_config(); };

        content->add_section(
            horizon::i18n().tr("pluma-writer.preferences.general"),
            "preferences-system",
            std::make_unique<PlumaGeneralSection>(on_change),
            color_scheme_preferences::kSectionName);
        return content;
      },
      800, 600);

  std::string file_to_open;
  if (argc > 1) {
    file_to_open = argv[1];
  }
  auto window = std::make_unique<PlumaWindow>(file_to_open);
  app.set_root(std::move(window));

  app.run();
  return 0;
}
