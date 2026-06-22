#include "PlumaView.hpp"
#include <horizon/Logger.hpp>
#include <horizon/GraphicsContext.hpp>
#include <pluma/Plugins/PlumaArchiveExporter.hpp>
#include <pluma/Plugins/PdfExporter.hpp>
#include <pluma/Plugins/PlumaArchiveImporter.hpp>
#include <pluma/Render/CairoRenderer.hpp>
#include <pluma/Typography/DummyTypography.hpp>

#include <horizon/ThemeManager.hpp>
#include <horizon/WaylandWindow.hpp>
#include <horizon/Menu.hpp>
#include <pluma/Services/SpellCheckerService.hpp>
#include <pluma/Services/SpellCheckAnalyzer.hpp>
#include <string>
#include <fstream>
#include <unistd.h>

namespace pluma_app {

class RealCairoShaper : public pluma::ITextShaper {
public:
  RealCairoShaper() {
    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cr_ = cairo_create(surface);
    cairo_select_font_face(cr_, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr_, 16.0);
    cairo_surface_destroy(surface);
  }

  ~RealCairoShaper() { cairo_destroy(cr_); }

  pluma::ShapedTextRun
  shapeText(std::string_view text,
            const std::shared_ptr<pluma::IFont> &font) override {
    float size_pt = font->getDescriptor().size_pt;
    std::string family = font->getDescriptor().family;
    if (family.empty()) family = "sans-serif";

    cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
    if (static_cast<uint16_t>(font->getDescriptor().weight) >= 700) {
        weight = CAIRO_FONT_WEIGHT_BOLD;
    }
    
    cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
    if (font->getDescriptor().italic) {
        slant = CAIRO_FONT_SLANT_ITALIC;
    }

    cairo_select_font_face(cr_, family.c_str(), slant, weight);
    cairo_set_font_size(cr_, size_pt);

    pluma::ShapedTextRun run;
    run.max_ascent = pluma::Twips(size_pt * 15.0 * 0.8);
    run.max_descent = pluma::Twips(size_pt * 15.0 * 0.2);
    run.total_width = pluma::Twips(0);

    for (size_t i = 0; i < text.length();) {
      unsigned char c = text[i];
      size_t char_len = 1;
      if ((c & 0xE0) == 0xC0)
        char_len = 2;
      else if ((c & 0xF0) == 0xE0)
        char_len = 3;
      else if ((c & 0xF8) == 0xF0)
        char_len = 4;

      if (i + char_len > text.length())
        char_len = text.length() - i;

      std::string char_str(text.substr(i, char_len));
      cairo_text_extents_t extents;
      cairo_text_extents(cr_, char_str.c_str(), &extents);

      pluma::Twips advance(extents.x_advance * 15.0);

      pluma::Glyph g;
      g.x_advance = advance;
      run.glyphs.push_back(g);
      run.total_width = run.total_width + advance;

      for (size_t j = 1; j < char_len; ++j) {
        pluma::Glyph g_empty;
        g_empty.x_advance = pluma::Twips(0);
        run.glyphs.push_back(g_empty);
      }

      i += char_len;
    }

    return run;
  }

private:
  cairo_t *cr_;
};

PlumaView::PlumaView() : horizon::Widget() {
  set_position_type(horizon::FREE);
  auto shaper = std::make_shared<RealCairoShaper>();
  auto font =
      std::make_shared<pluma::DummyFontManager>()->getFont({"Inter", 12.0f});
  m_editor = std::make_shared<pluma::PlumaEditor>(shaper, font);

  m_editor->setPageSize(pluma::PageSizes::A4);
  m_editor->setMargins(
      pluma::PageMargins(pluma::Twips(1134), pluma::Twips(1134),
                         pluma::Twips(1134), pluma::Twips(1134)));
  m_editor->setMarginColor(pluma::Color(0xFF00AA00));
  m_editor->showMargins();

  m_editor->loadText("");

  m_service_manager = std::make_shared<pluma::ServiceManager>();

  auto spell_service = std::make_shared<pluma::SpellCheckerService>();
  
  std::vector<std::string> search_paths = {
      "dictionaries/",
      "build/dictionaries/",
      "assets/dictionaries/",
      "/usr/share/pluma-writer/dictionaries/",
      "/usr/local/share/pluma-writer/dictionaries/"
  };
  
  bool dict_loaded = false;
  for (const auto& dir : search_paths) {
      if (!std::filesystem::exists(dir)) continue;
      for (const auto& entry : std::filesystem::directory_iterator(dir)) {
          if (entry.is_regular_file() && entry.path().extension() == ".aff") {
              std::string path = entry.path().string();
              std::string base = path.substr(0, path.length() - 4);
              if (std::filesystem::exists(base + ".dic")) {
                  std::string lang = entry.path().stem().string();
                  spell_service->registerDictionary(lang, path, base + ".dic");
                  dict_loaded = true;
              }
          }
      }
  }

  if (dict_loaded) {
      m_spell_service = spell_service;

      auto analyzer = std::make_shared<pluma::SpellCheckAnalyzer>(
          spell_service,
          "es_ES",
          [this](const std::vector<std::pair<uint32_t, uint32_t>>& errors) {
              if (application()) {
                  application()->post_task([this, errors]() {
                      if (m_editor) {
                          m_editor->clearDecorationGlobally(pluma::TextDecoration::SpellingError);
                          for (const auto& err : errors) {
                              m_editor->applyStyle(err.first, err.second, pluma::PropertyId::Decoration, pluma::TextDecoration::SpellingError);
                          }
                          invalidate();
                      }
                  });
              }
          }
      );
      m_service_manager->registerService(analyzer);
  }

  set_background_color(horizon::Color(0.8f, 0.8f, 0.8f, 1.0f));
  set_focusable(true);

  when_application_load.connect([this](horizon::EventContext &) {
    set_focus(true);
  });

  when_mouse_press.connect([this](horizon::MouseButtonEventContext &ctx) {
    if (application()) {
      application()->set_focused_widget(this);
    }

    double local_x = (ctx.x - x()) / m_zoom;
    double local_y = (ctx.y - y()) / m_zoom;

    // Let the parent ScrollArea handle scrollbar clicks
    if (local_x > width() - 20 || local_y > height() - 20) {
      return;
    }

    if (m_editor) {
      pluma::MouseButton pbtn = pluma::MouseButton::None;
      if (ctx.button == 272 || ctx.button == 1)
        pbtn = pluma::MouseButton::Left;
      else if (ctx.button == 274 || ctx.button == 3)
        pbtn = pluma::MouseButton::Middle;

      if (pbtn != pluma::MouseButton::None) {
          m_editor->onMouseDown(local_x, local_y, pbtn,
                                static_cast<pluma::ModifierFlags>(ctx.modifiers));
          invalidate();
      }
    }
  });

  when_right_click.connect([this](horizon::MouseButtonEventContext &ctx) {
    if (application()) {
      application()->set_focused_widget(this);
    }

    double local_x = (ctx.x - x()) / m_zoom;
    double local_y = (ctx.y - y()) / m_zoom;

    if (local_x > width() - 20 || local_y > height() - 20) {
      return;
    }

    if (m_editor) {
      if (m_editor->getSelectionRange().isCollapsed()) {
        m_editor->onMouseDown(local_x, local_y, pluma::MouseButton::Right,
                              static_cast<pluma::ModifierFlags>(ctx.modifiers));
        m_editor->onMouseUp(local_x, local_y, pluma::MouseButton::Right,
                            static_cast<pluma::ModifierFlags>(ctx.modifiers));
        invalidate();
      }

      if (!ctx.stop_propagation) {
          m_active_context_menu = buildContextMenu(local_x, local_y);
          if (m_active_context_menu) {
              if (application()) {
                  application()->show_context_menu(m_active_context_menu.get(), -1, -1, ctx.serial, this);
              }
              ctx.stop_propagation = true;
          }
      }
    }
  });

  when_mouse_release.connect([this](horizon::MouseButtonEventContext &ctx) {
    if (m_editor) {
      double local_x = (ctx.x - x()) / m_zoom;
      double local_y = (ctx.y - y()) / m_zoom;
      pluma::MouseButton pbtn = pluma::MouseButton::None;
      if (ctx.button == 272 || ctx.button == 1)
        pbtn = pluma::MouseButton::Left;
      else if (ctx.button == 273 || ctx.button == 2)
        pbtn = pluma::MouseButton::Right;
      else if (ctx.button == 274 || ctx.button == 3)
        pbtn = pluma::MouseButton::Middle;

      m_editor->onMouseUp(local_x, local_y, pbtn,
                          static_cast<pluma::ModifierFlags>(ctx.modifiers));
      invalidate();
    }
  });

  when_mouse_drag.connect([this](horizon::MouseMoveEventContext &ctx) {
    if (m_editor) {
      double local_x = (ctx.x - x()) / m_zoom;
      double local_y = (ctx.y - y()) / m_zoom;
      m_editor->onMouseMove(local_x, local_y,
                            static_cast<pluma::ModifierFlags>(ctx.modifiers));
      invalidate();
    }
  });

  when_mouse_wheel.connect([this](horizon::MouseWheelEventContext &ctx) {
    if (parent()) {
      parent()->when_mouse_wheel.run(ctx);
    }
  });

  when_key_press.connect([this](horizon::KeyEventContext &ctx) {
    if (!m_editor)
      return;

    bool handled = m_editor->onKeyPress(
        ctx.keysym, static_cast<pluma::ModifierFlags>(ctx.modifiers));
    if (!handled) {
      if (ctx.keysym == 0xff0d || ctx.keysym == 0xff8d) { // Return or KP_Enter
        m_editor->onTextInput("\n");
        handled = true;
      } else if (!ctx.text.empty() && !(ctx.modifiers & (horizon::WaylandWindow::Modifier::CTRL | horizon::WaylandWindow::Modifier::ALT))) {
        std::string text = ctx.text;
        if (text == "\r")
          text = "\n";
        m_editor->onTextInput(text);
        handled = true;
      }
    }

    if (handled) {
      ctx.stop_propagation = true;
      calculate_layout();
      invalidate();
      if (parent())
        parent()->invalidate();
        
      if (m_service_manager && m_editor) {
          m_service_manager->runAnalysis(m_editor->getSnapshot(), m_editor->getFormatRegistry());
        }
    }
  });

  when_key_release.connect([this](horizon::KeyEventContext &ctx) {
    if (!m_editor)
      return;
    bool handled = m_editor->onKeyRelease(
        ctx.keysym, static_cast<pluma::ModifierFlags>(ctx.modifiers));
    if (handled) {
      ctx.stop_propagation = true;
      invalidate();
    }
  });

  when_undo.connect([this](horizon::EventContext &ctx) {
    if (m_editor) {
      m_editor->undo();
      calculate_layout();
      invalidate();
      if (parent()) {
        parent()->calculate_layout();
        parent()->invalidate();
      }
    }
  });

  when_redo.connect([this](horizon::EventContext &ctx) {
    if (m_editor) {
      m_editor->redo();
      calculate_layout();
      invalidate();
      if (parent()) {
        parent()->calculate_layout();
        parent()->invalidate();
      }
    }
  });
}

PlumaView::~PlumaView() {
    if (application() && m_blink_timer_id != 0) {
        application()->stop_timer(m_blink_timer_id);
    }
}

void PlumaView::set_application_recursive(horizon::WaylandWindow *app) {
    if (!app && application() && m_blink_timer_id != 0) {
        application()->stop_timer(m_blink_timer_id);
        m_blink_timer_id = 0;
    }
    
    horizon::Widget::set_application_recursive(app);
    
    if (app && m_blink_timer_id == 0) {
        m_blink_timer_id = app->add_timer(500, [this]() {
            if (m_editor && m_editor->onBlinkTimer()) {
                invalidate();
            }
        }, true);
        
        app->when_popup_dismissed.connect([this](horizon::PopupDismissedContext &) {
            m_active_context_menu.reset();
        });
    }
}


void PlumaView::draw(horizon::GraphicsContext &ctx) {
  if (m_is_printing) return;
  if (!m_editor)
    return;

  cairo_t *cr = static_cast<cairo_t *>(ctx.getNativeContext());
  if (!cr)
    return;

  cairo_save(cr);
  cairo_translate(cr, x(), y());
  cairo_scale(cr, m_zoom, m_zoom);

  // Sync theme colors
  auto h_to_p = [](const horizon::Color &c) -> uint32_t {
    uint32_t a = static_cast<uint32_t>(c.a * 255.0f) & 0xFF;
    uint32_t r = static_cast<uint32_t>(c.r * 255.0f) & 0xFF;
    uint32_t g = static_cast<uint32_t>(c.g * 255.0f) & 0xFF;
    uint32_t b = static_cast<uint32_t>(c.b * 255.0f) & 0xFF;
    return (a << 24) | (r << 16) | (g << 8) | b;
  };

  horizon::Color bg = horizon::ThemeManager::instance().get_color("textbox_bg");
  horizon::Color text =
      horizon::ThemeManager::instance().get_color("textbox_fg");
  std::string variant = horizon::ThemeManager::instance().get_variant();
  horizon::Color margin = (variant == "dark") ? bg.lighter(20.0f) : bg.darker(20.0f);
  horizon::Color workspace =
      horizon::ThemeManager::instance().get_color("window_bg");
  workspace = workspace.darker(50.0f);

  m_editor->setPageBackgroundColor(h_to_p(bg));
  m_editor->setDefaultTextColor(h_to_p(text));
  m_editor->setWorkspaceBackgroundColor(h_to_p(workspace));
  m_editor->setMarginColor(h_to_p(margin));

  // Clear the visible area to prevent smearing when scrolling
  double x1, y1, x2, y2;
  cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

  cairo_set_source_rgba(cr, workspace.r, workspace.g, workspace.b, workspace.a);
  cairo_rectangle(cr, x1, y1, x2 - x1, y2 - y1);
  cairo_fill(cr);

  pluma::CairoRenderer renderer(cr);
  m_editor->render(renderer);

  cairo_restore(cr);
}

void PlumaView::calculate_layout() {
  if (m_is_printing) return;
  horizon::Widget::calculate_layout();

  int target_w = preferred_width();
  int target_h = preferred_height();

  if (parent()) {
    int p_w = parent()->width();
    int p_h = parent()->height();

    bool has_v_scroll = target_h > p_h;
    bool has_h_scroll = target_w > p_w;

    int viewport_w = p_w - (has_v_scroll ? 16 : 0);
    int viewport_h = p_h - (has_h_scroll ? 16 : 0);

    if (viewport_w > target_w) {
      target_w = viewport_w;
    }
    if (viewport_h > target_h) {
      target_h = viewport_h;
    }
  }

  if (width() != target_w || height() != target_h) {
    set_size(target_w, target_h);
  }

  if (m_editor) {
    m_editor->setViewport(pluma::Twips((target_w / m_zoom) * 15),
                          pluma::Twips((target_h / m_zoom) * 15));
  }
}


int PlumaView::preferred_width() const {
  return 800 * m_zoom; // default A4 width in pixels
}

int PlumaView::preferred_height() const {
  return 1120 * m_zoom; // default A4 height in pixels
}

int PlumaView::preferred_height(int /*width*/) const { return 1120 * m_zoom; }

bool PlumaView::load_document(const std::string &path) {
  if (!m_editor)
    return false;
  pluma::plugins::PlumaArchiveImporter importer;
  bool success = importer.importFile(path, *m_editor);
  if (success) {
    calculate_layout();
    invalidate();
  }
  return success;
}

bool PlumaView::save_document(const std::string &path) {
  if (!m_editor)
    return false;
    
  if (path.length() >= 4 && path.substr(path.length() - 4) == ".pdf") {
    pluma::plugins::PdfExporter exporter;
    return exporter.exportToFile(path, *m_editor);
  }
  
  pluma::plugins::PlumaArchiveExporter exporter;
  return exporter.exportToFile(path, *m_editor);
}

bool PlumaView::can_perform(horizon::ClipboardAction action) const {
    if (!m_editor) return false;
    switch (action) {
        case horizon::ClipboardAction::Copy:
        case horizon::ClipboardAction::Cut:
            return !m_editor->getSelectionRange().isCollapsed();
        case horizon::ClipboardAction::Paste:
            return true;
    }
    return false;
}

void PlumaView::perform(horizon::ClipboardAction action) {
    if (!m_editor) return;
    
    switch (action) {
        case horizon::ClipboardAction::Copy:
            m_clipboard_buffer = m_editor->getSelectedText();
            if (application()) application()->set_clipboard_owner(this);
            break;
        case horizon::ClipboardAction::Cut:
            m_clipboard_buffer = m_editor->getSelectedText();
            if (application()) application()->set_clipboard_owner(this);
            m_editor->deleteSelection();
            calculate_layout();
            invalidate();
            if (parent()) {
                parent()->calculate_layout();
                parent()->invalidate();
            }
            triggerAnalysis();
            break;
        case horizon::ClipboardAction::Paste:
            if (application()) application()->request_clipboard_data(this);
            break;
    }
    
    set_focus(true);
}

void PlumaView::provide_clipboard_data(const std::string &mime, horizon::DataSink &sink) {
    if (mime == "text/plain") {
        sink.write(std::vector<uint8_t>(m_clipboard_buffer.begin(), m_clipboard_buffer.end()));
        sink.done();
    } else {
        sink.error();
    }
}

void PlumaView::on_clipboard_data_received(const std::string &mime, const std::vector<uint8_t> &data) {
    if (mime == "text/plain") {
        std::string content(data.begin(), data.end());
        if (m_editor) {
            m_editor->pasteText(content);
            calculate_layout();
            invalidate();
            if (parent()) {
                parent()->calculate_layout();
                parent()->invalidate();
            }
            triggerAnalysis();
        }
    }
    
    set_focus(true);
}

horizon::print::PrintDocument PlumaView::generate_print_document(const horizon::print::PrintConfig& config) {
    horizon::print::PrintDocument doc;
    doc.mime_type = "application/pdf";
    doc.title = "Pluma Document";

    if (!m_editor) return doc;

    char temp_template[] = "/tmp/pluma_print_XXXXXX.pdf";
    int fd = mkstemps(temp_template, 4);
    if (fd == -1) return doc;
    close(fd);

    std::string temp_path(temp_template);

    m_is_printing = true;
    pluma::plugins::PdfExporter exporter;
    bool success = exporter.exportToFile(temp_path, *m_editor);
    m_is_printing = false;

    if (success) {
        std::ifstream file(temp_path, std::ios::binary);
        if (file) {
            file.seekg(0, std::ios::end);
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            if (size > 0) {
                doc.data.resize(size);
                if (!file.read(reinterpret_cast<char*>(doc.data.data()), size)) {
                    doc.data.clear();
                }
            }
        }
    }

    unlink(temp_path.c_str());
    return doc;
}



void PlumaView::triggerAnalysis() {
  if (m_service_manager && m_editor) {
    m_service_manager->runAnalysis(m_editor->getSnapshot(), m_editor->getFormatRegistry());
  }
}

std::unique_ptr<horizon::Menu> PlumaView::buildContextMenu(double local_x, double local_y) {
    auto menu = std::make_unique<horizon::Menu>();
    if (!m_editor) return menu;

    uint32_t head = m_editor->getSelectionRange().head;
    auto style = m_editor->getFormatRegistry().getStyleAt(head);
    
    bool is_spell_error = false;
    if (auto dec = style.get(pluma::PropertyId::Decoration)) {
        if (std::get<pluma::TextDecoration>(*dec) == pluma::TextDecoration::SpellingError) {
            is_spell_error = true;
        }
    }

    if (is_spell_error && m_spell_service) {
        std::string text = m_editor->getSnapshot()->getText();
        int start = head;
        while (start > 0 && std::isalpha(static_cast<unsigned char>(text[start - 1]))) start--;
        int end = head;
        while (end < text.length() && std::isalpha(static_cast<unsigned char>(text[end]))) end++;
        
        if (start < end) {
            std::string word = text.substr(start, end - start);
            std::string lang = "es_ES";
            if (auto l = style.get(pluma::PropertyId::Language)) {
                lang = std::get<std::string>(*l);
            }
            
            auto suggestions = m_spell_service->getSuggestions(word, lang);
            for (size_t i = 0; i < std::min<size_t>(5, suggestions.size()); ++i) {
                auto item = std::make_unique<horizon::MenuItem>(suggestions[i]);
                item->when_click.connect([this, start, end, sugg = suggestions[i]](auto&) {
                    m_editor->setSelection(start, end);
                    m_editor->insertTextAtCursor(sugg);
                    m_editor->applyStyle(start, sugg.length(), pluma::PropertyId::Decoration, pluma::TextDecoration::None);
                    triggerAnalysis();
                    invalidate();
                    if (parent()) parent()->invalidate();
                });
                menu->add_item(std::move(item));
            }
            
            if (!suggestions.empty()) {
                menu->add_separator();
            }
        }
    }

    if (m_editor->isInTable()) {
        auto table_menu = std::make_unique<horizon::Menu>();
        
        auto add_row_up = std::make_unique<horizon::MenuItem>("Fila Arriba");
        add_row_up->when_click.connect([this](auto&){ m_editor->insertTableRowAbove(); invalidate(); });
        table_menu->add_item(std::move(add_row_up));
        
        auto add_row_down = std::make_unique<horizon::MenuItem>("Fila Abajo");
        add_row_down->when_click.connect([this](auto&){ m_editor->insertTableRowBelow(); invalidate(); });
        table_menu->add_item(std::move(add_row_down));
        
        auto add_col_left = std::make_unique<horizon::MenuItem>("Columna Antes");
        add_col_left->when_click.connect([this](auto&){ m_editor->insertTableColumnLeft(); invalidate(); });
        table_menu->add_item(std::move(add_col_left));
        
        auto add_col_right = std::make_unique<horizon::MenuItem>("Columna Después");
        add_col_right->when_click.connect([this](auto&){ m_editor->insertTableColumnRight(); invalidate(); });
        table_menu->add_item(std::move(add_col_right));
        
        auto insert_item = std::make_unique<horizon::MenuItem>("Insertar");
        insert_item->set_submenu(std::move(table_menu));
        menu->add_item(std::move(insert_item));
    }

    return menu;
}

} // namespace pluma_app
