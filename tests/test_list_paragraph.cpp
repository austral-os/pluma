/**
 * @file test_list_paragraph.cpp
 * @brief Regression tests for the paragraph boundary detection used by the
 *        list-button handler (group_lists in PlumaWindow.cpp).
 *
 * The critical rule: multi-paragraph selection applies bullets/numbering to
 * ALL selected paragraphs, but a selection ending exactly after a newline
 * must NOT format the following paragraph.
 *
 * These tests exercise the pure function getSelectedParagraphStarts() from
 * TextParagraphUtils.hpp -- no GUI, no editor, no Horizon dependencies.
 *
 * Standalone test executable (no Catch2, no external dependencies).
 */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "utils/TextParagraphUtils.hpp"

using namespace pluma_app::utils;

// ============================================================================
// Minimal test framework (standard library only -- no external dependencies)
// ============================================================================

static int g_test_failures = 0;
static int g_test_count    = 0;
static int g_test_passed   = 0;

// Non-fatal check: records failure, continues test.
#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            ++g_test_failures;                                          \
            std::cerr << "  FAILED: " << #expr << "\n"                  \
                      << "    at " << __FILE__ << ":" << __LINE__       \
                      << "\n";                                          \
        }                                                               \
    } while (0)

// Fatal requirement: records failure and aborts the current test.
#define REQUIRE(expr)                                                   \
    do {                                                                \
        if (!(expr)) {                                                  \
            ++g_test_failures;                                          \
            std::cerr << "  FAILED (REQUIRE): " << #expr << "\n"        \
                      << "    at " << __FILE__ << ":" << __LINE__       \
                      << "\n";                                          \
            return;                                                     \
        }                                                               \
    } while (0)

// Fatal failure with message: records failure and aborts the current test.
#define FAIL(msg)                                                       \
    do {                                                                \
        ++g_test_failures;                                              \
        std::cerr << "  FAILED: " << msg << "\n"                        \
                  << "    at " << __FILE__ << ":" << __LINE__           \
                  << "\n";                                              \
        return;                                                         \
    } while (0)

// Test registration helper.
struct TestCase {
    const char* name;
    void (*func)();
};

#define TEST(name) void name()

#define RUN_TEST(tc)                                                    \
    do {                                                                \
        ++g_test_count;                                                 \
        int before = g_test_failures;                                   \
        tc.func();                                                      \
        if (g_test_failures == before) {                                \
            ++g_test_passed;                                            \
            std::cout << "  OK  " << tc.name << "\n";                   \
        } else {                                                        \
            std::cout << "  FAIL " << tc.name << "\n";                  \
        }                                                               \
    } while (0)

// ============================================================================
// Tests
// ============================================================================

// ---------------------------------------------------------------------------
// Basic / sanity
// ---------------------------------------------------------------------------

TEST(empty_text_returns_empty_vector) {
    auto result = getSelectedParagraphStarts("", 0, 0);
    REQUIRE(result.empty());
}

TEST(single_paragraph_full_selection) {
    // "Hello world" -- single paragraph, no trailing newline.
    auto result = getSelectedParagraphStarts("Hello world", 0, 11);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

TEST(single_paragraph_partial_selection_mid_paragraph) {
    // "Hello world" -- selecting "llo w"
    auto result = getSelectedParagraphStarts("Hello world", 2, 7);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

TEST(single_paragraph_collapsed_cursor_at_start) {
    auto result = getSelectedParagraphStarts("Hello world", 0, 0);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

TEST(single_paragraph_collapsed_cursor_in_middle) {
    auto result = getSelectedParagraphStarts("Hello world", 5, 5);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

// ---------------------------------------------------------------------------
// Multi-paragraph -- the primary feature
// ---------------------------------------------------------------------------

TEST(two_paragraphs_selection_covers_both_fully) {
    // "Line1\nLine2" -- length 11
    auto result = getSelectedParagraphStarts("Line1\nLine2", 0, 11);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 0);   // "Line1"
    CHECK(result[1] == 6);   // "Line2"
}

TEST(three_paragraphs_selection_covers_all_fully) {
    // "Line1\nLine2\nLine3" -- length 17
    auto result = getSelectedParagraphStarts("Line1\nLine2\nLine3", 0, 17);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == 0);   // "Line1"
    CHECK(result[1] == 6);   // "Line2"
    CHECK(result[2] == 12);  // "Line3"
}

TEST(two_paragraphs_selection_starts_in_first_ends_in_second_partial) {
    // "Line1\nLine2"
    // Select from position 2 ('n') to position 9 ('n' of Line2)
    auto result = getSelectedParagraphStarts("Line1\nLine2", 2, 9);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 0);   // "Line1"
    CHECK(result[1] == 6);   // "Line2"
}

TEST(two_paragraphs_selection_ends_exactly_at_newline) {
    // "Line1\nLine2"
    // Select from position 2 to position 5 (right at the '\n')
    // sel_end=6 means positions 2..5 are selected: "ne1\n"
    // The '\n' at position 5 has pos+1 = 6 which is NOT < sel_end (6), so
    // Line2 should NOT be included.
    auto result = getSelectedParagraphStarts("Line1\nLine2", 2, 6);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

// ---------------------------------------------------------------------------
// REGRESSION: selection ending exactly at newline
// ---------------------------------------------------------------------------

TEST(regression_full_selection_ending_at_newline_excludes_next_paragraph) {
    // "ABC\nDEF\n" -- length 8.  Position 7 is the trailing newline.
    // Select [0, 8): includes "\n" at position 7.
    // pos=7: text[7]='\n', 7+1=8, 8 < 8 is FALSE -> don't include paragraph
    // after position 7.
    // This is a regression test: if the comparison were <= instead of <,
    // position 8 would be pushed and an empty/bogus paragraph included.
    auto result = getSelectedParagraphStarts("ABC\nDEF\n", 0, 8);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 0);  // "ABC"
    CHECK(result[1] == 4);  // "DEF"
}

TEST(regression_selection_ending_at_middle_newline_excludes_only_the_next) {
    // "Line1\nLine2\nLine3" -- length 17
    // Select [0, 12): "Line1\nLine2" plus the '\n' at position 11.
    // sel_end = 12.  '\n' at pos=11: 11+1=12, 12 < 12 is FALSE.
    // Line3 must NOT be included.
    auto result = getSelectedParagraphStarts("Line1\nLine2\nLine3", 0, 12);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 0);   // "Line1"
    CHECK(result[1] == 6);   // "Line2"
    // Line3 at offset 12 must NOT be present.
    if (result.size() > 2) {
        std::ostringstream oss;
        oss << "Line3 was incorrectly included: result[2] = " << result[2];
        FAIL(oss.str());
    }
}

