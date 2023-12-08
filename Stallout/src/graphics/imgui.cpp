#include "pch.h"

#include "graphics/imgui.h"

#include <imgui.h>

#include "os/io.h"

NS_BEGIN(stallout);
NS_BEGIN(imgui);

ImGui_Renderer* rend = NULL;

struct File_Dialog_Context {
    String filter = "";
    String title = "Title";
    String current_dir;
    File_Dialog_Flags flags = FILE_DIALOG_FLAGS_NONE;

    bool is_open = false;
    bool need_open = false;

    Array<String> dirs;
    Array<String> files;
    Array<String> root_dirs;
    String current_root;

    Hash_Set<String> selected;
    Hash_Set<String> filter_set;
    Array<String> filter_arr;

    Hash_Map<String, char*> file_sizes;

    bool need_update = false;
    std::function<void(Array<String>&)> callback;

    char create_filename[ST_MAX_PATH] = "new_file";

    bool renaming = false;
    bool was_renaming = false;
    char temp_dir_name[ST_MAX_PATH] = "";
};
File_Dialog_Context* file_dialog_ctx = NULL;

struct Message_Box_Context {
    String msg = "";
    std::function<void(bool)> callback;
    Message_Box_Flags flags = MESSAGE_BOX_FLAGS_OKCANCEL;
    bool is_open = false;
    bool need_open = false;
};
Message_Box_Context* msg_box_ctx = NULL;

void make_current() {
    ST_ASSERT(rend, "ST imgui not initialized. Call st::imgui::init().");

    if (!file_dialog_ctx) {
        // TODO: #memory Memory Leak
        file_dialog_ctx = stnew (File_Dialog_Context);
        msg_box_ctx = stnew (Message_Box_Context);
    }


    ImGui::SetCurrentContext((ImGuiContext*)rend->_imgui_context);
}

void init(Graphics_Driver* driver, os::Window* window) {
    rend = stnew (ImGui_Renderer) (driver, window);
    
    window->add_event_callback([](os::Window* window, os::Window_Event_Type etype, void* param, void* ud)-> bool  {
        return rend->handle_window_event(window, etype, param, ud);
    }, rend);
}

void set_renderer(ImGui_Renderer* _rend) {
    rend = _rend;
}
ImGui_Renderer* get_renderer() {
    return rend;
}

