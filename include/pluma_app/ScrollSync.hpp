#pragma once

#include <horizon/ScrollArea.hpp>
#include <pluma/PlumaEditor.hpp>
#include <utility>

namespace pluma_app {

inline void transfer_editor_scroll_to_scroll_area(pluma::PlumaEditor& editor,
                                                   horizon::ScrollArea& scroll_area,
                                                   float zoom) {
    scroll_area.set_scroll_position(
        static_cast<int>((editor.getScrollX().getValue() / 15.0f) * zoom),
        static_cast<int>((editor.getScrollY().getValue() / 15.0f) * zoom));

    // Horizon ScrollArea normally applies the child offset during render().
    // Link navigation updates the scroll position from an input handler, and a
    // second Ctrl+click can arrive before the next render pass. Keep the child
    // position in sync immediately so hit_test() routes that click to PlumaView
    // instead of the ScrollArea/background.
    const auto& children = scroll_area.children();
    if (!children.empty() && children[0]) {
        children[0]->set_position(scroll_area.x() - scroll_area.scroll_x(),
                                  scroll_area.y() - scroll_area.scroll_y());
    }
}

inline void sync_editor_scroll_from_scroll_area(pluma::PlumaEditor& editor,
                                                const horizon::ScrollArea& scroll_area,
                                                float zoom) {
    editor.setScroll(
        pluma::Twips(static_cast<int>((scroll_area.scroll_x() / zoom) * 15.0f)),
        pluma::Twips(static_cast<int>((scroll_area.scroll_y() / zoom) * 15.0f)));
}

/// Convert parent-relative (screen) coordinates to editor viewport-local
/// coordinates.  When the widget is inside a ScrollArea the view is
/// positioned at (-scroll_x, -scroll_y) so raw (ctx - pos) / zoom yields
/// *content* coordinates.  PlumaEditor expects *viewport-local*
/// coordinates (0,0 = top-left of visible area) and adds viewport_x/y_
/// internally.  Without this subtraction the scroll offset is applied
/// twice — once in the content coordinate and once by the editor.
///
/// @param ctx_x  Parent-relative x in screen pixels.
/// @param ctx_y  Parent-relative y in screen pixels.
/// @param widget_x  Widget x() position in parent space (may be negative when scrolled).
/// @param widget_y  Widget y() position in parent space.
/// @param zoom  Current zoom factor.
/// @param scroll_x  ScrollArea horizontal scroll in screen pixels (0 if no ScrollArea).
/// @param scroll_y  ScrollArea vertical scroll in screen pixels (0 if no ScrollArea).
/// @return Viewport-local coordinates in content pixels.
inline std::pair<double, double> toViewportLocalCoords(
        double ctx_x, double ctx_y,
        double widget_x, double widget_y,
        float zoom,
        int scroll_x, int scroll_y) {
    double content_x = (ctx_x - widget_x) / zoom;
    double content_y = (ctx_y - widget_y) / zoom;
    // When inside a ScrollArea, widget_x = -scroll_x so content_x already
    // includes the scroll offset.  Subtract it to get viewport-local.
    content_x -= scroll_x / zoom;
    content_y -= scroll_y / zoom;
    return {content_x, content_y};
}

/// Compute the viewport size PlumaEditor should use for navigation and
/// hit-testing.  The editor viewport is the visible ScrollArea window, not the
/// full content widget size; otherwise scroll-to-link centres against the whole
/// document height and collapses deep targets back to the top.
inline std::pair<int, int> editorViewportPixelsForScrollArea(
        int content_w, int content_h,
        int parent_w, int parent_h) {
    bool has_v_scroll = content_h > parent_h;
    bool has_h_scroll = content_w > parent_w;

    int viewport_w = parent_w - (has_v_scroll ? 16 : 0);
    int viewport_h = parent_h - (has_h_scroll ? 16 : 0);

    if (viewport_w < 1) viewport_w = 1;
    if (viewport_h < 1) viewport_h = 1;
    return {viewport_w, viewport_h};
}

} // namespace pluma_app