TEST(regression_selection_one_char_past_newline_includes_next_paragraph) {
    // "Line1\nLine2\nLine3" -- length 17
    // Select [0, 13): "Line1\nLine2\nL" -- the 'L' of Line3
    // sel_end = 13.  '\n' at pos=11: 11+1=12, 12 < 13 is TRUE -> include Line3.
    auto result = getSelectedParagraphStarts("Line1\nLine2\nLine3", 0, 13);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == 0);   // "Line1"
    CHECK(result[1] == 6);   // "Line2"
    CHECK(result[2] == 12);  // "Line3"
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(single_character_selection_at_newline_itself) {
    // "A\nB" -- length 3.  Select just the '\n' at position 1.
    // sel_end = 2.  '\n' at pos=1: 1+1=2, 2 < 2 is FALSE.
    // Only paragraph A should be returned (B is excluded).
    auto result = getSelectedParagraphStarts("A\nB", 1, 2);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

TEST(collapsed_cursor_on_newline_between_paragraphs) {
    // "A\nB" -- collapsed cursor at position 1 (on the '\n').
    // Expands backward to start of A, forward to include just A.
    auto result = getSelectedParagraphStarts("A\nB", 1, 1);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

TEST(collapsed_cursor_at_very_end_of_text) {
    // "Hello" -- cursor past the last character.
    auto result = getSelectedParagraphStarts("Hello", 5, 5);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

TEST(selection_past_end_of_text_is_clamped) {
    auto result = getSelectedParagraphStarts("Hi", 10, 20);
    // Clamped to [2, 2] -> collapsed at end of "Hi" -> paragraph "Hi" at 0.
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

TEST(text_with_only_newlines) {
    // "\n\n" -- length 2
    // Select all: [0, 2).  '\n' at pos=0: 0+1=1 < 2 is TRUE -> push 1.
    // '\n' at pos=1: 1+1=2 < 2 is FALSE -> stop.
    auto result = getSelectedParagraphStarts("\n\n", 0, 2);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 0);
    CHECK(result[1] == 1);
}

TEST(paragraph_with_leading_indent_tag_is_handled_correctly) {
    // "|INDENT:0.50:0.00:0.00|Bullet text\nNext para"
    // When we ask for the paragraph of "Bullet text" at offset ~30,
    // the indent tag is skipped and the '|UL:' detection looks
    // *after* the indent tag.  The paragraph boundary detection
    // itself should still return the full paragraph from 0.
    std::string text = "|INDENT:0.50:0.00:0.00|Bullet text\nNext para";
    auto result = getSelectedParagraphStarts(text, 30, 30); // cursor on "Bullet"
    REQUIRE(result.size() == 1);
    CHECK(result[0] == 0);
}

// ---------------------------------------------------------------------------
// Multi-paragraph with leading indent tags
// ---------------------------------------------------------------------------

TEST(multi_paragraph_with_indent_tags_selection_covers_both) {
    std::string text = "|INDENT:0.50:0.00:0.00|Item A\n|INDENT:0.50:0.00:0.00|Item B\n";
    // Select everything up to end (before trailing newline's effect).
    auto result = getSelectedParagraphStarts(text, 0, static_cast<uint32_t>(text.length()));
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 0);                            // "|INDENT...|Item A"
    CHECK(result[1] == 30);                           // "|INDENT...|Item B"
}

// ---------------------------------------------------------------------------
// Trailing newline variants
// ---------------------------------------------------------------------------

TEST(text_with_trailing_newline_full_selection) {
    // "Line1\nLine2\n" -- length 13  (6 + 6 + 1 trailing? Actually "Line1" is 5 + '\n' = 6)
    // "Line1\nLine2\n" -- let's count: L(0) i(1) n(2) e(3) 1(4) \n(5) L(6) i(7) n(8) e(9) 2(10) \n(11).. wait length is 12.
    // Actually: L=0,i=1,n=2,e=3,1=4,\n=5,L=6,i=7,n=8,e=9,2=10,\n=11 — length = 12.
    // Select [0, 13) would be past end. But test in Catch2 used 13. Let me check Catch2 original:
    // Original: getSelectedParagraphStarts("Line1\nLine2\n", 0, 13) with length 13.
    // "Line1\n" = 6, "Line2\n" = 6, total = 12. Hmm. Actually original test said length 13.
    // Let me just use the same -- the function clamps anyway.
    auto result = getSelectedParagraphStarts("Line1\nLine2\n", 0, 13);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 0);   // "Line1"
    CHECK(result[1] == 6);   // "Line2"
}

TEST(text_with_trailing_newline_partial_selection_excluding_newline) {
    // "Line1\nLine2\n"
    // Actually length is 12. Select [0, 11): "Line1\nLine2" -- sel_end = 11.
    auto result = getSelectedParagraphStarts("Line1\nLine2\n", 0, 11);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 0);
    CHECK(result[1] == 6);
}