void new_frame(f32 delta_seconds) {
    make_current();
    rend->new_frame(delta_seconds);
}
void render() {
    make_current();

    show_message_box();
    show_file_dialog();

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

bool color_input(const char* name, mz::color& value) {
    return ImGui::ColorEdit4(name, value.ptr);
}

void text(const char* fmt, ...) {
    make_current();
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

void colored_text(mz::color color, const char* fmt, ...) {
    make_current();
    va_list args;
    va_start(args, fmt);
    ImGui::TextColoredV(ImVec4(color.r, color.g, color.b, color.a), fmt, args);
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
mz::fvec2 get_item_pos() {
    auto imvec = ImGui::GetItemRectMin();
    auto wnd = ImGui::GetWindowPos();
    f32 scroll = ImGui::GetScrollY();
    float title = ImGui::GetStyle().FramePadding.y * 2.0f + ImGui::GetFontSize();    
    return { imvec.x - wnd.x - ImGui::GetStyle().WindowPadding.x, imvec.y - wnd.y + scroll - ImGui::GetStyle().WindowPadding.y - title };
}
mz::fvec2 get_item_size() {
    auto imvec = ImGui::GetItemRectSize();
    return { imvec.x, imvec.y };
}

bool begin_menu_bar() {
    make_current();

    return ImGui::BeginMenuBar();
}
void end_menu_bar() {
    make_current();
    
    ImGui::EndMenuBar();
}

bool begin_main_menu_bar() {
    make_current();

    return ImGui::BeginMainMenuBar();
}
void end_main_menu_bar() {
    make_current();
    
    ImGui::EndMainMenuBar();
}

bool push_treenode(const char* name) {
    make_current();

    return ImGui::TreeNode(name);
}
void pop_treenode() {
    make_current();

    ImGui::TreePop();
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

void push_id(const char* id) {
    make_current();

    ImGui::PushID(id);
}
void push_id(const int id) {
    make_current();

    ImGui::PushID(id);
}
void pop_id() {
    make_current();

    ImGui::PopID();
}


void open_file_dialog(const char* title, const std::function<void(Array<String>&)>& callback, const char* filter, File_Dialog_Flags flags) {
    make_current(); 

    ST_DEBUG_ASSERT(!file_dialog_ctx->is_open, "Only one file dialog can be active at a time");

    file_dialog_ctx->need_open = true;
    file_dialog_ctx->title = title;
    file_dialog_ctx->filter = filter;
    file_dialog_ctx->is_open = true;
    file_dialog_ctx->flags = flags;
    file_dialog_ctx->current_dir = os::io::get_workspace_dir();
    file_dialog_ctx->need_update = true;
    file_dialog_ctx->callback = callback;

    if (flags & FILE_DIALOG_FLAGS_CREATE_FILE) {
        ST_DEBUG_ASSERT(!(flags & FILE_DIALOG_FLAGS_DIRECTORY), "Cannot enabled directory selection and file creation");
        ST_DEBUG_ASSERT(!(flags & FILE_DIALOG_FLAGS_ALLOW_MULTIPLE), "Cannot create multiple files");
    }
}
void close_file_dialog() {
    make_current();

    file_dialog_ctx->is_open = false;
}
bool special_selectable(const char* name, bool selected) {
    // Backup styles
    ImVec4 col_text = ImGui::GetStyle().Colors[ImGuiCol_Text];
    ImVec4 col_text_disabled = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];

    bool result = ImGui::Selectable(name, selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);


    // Restore styles
    ImGui::GetStyle().Colors[ImGuiCol_Text] = col_text;
    ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = col_text_disabled;

    return result;
}
bool show_file_dialog() {
    make_current();

    auto ctx = file_dialog_ctx;

    if (file_dialog_ctx->need_open) {
        ImGui::OpenPopup("filedialog");
        ctx->need_open = false;
    }

    if (!ctx->is_open) return false;

    bool is_modal = file_dialog_ctx->flags & FILE_DIALOG_FLAGS_MODAL;

    bool popup_opened = is_modal ? 
                        ImGui::BeginPopupModal("filedialog", &file_dialog_ctx->is_open, ImGuiWindowFlags_NoTitleBar) :
                        ImGui::Begin("filedialog", &file_dialog_ctx->is_open, ImGuiWindowFlags_NoTitleBar);

    if (popup_opened && ctx->is_open) {

        ImGui::Text(file_dialog_ctx->title.str);
        ImGui::Spacing();
        ImGui::Text("Filter: %s", ctx->filter.str);

        bool double_clk = ImGui::IsMouseDoubleClicked(0);

        
        if (ctx->need_update) {
            ctx->need_update = false;
            for (auto& [_, value] : ctx->file_sizes) {
                ST_FREE(value, 20);
            }
            ctx->file_sizes.clear();
            ctx->dirs.clear();
            ctx->files.clear();
            ctx->selected.clear();
            ctx->filter_arr.clear();
            ctx->filter_set.clear();

            
            split_string(ctx->filter, ',', &ctx->filter_arr);

            for (auto& f : ctx->filter_arr) {
                ctx->filter_set.emplace(f);
            }

            // TODO: (2023-10-29) #feature #portability #UI #editor
            // This is only relevant if there are multiple drives
            // which is also only relevant in Windows OS, so should
            // just ignore if there's only 1 "root dir".
            
            ctx->root_dirs = os::io::get_root_dirs();
            for (auto& dir : ctx->root_dirs) {
                if (memcmp(dir.str, file_dialog_ctx->current_dir.str, dir.len()) == 0) {
                    ctx->current_root = dir;
                    break;
                }
            }
            ST_DEBUG_ASSERT(ctx->current_root);

            size_t nentries = 0;
            os::io::count_directory_entries(file_dialog_ctx->current_dir.str, &nentries);

            if (nentries) {
                char* entries = (char*)ST_MEM(nentries * MAX_PATH);

                ST_ASSERT(os::io::scan_directory(file_dialog_ctx->current_dir.str, entries) == IO_STATUS_OK);
                char* entry = entries;

                for (size_t i = 0; i < nentries; i++) {
                    if (strcmp(entry, ".") == 0 || strcmp(entry, "..") == 0) {
                        entry += strlen(entry) + 1;
                        continue;
                    }

                    char full_path[MAX_PATH];
                    sprintf(full_path, "%s/%s", file_dialog_ctx->current_dir.str, entry);

                    if (os::io::is_directory(full_path)) {
                        ctx->dirs.push_back(entry);
                    } else if (os::io::is_file(full_path)) {

                        auto ext = os::io::get_file_extension(full_path);
                        bool allowed = ctx->filter_set.size() == 0 || ctx->filter_set.contains(ext);

                        if (allowed) {
                            ctx->files.push_back(entry);

                            os::io::File_Info fifo;
                            os::io::get_file_info(full_path, &fifo);
                            char* file_sz = (char*)ST_MEM(20);

                            if (fifo.file_size < 1000) {
                                sprintf(file_sz, "%zu B", fifo.file_size);
                            } else if (fifo.file_size < 1000 * 1000) { // Less than 1MB
                                sprintf(file_sz, "%.2f KB", fifo.file_size / 1000.0);
                            } else if (fifo.file_size < 1000 * 1000 * 1000) {  // Less than 1GB
                                sprintf(file_sz, "%.2f MB", fifo.file_size / (1000.0 * 1000));
                            } else {
                                sprintf(file_sz, "%.2f GB", fifo.file_size / (1000.0 * 1000 * 1000));
                            }

                            if (ctx->file_sizes.contains(entry)) {
                                ST_FREE(ctx->file_sizes[entry], 20);
                            }
                            ctx->file_sizes[entry] = file_sz;
                        }
                    }

                    entry += strlen(entry) + 1;
                }

                ST_FREE(entries, nentries * MAX_PATH);
            }

        }


        bool select_files = true;
        bool select_dirs = false;

        bool allow_multiple = ctx->flags & FILE_DIALOG_FLAGS_ALLOW_MULTIPLE;

        if (ctx->flags & FILE_DIALOG_FLAGS_DIRECTORY) {
            select_files = (ctx->flags & FILE_DIALOG_FLAGS_FILE);
            select_dirs = true;
        }
        bool show_files = select_files;

        bool create_file = ctx->flags & FILE_DIALOG_FLAGS_CREATE_FILE;

        float text_width = ImGui::CalcTextSize("C:/").x;
        float handle_width = ImGui::GetStyle().FramePadding.x * 2;
        float combo_width = text_width + handle_width + 10.0f;

        if (ctx->root_dirs.size() > 1) {
            ImGui::SetNextItemWidth(combo_width);
            if (ImGui::BeginCombo("##Root", ctx->current_root.str)) {
                for (auto& root : ctx->root_dirs) {
                    if (ImGui::MenuItem(root.str)) {
                        ctx->current_root = root;
                        ctx->current_dir = root;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
        }


        Array<String> parts;
        split_string(file_dialog_ctx->current_dir, '/', &parts);

        size_t new_len = 0;
        for (size_t i = 0; i < parts.size(); i++) {
            if (i != 0) {
                ImGui::SameLine();
                ImGui::Text("/");
                ImGui::SameLine();
            }
            new_len += parts[i].len() + 1;

            if (ImGui::Button(parts[i].str)) {
                ctx->need_update = true;
                file_dialog_ctx->current_dir = String(new_len);

                for (size_t j = 0; j <= i; j++) {
                    file_dialog_ctx->current_dir.concat(parts[j].str);
                    if (j != i) file_dialog_ctx->current_dir.concat('/');
                }

                break;
            }
        }

        auto wnd_sz = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("browser", { wnd_sz.x, wnd_sz.y * .8f }, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {

            if (ImGui::MenuItem("New Directory")) {

                auto new_dir = ctx->current_dir;
                new_dir.concat("/New Directory");

                int i = 1;
                while (os::io::exists(new_dir.str)) {
                    new_dir = ctx->current_dir;
                    char dir_name[255];
                    sprintf(dir_name, "/New Directory(%i)", i++);
                    new_dir.concat(dir_name);
                }

                os::io::create_dir(new_dir.str);

                ctx->need_update = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Rename", 0, false, ctx->selected.size() == 1)) {
                ctx->renaming = true;
            }
            
            ImGui::Separator();

            if (ImGui::MenuItem("Delete", 0, false, ctx->selected.size() > 0)) {
                
                open_message_box("uh oh spaget");

            }

            ImGui::EndMenuBar();
        }

        if (special_selectable("..", false)) {
            if (double_clk) {
                ctx->need_update = true;
                parts.pop_back();
                file_dialog_ctx->current_dir = "";
                for (size_t i = 0; i < parts.size(); i++) {
                    file_dialog_ctx->current_dir.concat(parts[i].str);
                    if (i != parts.size()-1) file_dialog_ctx->current_dir.concat('/');
                }
            }
        }

        for (auto dir : ctx->dirs) {
            if (ctx->renaming && *ctx->selected.begin() == dir) {
                if (!ctx->was_renaming) {
                    ImGui::SetKeyboardFocusHere();
                }
                if (ImGui::InputText("##NewName", ctx->temp_dir_name, sizeof(ctx->temp_dir_name), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    String full_src = ctx->current_dir;
                    full_src.concat("/%s", dir.str);
                    String full_dst = ctx->current_dir;
                    full_dst.concat("/%s", ctx->temp_dir_name);
                    if (os::io::move(full_src.str, full_dst.str) == IO_STATUS_OK || ImGui::IsKeyDown(ImGuiKey_Escape)) {
                        ctx->renaming = false;
                        ctx->need_update = true;
                    }
                }
                if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered()) {
                    ctx->renaming = false;
                }
                
            } else if (special_selectable(dir.str, ctx->selected.contains(dir))) {
                if (double_clk) {
                    ctx->need_update = true;
                    file_dialog_ctx->current_dir.concat("/");
                    file_dialog_ctx->current_dir.concat(dir.str);
                    break;
                }

                if (select_dirs) {
                    if (allow_multiple && ImGui::IsWindowFocused() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                        ctx->selected.emplace(dir);
                    } else {
                        ctx->selected.clear();
                        ctx->selected.emplace(dir);
                    }
                }
            }
        }
        ctx->was_renaming = ctx->renaming;

        if (show_files) {
            for (auto file : ctx->files) {
                if (special_selectable(file.str, ctx->selected.contains(file))) {

                    if (select_files) {
                        if (allow_multiple && ImGui::IsWindowFocused() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                            ctx->selected.emplace(file);
                        } else {
                            ctx->selected.clear();
                            ctx->selected.emplace(file);
                        }

                        if (create_file) {
                            strcpy(ctx->create_filename, ctx->selected.begin()->str);
                        }
                    }
                }

                ST_DEBUG_ASSERT(ctx->file_sizes.contains(file.str));
                ImGui::SameLine();
                align_right(ImGui::CalcTextSize(ctx->file_sizes[file.str]).x);
                ImGui::Text(ctx->file_sizes[file.str]);

                
            }
        }

        ImGui::EndChild();

        if (create_file) {
            ImGui::InputText("Filename", ctx->create_filename, sizeof(ctx->create_filename), ImGuiInputTextFlags_CharsScientific);
        } else {
            if (ctx->selected.size() > 0) {
                ImGui::Text("Selection:");
                for (auto& path : ctx->selected) {
                    ImGui::SameLine();
                    ImGui::Text("%s", path.str);
                }
            } else {
                ImGui::Text("NONE");
            }
        }

        

        ImGui::Spacing();

        f32 width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.f;
        bool ok = ImGui::Button("Ok", ImVec2(width, 0));
        ImGui::SameLine();
        bool cancel = ImGui::Button("Cancel");

        bool done = ok || cancel;

        if (cancel) {
            ctx->selected.clear();
        }

        if (done) {
            Array<String> result;

            if (!cancel) {
                if (create_file) {
                    char full_path[ST_MAX_PATH];
                    sprintf(full_path, "%s/%s", ctx->current_dir.str, ctx->create_filename);
                    result.push_back(full_path);
                } else {
                    for (auto filename : ctx->selected) {
                        char full_path[ST_MAX_PATH];
                        sprintf(full_path, "%s/%s", ctx->current_dir.str, filename.str);

                        result.push_back(full_path);
                    }
                }
            }


            ImGui::CloseCurrentPopup();
            ctx->is_open = false;
            if (create_file) {
                if (!cancel && os::io::exists(result[0].str)) {
                    open_message_box("File already exists, would you like to replace it?", [result](bool ok) mutable {
                        if (!ok) result.clear();
                        if (file_dialog_ctx->callback) file_dialog_ctx->callback(result);
                    });
                } else {
                    if (ctx->callback) ctx->callback(result);    
                }
                
            } else {
                if (ctx->callback) ctx->callback(result);
            }

        }

        if (is_modal) {
            ImGui::EndPopup();
        } else {
            ImGui::End();
        }
        return true;
    }
    return false;
}

void open_message_box(const char* message, const std::function<void(bool)>& callback, Message_Box_Flags flags) {
    if (flags == MESSAGE_BOX_FLAGS_NONE) {
        if (callback) {
            flags |= MESSAGE_BOX_FLAGS_OKCANCEL;
        } else {
            flags |= MESSAGE_BOX_FLAGS_OK;
        }
    }

    msg_box_ctx->need_open = true;
    msg_box_ctx->is_open = true;
    msg_box_ctx->flags = flags;
    msg_box_ctx->msg = message;
    msg_box_ctx->callback = callback;
}
bool show_message_box() {

    auto ctx = msg_box_ctx;

    if (ctx->need_open) {
        ImGui::OpenPopup("msgbox");
        ctx->need_open = false;
    } 
    
    if (!ctx->is_open) {
        return false;
    }

    bool show = ImGui::BeginPopupModal("msgbox");

    if (show) {

        ImGui::Text(ctx->msg.str);

        bool positive = false;
        bool negative = false;

        if (ctx->flags & MESSAGE_BOX_FLAGS_OK) {
            f32 w = measure_button("Ok").x;
            align_center(w);
            negative = positive = ImGui::Button("Ok");
        } else if (ctx->flags & MESSAGE_BOX_FLAGS_YESNO) {
            positive = ImGui::Button("Yes");
            ImGui::SameLine();
            negative = ImGui::Button("No");
        } else if (ctx->flags & MESSAGE_BOX_FLAGS_OKCANCEL) {
            positive = ImGui::Button("Ok");
            ImGui::SameLine();
            negative = ImGui::Button("Cancel");
        } else {
            INTENTIONAL_CRASH("No flag specified");
        }

        ImGui::EndPopup();

        if (positive || negative) {
            ImGui::CloseCurrentPopup();
            if (ctx->callback) ctx->callback(positive);
            ctx->is_open = false;
        }
    }

    return show;
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
void end_combo() {
    make_current();

    ImGui::EndCombo();
}
bool checkbox(const char* name, bool& value) {
    make_current();

    return ImGui::Checkbox(name, &value);
}

bool begin_menu(const char* name, bool enabled) {
    make_current();
    return ImGui::BeginMenu(name, enabled);
}
void end_menu() {
    make_current();

    ImGui::EndMenu();
}


void align_right(f32 item_width) {
    make_current();
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - item_width);
}
void align_center(f32 item_width) {
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.f - item_width / 2.f);
}

void spacing() {
    make_current();
    ImGui::Spacing();   
}
void separator() {
    make_current();
    ImGui::Separator();
}

mz::fvec2 measure_string(const char* str) {
    auto imvec = ImGui::CalcTextSize(str, 0, true);

    return { imvec.x, imvec.y };
}
mz::fvec2 measure_button(const char* label) {
    auto imvec = ImGui::GetStyle().FramePadding;
    mz::fvec2 padding = {imvec.x* 2.f, imvec.y* 2.f};
    auto text_sz = measure_string(label);
    return text_sz + padding;
}

NS_END(imgui);
NS_END(stallout);