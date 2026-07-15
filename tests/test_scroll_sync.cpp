#include "pluma_app/ScrollSync.hpp"

#include <cmath>
#include <cstdio>
#include <horizon/Widget.hpp>
#include <pluma/Typography/DummyTypography.hpp>

class LargeWidget : public horizon::Widget {
public:
    LargeWidget() { set_size(2000, 4000); }
};

class TestScrollArea : public horizon::ScrollArea {
public:
    using horizon::ScrollArea::hit_test;
};

static bool approx_eq(double a, double b, double eps = 0.01) {
    return std::fabs(a - b) < eps;
}

static void check(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

int main() {
    auto shaper = std::make_shared<pluma::DummyTextShaper>();
    auto font_manager = std::make_shared<pluma::DummyFontManager>();
    auto font = font_manager->getFont({"Arial", 12.0f});
    pluma::PlumaEditor editor(shaper, font);

    TestScrollArea scroll_area;
    scroll_area.set_size(800, 600);
    scroll_area.set_content(std::make_unique<LargeWidget>());
    scroll_area.calculate_layout();

    // --- Test 1: zoom 1.0 transfer preserves editor scroll for culling ---
    editor.setScroll(pluma::Twips(0), pluma::Twips(3000));
    pluma_app::transfer_editor_scroll_to_scroll_area(editor, scroll_area, 1.0f);

    check(scroll_area.scroll_y() == 200, "zoom 1.0 transfer sets ScrollArea y to 200");
    // Editor scroll must NOT be reset — draw() syncs viewport for culling.
    check(editor.getScrollY().getValue() == 3000, "zoom 1.0 transfer preserves editor scroll");

    scroll_area.set_scroll_position(0, 0);
    check(scroll_area.scroll_y() == 0, "ScrollArea can return to top");

    // --- Test 2: zoom 2.0 ---
    editor.setScroll(pluma::Twips(0), pluma::Twips(3000));
    pluma_app::transfer_editor_scroll_to_scroll_area(editor, scroll_area, 2.0f);
    check(scroll_area.scroll_y() == 400, "zoom 2.0 transfer sets ScrollArea y to 400");
    check(editor.getScrollY().getValue() == 3000, "zoom 2.0 transfer preserves editor scroll");

    // --- Test 3: regression — deep scroll target must not clamp to 0 ---
    auto big_widget = std::make_unique<LargeWidget>();
    LargeWidget* big_widget_ptr = big_widget.get();
    big_widget->set_size(2000, 20000);
    scroll_area.set_content(std::move(big_widget));
    scroll_area.calculate_layout();

    editor.setScroll(pluma::Twips(0), pluma::Twips(120000));
    pluma_app::transfer_editor_scroll_to_scroll_area(editor, scroll_area, 1.0f);
    check(scroll_area.scroll_y() == 8000, "deep scroll target does not clamp to zero");
    check(editor.getScrollY().getValue() == 120000, "deep scroll preserves editor scroll");
    check(big_widget_ptr->y() == scroll_area.y() - scroll_area.scroll_y(),
          "transfer updates content y immediately for hit-testing before render");
    check(scroll_area.hit_test(scroll_area.x() + 100, scroll_area.y() + 100) == big_widget_ptr,
          "post-navigation hit-test still reaches content before render");

    // ==================================================================
    // toViewportLocalCoords — catches the double-scroll input bug
    // ==================================================================

    // --- Test 4: no scroll, zoom 1.0 — identity ---
    {
        // Widget at x=0,y=0 in parent, no scroll, zoom=1.0
        auto [vx, vy] = pluma_app::toViewportLocalCoords(
            100.0, 200.0,  /*ctx*/ 0.0, 0.0,  /*widget pos*/ 1.0f,  /*zoom*/ 0, 0);
        check(approx_eq(vx, 100.0), "no-scroll x identity");
        check(approx_eq(vy, 200.0), "no-scroll y identity");
    }

    // --- Test 5: no scroll, zoom 2.0 ---
    {
        // Click at screen (200, 400), widget at (0,0), zoom=2.0
        auto [vx, vy] = pluma_app::toViewportLocalCoords(
            200.0, 400.0, 0.0, 0.0, 2.0f, 0, 0);
        check(approx_eq(vx, 100.0), "zoom 2.0 x conversion");
        check(approx_eq(vy, 200.0), "zoom 2.0 y conversion");
    }

    // --- Test 6: widget offset, no scroll, zoom 1.0 ---
    {
        // Widget at x=50,y=100 in parent, click at (150, 300)
        auto [vx, vy] = pluma_app::toViewportLocalCoords(
            150.0, 300.0, 50.0, 100.0, 1.0f, 0, 0);
        check(approx_eq(vx, 100.0), "widget offset x conversion");
        check(approx_eq(vy, 200.0), "widget offset y conversion");
    }

    // --- Test 7: scrolled widget, zoom 1.0 — THE BUG SCENARIO ---
    // When inside a ScrollArea, widget x()=-scroll_x, y()=-scroll_y.
    // ScrollArea scrolled 200px down. Click at (100, 300) in parent space.
    // Widget is at y=-200 in parent space.
    // Raw: (300 - (-200)) / 1 = 500 content pixels.
    // Viewport-local: 500 - 200/1 = 300.  ← This is what the editor needs.
    // OLD BUG: would pass 500, causing editor to add viewport_y_ again.
    {
        auto [vx, vy] = pluma_app::toViewportLocalCoords(
            100.0, 300.0,       /*ctx*/
            0.0,  -200.0,       /*widget pos = -scroll*/
            1.0f,               /*zoom*/
            0, 200);            /*scroll_x=0, scroll_y=200*/
        check(approx_eq(vx, 100.0), "scrolled widget x conversion");
        check(approx_eq(vy, 300.0), "scrolled widget y conversion");
    }

    // --- Test 8: scrolled widget, zoom 2.0 ---
    // Widget at (-100, -400) (scroll_x=100, scroll_y=400), zoom=2.0
    // Click at (300, 600) in parent space.
    // Raw: (600-(-400))/2 = 500 content pixels.
    // Viewport-local: 500 - 400/2 = 300.
    {
        auto [vx, vy] = pluma_app::toViewportLocalCoords(
            300.0, 600.0,
            -100.0, -400.0,
            2.0f,
            100, 400);
        check(approx_eq(vx, 150.0), "scrolled zoom x conversion");
        check(approx_eq(vy, 300.0), "scrolled zoom y conversion");
    }

    // --- Test 9: double-scroll regression must NOT happen ---
    // This verifies the exact scenario from the bug report:
    // PlumaEditor adds viewport_y_ internally, so the input must be
    // viewport-local.  If the scroll is applied twice, the cursor
    // would land ~200px below the click.
    {
        // Simulate: scroll_y=200, click at screen y=300
        auto [vx, vy] = pluma_app::toViewportLocalCoords(
            50.0, 300.0,
            0.0, -200.0,   // widget positioned by scroll
            1.0f,
            0, 200);       // scroll_y = 200
        // Without fix: would be 500 (raw content) → editor adds viewport_y_
        // → cursor at ~700, way off target.
        // With fix: 300 → editor adds viewport_y_ → correct position.
        check(approx_eq(vy, 300.0), "double-scroll regression maps to viewport-local y");
        // Guard: must NOT be 500 (the double-scroll value)
        check(!approx_eq(vy, 500.0), "double-scroll regression avoids content-space y");
    }

    // --- Test 10: editor viewport for link navigation is visible area ---
    // PlumaView's widget remains full document size so ScrollArea can scroll,
    // but PlumaEditor::scrollToBookmark centres against its viewport height.
    // Passing the full content height makes deep links compute target_y <= 0.
    {
        auto [vw, vh] = pluma_app::editorViewportPixelsForScrollArea(
            2000, 20000, 800, 600);
        check(vw == 784, "editor viewport width subtracts vertical scrollbar");
        check(vh == 584, "editor viewport height subtracts horizontal scrollbar");

        // Document smaller than parent: viewport is still the visible parent area.
        auto [small_vw, small_vh] = pluma_app::editorViewportPixelsForScrollArea(
            400, 300, 800, 600);
        check(small_vw == 800, "small document editor viewport uses parent width");
        check(small_vh == 600, "small document editor viewport uses parent height");
    }

    // --- Test 11: input hit-testing syncs editor scroll before Ctrl+click ---
    // PlumaEditor::getHyperlinkAt(x,y) expects viewport-local coordinates but
    // adds the editor viewport internally. If PlumaView does not copy the
    // ScrollArea scroll into the editor before hit-testing, Ctrl+click after a
    // scroll uses stale viewport_y_ and misses the link entirely.
    {
        pluma::PlumaEditor link_editor(shaper, font);
        link_editor.setViewport(pluma::Twips(5000), pluma::Twips(5000));
        link_editor.loadText("Click here for more");
        link_editor.applyHyperlink(0, 5, "url", "https://example.com");

        auto link_widget = std::make_unique<LargeWidget>();
        link_widget->set_size(2000, 4000);
        scroll_area.set_content(std::move(link_widget));
        scroll_area.calculate_layout();
        scroll_area.set_scroll_position(0, 40);

        check(!link_editor.getHyperlinkAt(100.0, 85.0).has_value(),
              "stale editor scroll misses scrolled link hit-test");

        pluma_app::sync_editor_scroll_from_scroll_area(link_editor, scroll_area, 1.0f);
        auto info = link_editor.getHyperlinkAt(100.0, 85.0);
        check(info.has_value(), "synced editor scroll hits Ctrl+click hyperlink");
        check(info->type == "url", "synced hit-test preserves hyperlink type");
        check(info->target == "https://example.com", "synced hit-test preserves hyperlink target");
    }

    std::printf("All scroll sync + viewport coord tests passed.\n");
    return 0;
}