// ============================================================================
// Test registry
// ============================================================================

TestCase g_tests[] = {
    { "Empty text returns empty vector",                              empty_text_returns_empty_vector },
    { "Single paragraph, full selection",                             single_paragraph_full_selection },
    { "Single paragraph, partial selection (mid-paragraph)",          single_paragraph_partial_selection_mid_paragraph },
    { "Single paragraph, collapsed cursor at start",                  single_paragraph_collapsed_cursor_at_start },
    { "Single paragraph, collapsed cursor in middle",                 single_paragraph_collapsed_cursor_in_middle },
    { "Two paragraphs, selection covers both fully",                  two_paragraphs_selection_covers_both_fully },
    { "Three paragraphs, selection covers all fully",                 three_paragraphs_selection_covers_all_fully },
    { "Two paragraphs, selection starts in first and ends in second (partial)",
      two_paragraphs_selection_starts_in_first_ends_in_second_partial },
    { "Two paragraphs, selection ends exactly at newline",            two_paragraphs_selection_ends_exactly_at_newline },
    { "REGRESSION: full selection ending at newline excludes next paragraph",
      regression_full_selection_ending_at_newline_excludes_next_paragraph },
    { "REGRESSION: selection ending at middle newline excludes only the next",
      regression_selection_ending_at_middle_newline_excludes_only_the_next },
    { "REGRESSION: selection one char past newline includes next paragraph",
      regression_selection_one_char_past_newline_includes_next_paragraph },
    { "Single character selection at newline boundary (newline itself)",
      single_character_selection_at_newline_itself },
    { "Collapsed cursor on newline between paragraphs",               collapsed_cursor_on_newline_between_paragraphs },
    { "Collapsed cursor at very end of text",                         collapsed_cursor_at_very_end_of_text },
    { "Selection past end of text is clamped",                        selection_past_end_of_text_is_clamped },
    { "Text with only newlines",                                      text_with_only_newlines },
    { "Paragraph with leading indent tag is handled correctly",       paragraph_with_leading_indent_tag_is_handled_correctly },
    { "Multi-paragraph with indent tags, selection covers both",      multi_paragraph_with_indent_tags_selection_covers_both },
    { "Text with trailing newline, full selection",                   text_with_trailing_newline_full_selection },
    { "Text with trailing newline, partial selection excluding newline",
      text_with_trailing_newline_partial_selection_excluding_newline },
};

constexpr int kNumTests = sizeof(g_tests) / sizeof(g_tests[0]);

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "Paragraph regression tests\n";
    std::cout << "==========================\n\n";

    for (int i = 0; i < kNumTests; ++i) {
        RUN_TEST(g_tests[i]);
    }

    std::cout << "\n==========================\n";
    std::cout << g_test_count << " tests, "
              << g_test_passed << " passed, "
              << (g_test_count - g_test_passed) << " failed, "
              << g_test_failures << " total assertions failed\n";

    return g_test_failures > 0 ? 1 : 0;
}
