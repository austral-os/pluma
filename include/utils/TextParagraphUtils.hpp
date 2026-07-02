#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pluma_app {
namespace utils {

/**
 * @brief Given document text and a selection range [sel_start, sel_end),
 *        returns the starting offsets of all paragraphs that the selection
 *        covers, partially or fully.
 *
 * A paragraph is defined as text delimited by newline characters ('\n').
 * The first paragraph starts at offset 0; each newline starts a new
 * paragraph at the next character.
 *
 * Critical regression rule: if the selection ends exactly at a newline
 * boundary (sel_end points to the character immediately after a '\n'),
 * the following paragraph is NOT included.  This prevents formatting the
 * paragraph below the cursor when the user selects text that ends right
 * at the newline.
 *
 * Edge cases:
 *   - Empty text returns an empty vector.
 *   - sel_start == sel_end (collapsed cursor) still expands to the
 *     enclosing paragraph and returns its start offset.
 *   - sel_start or sel_end beyond text.length() is clamped.
 *
 * @param text      The full document text.
 * @param sel_start Start offset of the selection (inclusive).
 * @param sel_end   End offset of the selection (exclusive — past-the-end).
 * @return A vector of paragraph start offsets in ascending order.
 */
inline std::vector<uint32_t> getSelectedParagraphStarts(
    const std::string& text,
    uint32_t sel_start,
    uint32_t sel_end)
{
    if (text.empty()) {
        return {};
    }

    // Clamp to document bounds.
    if (sel_start > text.length()) sel_start = static_cast<uint32_t>(text.length());
    if (sel_end   > text.length()) sel_end   = static_cast<uint32_t>(text.length());

    // Expand sel_start backward to the start of its enclosing paragraph.
    uint32_t first_para_start = sel_start;
    while (first_para_start > 0 && text[first_para_start - 1] != '\n') {
        --first_para_start;
    }

    // Expand sel_end forward to the end of its enclosing paragraph (the '\n'
    // or end-of-text).  This bounds our search window below.
    uint32_t last_para_end = sel_end;
    while (last_para_end < text.length() && text[last_para_end] != '\n') {
        ++last_para_end;
    }

    std::vector<uint32_t> paragraph_starts;
    paragraph_starts.push_back(first_para_start);

    // Iterate through the search window.  Every '\n' whose *next character*
    // falls strictly within sel_end begins a paragraph that the selection
    // actually covers.  If pos + 1 == sel_end the selection ends right at
    // the newline and the following paragraph is excluded.
    for (uint32_t pos = first_para_start;
         pos < last_para_end && pos < text.length();
         ++pos)
    {
        if (text[pos] == '\n' && pos + 1 < sel_end) {
            paragraph_starts.push_back(pos + 1);
        }
    }

    return paragraph_starts;
}

} // namespace utils
} // namespace pluma_app
