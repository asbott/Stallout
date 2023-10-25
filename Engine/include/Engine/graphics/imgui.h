#pragma once

#include "Engine/graphics/graphics_driver.h"
#include "Engine/graphics/imgui_renderer.h"

NS_BEGIN(os);
struct Window;
NS_END(os);

NS_BEGIN(engine);
NS_BEGIN(imgui);

enum Window_Flags {
    WINDOW_FLAGS_NONE                            = 0,
    WINDOW_FLAGS_NO_TITLE_BAR                    = 1 << 0,
    WINDOW_FLAGS_NO_RESIZE                       = 1 << 1,
    WINDOW_FLAGS_NO_MOVE                         = 1 << 2,
    WINDOW_FLAGS_NO_SCROLLBAR                    = 1 << 3,
    WINDOW_FLAGS_NO_SCROLL_WITH_MOUSE            = 1 << 4,
    WINDOW_FLAGS_NO_COLLAPSE                     = 1 << 5,
    WINDOW_FLAGS_ALWAYS_AUTO_RESIZE              = 1 << 6,
    WINDOW_FLAGS_NO_BACKGROUND                   = 1 << 7,
    WINDOW_FLAGS_NO_SAVED_SETTINGS               = 1 << 8,
    WINDOW_FLAGS_NO_MOUSE_INPUTS                 = 1 << 9,
    WINDOW_FLAGS_MENU_BAR                        = 1 << 10,
    WINDOW_FLAGS_HORIZONTAL_SCROLLBAR            = 1 << 11,
    WINDOW_FLAGS_NO_FOCUS_ON_APPEARING           = 1 << 12,
    WINDOW_FLAGS_NO_BRING_TO_FRONT_ON_FOCUS      = 1 << 13,
    WINDOW_FLAGS_ALWAYS_VERTICAL_SCROLLBAR       = 1 << 14,
    WINDOW_FLAGS_ALWAYS_HORIZONTAL_SCROLLBAR     = 1 << 15,
    WINDOW_FLAGS_ALWAYS_USE_WINDOW_PADDING       = 1 << 16,
    WINDOW_FLAGS_NO_NAV_INPUTS                   = 1 << 18,
    WINDOW_FLAGS_NO_NAV_FOCUS                    = 1 << 19,
    WINDOW_FLAGS_UNSAVED_DOCUMENT                = 1 << 20,
    WINDOW_FLAGS_NO_DOCKING                      = 1 << 21,

    WINDOW_FLAGS_NO_NAV                          = WINDOW_FLAGS_NO_NAV_INPUTS | WINDOW_FLAGS_NO_NAV_FOCUS,
    WINDOW_FLAGS_NO_DECORATION                   = WINDOW_FLAGS_NO_TITLE_BAR | WINDOW_FLAGS_NO_RESIZE | WINDOW_FLAGS_NO_SCROLLBAR | WINDOW_FLAGS_NO_COLLAPSE,
    WINDOW_FLAGS_NO_INPUTS                       = WINDOW_FLAGS_NO_MOUSE_INPUTS | WINDOW_FLAGS_NO_NAV_INPUTS | WINDOW_FLAGS_NO_NAV_FOCUS,
};
FLAGIFY(Window_Flags);

enum Popup_Flags
{
    POPUP_FLAGS_NONE                    = 0,
    POPUP_FLAGS_MOUSE_BUTTON_LEFT         = 0,        // For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as ImGuiMouseButton_Left)
    POPUP_FLAGS_MOUSE_BUTTON_RIGHT        = 1,        // For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as ImGuiMouseButton_Right)
    POPUP_FLAGS_MOUSE_BUTTON_MIDDLE       = 2,        // For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as ImGuiMouseButton_Middle)
    POPUP_FLAGS_NO_OPEN_OVER_EXISTING_POPUP = 1 << 5,   // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
    POPUP_FLAGS_NO_OPEN_OVER_ITEMS         = 1 << 6,   // For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
    POPUP_FLAGS_ANY_POPUP_ID              = 1 << 7,   // For IsPopupOpen(): ignore the ImGuiID parameter and test for any popup.
    POPUP_FLAGS_ANY_POPUP_LEVEL           = 1 << 8,   // For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
    POPUP_FLAGS_ANY_POPUP                = POPUP_FLAGS_ANY_POPUP_ID | POPUP_FLAGS_ANY_POPUP_LEVEL,
};
FLAGIFY(Popup_Flags);

