#include "pch.h"

#include "graphics/imgui.h"

#include <imgui.h>

NS_BEGIN(engine);
NS_BEGIN(imgui);

ImGui_Renderer* rend = NULL;

void make_current() {
    ST_ASSERT(rend, "ST imgui not initialized. Call st::imgui::init().");

    ImGui::SetCurrentContext((ImGuiContext*)rend->_imgui_context);
}

void init(Graphics_Driver* driver, os::Window* window) {
    rend = stnew (ImGui_Renderer) (driver, window);

    window->add_event_callback([](os::Window* window, os::Window_Event_Type etype, void* param, void* ud)-> bool  {
        return rend->handle_window_event(window, etype, param, ud);
    }, rend);
}

void new_frame(f32 delta_seconds) {
    make_current();
    rend->new_frame(delta_seconds);
}
void render() {
    make_current();
    rend->render();
}

bool begin_window(const char* name, bool* open, Window_Flags flags) {
    make_current();
    return ImGui::Begin(name, open, (ImGuiWindowFlags)flags);
}
void end_window() {
    make_current();
    ImGui::End();
}

bool text_input(const char* name, char* buffer, size_t buffer_sz, Input_Text_Flags flags) {
    make_current();

    return ImGui::InputText(name, buffer, buffer_sz, (ImGuiInputTextFlags)flags);
}
bool f32_drag(const char* name, f32& value, f32 speed, f32 min, f32 max, const char* fmt, Slider_Flags flags) {
    make_current();
    return ImGui::DragFloat(name, &value, speed, min, max, fmt, flags);
}
bool f32vec2_drag(const char* name, mz::f32vec2& value, f32 speed, f32 min, f32 max, const char* fmt, Slider_Flags flags) {
    make_current();
    return ImGui::DragFloat2(name, value, speed, min, max, fmt, flags);
}
bool f32vec3_drag(const char* name, mz::f32vec3& value, f32 speed, f32 min, f32 max, const char* fmt, Slider_Flags flags) {
    make_current();
    return ImGui::DragFloat3(name, value, speed, min, max, fmt, flags);
}
bool f32vec4_drag(const char* name, mz::f32vec4& value, f32 speed, f32 min, f32 max, const char* fmt, Slider_Flags flags) {
    make_current();
    return ImGui::DragFloat4(name, value, speed, min, max, fmt, flags);
}

bool s32_drag(const char* name, s32& value, f32 speed, s32 min, s32 max, const char* fmt, Slider_Flags flags) {
    make_current();
    return ImGui::DragInt(name, &value, speed, min, max, fmt, flags);
}
bool s32vec2_drag(const char* name, mz::s32vec2& value, f32 speed, s32 min, s32 max, const char* fmt, Slider_Flags flags) {
    make_current();
    return ImGui::DragInt2(name, value, speed, min, max, fmt, flags);
}
bool s32vec3_drag(const char* name, mz::s32vec3& value, f32 speed, s32 min, s32 max, const char* fmt, Slider_Flags flags) {
    make_current();
    return ImGui::DragInt3(name, value, speed, min, max, fmt, flags);
}
bool s32vec4_drag(const char* name, mz::s32vec4& value, f32 speed, s32 min, s32 max, const char* fmt, Slider_Flags flags) {
    make_current();
    return ImGui::DragInt4(name, value, speed, min, max, fmt, flags);
}

