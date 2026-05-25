#!/bin/bash
export XDG_RUNTIME_DIR=/tmp/wayland-$(id -u)
export WAYLAND_DISPLAY=wayland-1
export XDG_CURRENT_DESKTOP=horizon:hzn-wayfire
cd /home/horacio/HdrDevs/pluma/build
./pluma-app &
PID=$!
sleep 2
# Now, let's take a screenshot before scrolling
grim -t png before_scroll.png
# Let's try to simulate a scroll!
# Since we don't have wtype easily doing scroll, we can just change PlumaView's m_y directly? No, we can't.
# We can't easily drag the scrollbar from bash without a tool.
kill $PID