enum Slider_Flags
{
    SLIDER_FLAGS_NONE                   = 0,
    SLIDER_FLAGS_ALWAYS_CLAMP            = 1 << 4,       // Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.
    SLIDER_FLAGS_LOGARITHMIC            = 1 << 5,       // Make the widget logarithmic (linear otherwise). Consider using ImGuiSliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.
    SLIDER_FLAGS_NO_ROUND_FORMAT        = 1 << 6,       // Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits)
    SLIDER_FLAGS_NO_INPUT                = 1 << 7,       // Disable CTRL+Click or Enter key allowing to input text directly into the widget
};
FLAGIFY(Slider_Flags);

enum Selectable_Flags
{
    SELECTABLE_FLAGS_NONE               = 0,
    SELECTABLE_FLAGS_DONT_CLOSE_POPUPS    = 1 << 0,   // Clicking this doesn't close parent popup window
    SELECTABLE_FLAGS_SPAN_ALL_COLUMNS     = 1 << 1,   // Selectable frame can span all columns (text will still fit in current column)
    SELECTABLE_FLAGS_ALLOW_DOUBLE_CLICK   = 1 << 2,   // Generate press events on double clicks too
    SELECTABLE_FLAGS_DISABLED           = 1 << 3,   // Cannot be selected, display grayed out text
    SELECTABLE_FLAGS_ALLOW_OVERLAP       = 1 << 4,   // (WIP) Hit testing to allow subsequent widgets to overlap this one
};
FLAGIFY(Selectable_Flags);

enum Input_Text_Flags
{
    INPUT_TEXT_FLAGS_NONE                     = 0,
    INPUT_TEXT_FLAGS_CHARS_DECIMAL            = 1 << 0,   // Allow 0123456789.+-*/
    INPUT_TEXT_FLAGS_CHARS_HEXDECIMAL         = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    INPUT_TEXT_FLAGS_CHARS_UPPERCASE          = 1 << 2,   // Turn a..z into A..Z
    INPUT_TEXT_FLAGS_CHARS_NO_BLANK           = 1 << 3,   // Filter out spaces, tabs
    INPUT_TEXT_FLAGS_AUTO_SELECT_ALL          = 1 << 4,   // Select entire text when first taking mouse focus
    INPUT_TEXT_FLAGS_ENTER_RETURNS_TRUE       = 1 << 5,   // Return 'true' when Enter is pressed 
    INPUT_TEXT_FLAGS_CALLBACK_COMPLETION      = 1 << 6,   // Callback on pressing TAB (for completion handling)
    INPUT_TEXT_FLAGS_CALLBACK_HISTORY         = 1 << 7,   // Callback on pressing Up/Down arrows (for history handling)
    INPUT_TEXT_FLAGS_CALLBACK_ALWAYS          = 1 << 8,   // Callback on each iteration.
    INPUT_TEXT_FLAGS_CALLBACK_CHAR_FILTER     = 1 << 9,   // Callback on character inputs to replace or discard them.
    INPUT_TEXT_FLAGS_ALLOW_TAB_INPUT          = 1 << 10,  // Pressing TAB input a '\t' character into the text field
    INPUT_TEXT_FLAGS_CTRL_ENTER_FOR_NEW_LINE  = 1 << 11,  // In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter.
    INPUT_TEXT_FLAGS_NO_HORIZONTAL_SCROLL     = 1 << 12,  // Disable following the cursor horizontally
    INPUT_TEXT_FLAGS_ALWAYS_OVERWRITE         = 1 << 13,  // Overwrite mode
    INPUT_TEXT_FLAGS_READ_ONLY                = 1 << 14,  // Read-only mode
    INPUT_TEXT_FLAGS_PASSWORD                 = 1 << 15,  // Password mode, display all characters as '*'
    INPUT_TEXT_FLAGS_NO_UNDO_REDO             = 1 << 16,  // Disable undo/redo.
    INPUT_TEXT_FLAGS_CHARS_SCIENTIFIC         = 1 << 17,  // Allow 0123456789.+-*/eE (Scientific notation input)
    INPUT_TEXT_FLAGS_CALLBACK_RESIZE          = 1 << 18,  // Callback on buffer capacity changes request 
    INPUT_TEXT_FLAGS_CALLBACK_EDIT            = 1 << 19,  // Callback on any edit
    INPUT_TEXT_FLAGS_ESCAPE_CLEARS_ALL        = 1 << 20,  // Escape key clears content if not empty, and deactivate otherwise
};
FLAGIFY(Input_Text_Flags);