void text(const char* fmt, ...) {
    make_current();
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

bool button(const char* name, mz::fvec2 size) {
    make_current();
    return ImGui::Button(name, ImVec2(size.x, size.y));
}
bool menu_item(const char* name, const char* shortcut, bool* selected, bool enabled) {
    make_current();    
    return ImGui::MenuItem(name, shortcut, selected, enabled);
}
bool selectable(const char* name, bool* selected, Selectable_Flags flags, mz::fvec2 size) {
    make_current();

    return ImGui::Selectable(name, selected, (ImGuiSelectableFlags)flags, ImVec2(size.x, size.y));
}

void image(Resource_Handle htex, mz::fvec2 size, mz::fvec2 uva, mz::fvec2 uvb, mz::color tint, mz::color border) {
    make_current();
    ImGui::Image(
        (ImTextureID)htex, 
        ImVec2(size.x, size.y), 
        ImVec2(uva.x, uva.y), 
        ImVec2(uvb.x, uvb.y), 
        ImVec4(tint.r, tint.g, tint.b, tint.a),
        ImVec4(border.r, border.g, border.b, border.a)
    ); 
}

void draw_line(mz::fvec2 a, mz::fvec2 b, mz::color color, f32 thickness) {
    make_current();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec2 wnd_pos = ImGui::GetWindowPos();
    ImVec2 content_min = ImGui::GetWindowContentRegionMin();

    ImVec2 offset(wnd_pos.x + content_min.x, wnd_pos.y + content_min.y);

    draw_list->AddLine(
        ImVec2(a.x + offset.x, a.y + offset.y),
        ImVec2(b.x + offset.x, b.y + offset.y),
        IM_COL32(
            (int)(255 * color.r),
            (int)(255 * color.g),
            (int)(255 * color.b),
            (int)(255 * color.a)
        ),
        thickness
    );
}

mz::frect get_window_content_area() {
    make_current();
    auto min = ImGui::GetWindowContentRegionMin();
    auto max = ImGui::GetWindowContentRegionMax();

    auto width  = max.x - min.x;
    auto height = max.y - min.y;

    return mz::frect(min.x, min.y, width, height);
}

mz::fvec2 get_mouse_pos() {
    make_current();
    ImVec2 mouse_pos = ImGui::GetMousePos();
    ImVec2 wnd_pos = ImGui::GetWindowPos();
    ImVec2 content_min = ImGui::GetWindowContentRegionMin();

    ImVec2 relative_pos = ImVec2(mouse_pos.x - (wnd_pos.x + content_min.x), mouse_pos.y - (wnd_pos.y + content_min.y));

    return { relative_pos.x, relative_pos.y };
}

bool mouse_clicked(int btn) {
    make_current();

    return ImGui::IsMouseClicked(btn);
}

bool double_click(int btn) {
    return ImGui::IsMouseDoubleClicked(btn);
}

mz::fvec2 get_mouse_drag(int btn, f32 threshold) {
    make_current();
    auto v = ImGui::GetMouseDragDelta(btn, threshold);

    return {v.x, v.y};
}
void reset_mouse_drag(int btn) {
    make_current();

    ImGui::ResetMouseDragDelta(btn);
}

bool is_item_hovered() {
    make_current();
    return ImGui::IsItemHovered();
}
bool is_item_active() {
    make_current();

    return ImGui::IsItemActive();
}

bool is_item_clicked(int btn) {
    make_current();

    return ImGui::IsItemClicked(btn);
}
bool is_item_double_clicked(int btn) {
    make_current();

    return ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(btn);
}

bool begin_menu_bar() {
    make_current();

    return ImGui::BeginMenuBar();
}
void end_menu_bar() {
    make_current();
    
    ImGui::EndMenuBar();
}

bool begin_popup(const char* name, Window_Flags flags) {
    make_current();

    return ImGui::BeginPopup(name, (ImGuiWindowFlags)flags);
}

void end_popup() {
    make_current();

    ImGui::EndPopup();
}

void open_popup(const char* name, Popup_Flags flags) {
    make_current();

    ImGui::OpenPopup(name, (ImGuiPopupFlags)flags);
}

void sameline(f32 offset, f32 spacing) {
    make_current();
    ImGui::SameLine(offset, spacing);
}

void indent(f32 width) {
    make_current();

    ImGui::Indent(width);
}

bool begin_combo(const char* name, const char* preview) {
    make_current();

    return ImGui::BeginCombo(name, preview);
}
bool checkbox(const char* name, bool& value) {
    make_current();

    return ImGui::Checkbox(name, &value);
}
void end_combo() {
    make_current();

    ImGui::EndCombo();
}

NS_END(imgui);
NS_END(engine);