using namespace graphics;

ST_API void init(Graphics_Driver* driver, os::Window* window);

ST_API void new_frame(f32 delta_seconds);
ST_API void render();

ST_API bool begin_window(const char* name, bool* open = NULL, Window_Flags flags = WINDOW_FLAGS_NONE);
ST_API void end_window();

ST_API bool text_input(const char* name, char* buffer, size_t buffer_sz, Input_Text_Flags flags = INPUT_TEXT_FLAGS_NONE);

ST_API bool f32_drag(const char* name, f32& value, f32 speed = .5f, f32 min = 0.f, f32 max = 0.f, const char* fmt = "%.3f", Slider_Flags flags = SLIDER_FLAGS_NONE);
ST_API bool f32vec2_drag(const char* name, mz::f32vec2& value, f32 speed = .5f, f32 min = 0.f, f32 max = 0.f, const char* fmt = "%.3f", Slider_Flags flags = SLIDER_FLAGS_NONE);
ST_API bool f32vec3_drag(const char* name, mz::f32vec3& value, f32 speed = .5f, f32 min = 0.f, f32 max = 0.f, const char* fmt = "%.3f", Slider_Flags flags = SLIDER_FLAGS_NONE);
ST_API bool f32vec4_drag(const char* name, mz::f32vec4& value, f32 speed = .5f, f32 min = 0.f, f32 max = 0.f, const char* fmt = "%.3f", Slider_Flags flags = SLIDER_FLAGS_NONE);

ST_API bool s32_drag(const char* name, s32& value, f32 speed = .5f, s32 min = 0.f, s32 max = 0.f, const char* fmt = "%d", Slider_Flags flags = SLIDER_FLAGS_NONE);
ST_API bool s32vec2_drag(const char* name, mz::s32vec2& value, f32 speed = .5f, s32 min = 0.f, s32 max = 0.f, const char* fmt = "%d", Slider_Flags flags = SLIDER_FLAGS_NONE);
ST_API bool s32vec3_drag(const char* name, mz::s32vec3& value, f32 speed = .5f, s32 min = 0.f, s32 max = 0.f, const char* fmt = "%d", Slider_Flags flags = SLIDER_FLAGS_NONE);
ST_API bool s32vec4_drag(const char* name, mz::s32vec4& value, f32 speed = .5f, s32 min = 0.f, s32 max = 0.f, const char* fmt = "%d", Slider_Flags flags = SLIDER_FLAGS_NONE);

ST_API bool checkbox(const char* name, bool& value);

ST_API bool begin_combo(const char* name, const char* preview);
ST_API void end_combo();

ST_API void text(const char* fmt, ...);

ST_API bool button(const char* name, mz::fvec2 size = 0);
ST_API bool menu_item(const char* name, const char* shortcut = 0, bool* selected = 0, bool enabled = true);
ST_API bool selectable(const char* name, bool* selected = 0, Selectable_Flags flags = SELECTABLE_FLAGS_NONE, mz::fvec2 size = 0);


ST_API void image(Resource_Handle htex, mz::fvec2 size, mz::fvec2 uva = 0, mz::fvec2 uvb = 1, mz::color tint = mz::COLOR_WHITE, mz::color border = mz::COLOR_TRANSPARENT);

ST_API void draw_line(mz::fvec2 a, mz::fvec2 b, mz::color color = mz::COLOR_WHITE, f32 thickness = 1.f);

ST_API mz::frect get_window_content_area();
ST_API mz::fvec2 get_mouse_pos();

ST_API bool mouse_clicked(int btn = 0);
ST_API bool double_click(int btn = 0);

ST_API mz::fvec2 get_mouse_drag(int btn = 0, f32 threshold = -1.f);
ST_API void reset_mouse_drag(int btn = 0);

ST_API bool is_item_hovered();
ST_API bool is_item_active();
ST_API bool is_item_clicked(int btn = 0);
ST_API bool is_item_double_clicked(int btn = 0);

ST_API bool begin_menu_bar();
ST_API void end_menu_bar();

ST_API bool begin_popup(const char* name, Window_Flags flags = WINDOW_FLAGS_NONE);
ST_API void end_popup();
ST_API void open_popup(const char* name, Popup_Flags flags = POPUP_FLAGS_NONE);

ST_API void sameline(f32 offset = 0.f, f32 spacing = -1.f);
ST_API void indent(f32 width = 0.f);

NS_END(imgui);
NS_END(engine);