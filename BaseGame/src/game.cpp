#include "pch.h"

#include <Engine/base.h>



engine::renderer::Render_Context* renderer;
engine::renderer::ImGui_Renderer* imgui_renderer;

engine::audio::Audio_Context* audio_engine;
engine::audio::audio_clip_t short_clip;
engine::audio::Audio_Player* short_player = NULL;
engine::audio::audio_clip_t long_clip;
engine::audio::Audio_Player* long_player = NULL;

const char* vert_src = R"(
    layout (location = 0) in vec3 pos;
    layout (location = 1) in vec2 tex_coord_vert;

    out vec2 tex_coord_frag;

    layout(std140) uniform CameraBlock
    {
        mat4 cam_transform;
    };

    void main()
    {
        gl_Position = transpose(cam_transform) * vec4(vec3(pos.x, pos.y, 0.0), 1.0);
        tex_coord_frag = vec2(tex_coord_vert.x, 1.0 - tex_coord_vert.y);
    }
)";
const char* pixel_src = R"(
    out vec4 color_result;

    in vec2 tex_coord_frag;

    layout(std140) uniform TextureBlock
    {
        int texture1;
    };

    void main()
    {
        //color_result = vec4(0.0, texture1, 0.0, 1.0);
        color_result = texture(textures[texture1], tex_coord_frag);
    }
)";

namespace st {
    using namespace engine;
    using namespace renderer;
}

st::Resource_Handle hshader, hfubo, hvubo, hvbo, hibo, hlayout, htexture;
size_t num_indices = 0;

struct CameraBlock {
    mz::fmat4 transform;
} camera;

auto ortho = mz::projection::ortho<f32>(-1280 / 2.f, 1280 / 2.f, -720 / 2.f, 720 / 2.f, -10000000, 10000000);
auto view = mz::fmat4(1.f);

export_function(int) init() {
    renderer = stnew (engine::renderer::Render_Context)(1024 * 1000 * 100);
	imgui_renderer = stnew (engine::renderer::ImGui_Renderer)(renderer);

	renderer->wait_ready();
	auto env = renderer->get_environment();
	log_info("Game Renderer is ready!\nVendor: {}\nHardware: {}\nDrivers: {}\nVersion: {}\nShading Lang Version: {}",
	         env.vendor, env.hardware, env.driver, env.version, env.shading_version);

	audio_engine = ST_NEW(engine::audio::Audio_Context);

    auto wks_dir = os::io::get_workspace_dir();

    path_str_t short_path = "";
    path_str_t long_path = "";
    path_str_t texture_path = "";

    sprintf(short_path, "%s/%s", wks_dir.str, "test.wav");
    sprintf(long_path, "%s/%s", wks_dir.str, "forest.mp3");
    sprintf(texture_path, "%s/%s", wks_dir.str, "test.png");

	short_clip = audio_engine->create_clip_from_file(short_path);
	short_player = audio_engine->create_player();

	long_clip = audio_engine->create_clip_from_file(long_path);
	long_player = audio_engine->create_player();

    short_player->looping = true;
	long_player->looping = true;

	short_player->play(short_clip);
	long_player->play(long_clip);

    hshader = renderer->create_shader(vert_src, pixel_src);

    float quad_width = 200;
	float quad_height = 200;

	// Create & populate  ubo, vbo, ibo, layout
	float vertices[] = {
        // positions         // texture coords
         0.5f * quad_width,  0.5f * quad_height, 0.0f,  1.0f, 1.0f,
         0.5f * quad_width, -0.5f * quad_height, 0.0f,  1.0f, 0.0f,
        -0.5f * quad_width, -0.5f * quad_height, 0.0f,  0.0f, 0.0f,
        -0.5f * quad_width,  0.5f * quad_height, 0.0f,  0.0f, 1.0f,

        1.f * quad_width,  0.5f * quad_height, 0.0f,  1.0f, 0.0f,
        1.f * quad_width, -0.5f * quad_height, 0.0f,  1.0f, 1.0f,
        0.f * quad_width, -0.5f * quad_height, 0.0f,  0.0f, 1.0f,
        0.f * quad_width,  0.5f * quad_height, 0.0f,  0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 0, 0, 1, 3,
              1, 2, 3,

              4, 5, 7,
              5, 6, 7
    };
    num_indices = sizeof(indices) / sizeof(unsigned int);
	
	hfubo 	 = renderer->create_buffer(engine::renderer::BUFFER_TYPE_UNIFORM_BUFFER, engine::renderer::BUFFER_USAGE_DYNAMIC_DRAW);
	hvubo 	 = renderer->create_buffer(engine::renderer::BUFFER_TYPE_UNIFORM_BUFFER, engine::renderer::BUFFER_USAGE_DYNAMIC_DRAW);
	hvbo 	 = renderer->create_buffer(engine::renderer::BUFFER_TYPE_ARRAY_BUFFER, engine::renderer::BUFFER_USAGE_STATIC_DRAW);
	hibo    = renderer->create_buffer(engine::renderer::BUFFER_TYPE_ELEMENT_ARRAY_BUFFER, engine::renderer::BUFFER_USAGE_STATIC_DRAW);
	hlayout = renderer->create_buffer_layout({
		{ 3, engine::renderer::DATA_TYPE_FLOAT }, // Position, 3 floats
		{ 2, engine::renderer::DATA_TYPE_FLOAT }, // Tex coord, 2 floats
	});

    renderer->swap_command_buffers();

	renderer->set_buffer(hfubo, nullptr, sizeof(int));
	
	renderer->set_buffer(hvbo, &vertices);
	renderer->set_buffer(hibo, &indices);

	renderer->set_uniform_block(hshader, hfubo, "TextureBlock", 0);
	renderer->set_uniform_block(hshader, hvubo, "CameraBlock", 1);

	int width, height, channels;
	void* texture_data = engine::utils::load_image_from_file(texture_path, &width, &height, &channels, 4);

	if (!texture_data) {
		log_error("Failed loading texture");
		return 1;
	}

	htexture = renderer->create_texture(width, height, channels, engine::renderer::TEXTURE2D_FORMAT_RGBA);
	renderer->set_texture2d(htexture, texture_data, width * height * 4);

    

	engine::utils::free_image(texture_data);


    return 0;
}

void render_thing(engine::renderer::Render_Window* wnd) {
    renderer->set_viewport({0, 0}, wnd->get_size());
    renderer->set_clear_color(mz::COLOR_ORANGE);
    renderer->clear(engine::renderer::CLEAR_FLAG_COLOR);
    renderer->set_blending(
        engine::renderer::BLEND_EQUATION_ADD,
        engine::renderer::BLEND_FACTOR_SRC_ALPHA,
        engine::renderer::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
    );

    auto wnd_sz = wnd->get_size();
    ortho = mz::projection::ortho<f32>(-wnd_sz.x / 2.f, wnd_sz.x / 2.f, -wnd_sz.y / 2.f, wnd_sz.y / 2.f, -1.0f, 1.0f);

    renderer->set_uniform_block(hshader, hfubo, "TextureBlock", 0);
    renderer->set_uniform_block(hshader, hvubo, "CameraBlock", 1);

    mz::fmat4 view_inverted = view;
    view_inverted.invert();
    camera.transform = ortho * view_inverted;
    renderer->set_buffer(hvubo, &camera);
    renderer->set_texture_slot(0, htexture);

    int texture_sampler = 0; // We will bind the texture to slot 0
    renderer->set_buffer(hfubo, &texture_sampler);

    renderer->draw_indexed(
        hlayout, hvbo, hibo, hshader, 
        num_indices * 2, engine::renderer::DATA_TYPE_UINT,
    2);
}

export_function(int) update(engine::Duration frame_time) {
    float delta_time = (f32)frame_time.get_seconds();
    if (short_player->get_timer().record().get_seconds() >= 1.0) {
        short_player->stop();
    }

    float cam_speed = 1000.f;
    if (os::is_input_down(os::INPUT_CODE_W)) {
        view.translate({0, cam_speed * delta_time, 0});
    } 
    if (os::is_input_down(os::INPUT_CODE_S)) {
        view.translate({0, cam_speed * -delta_time, 0});
    } 
    if (os::is_input_down(os::INPUT_CODE_A)) {
        view.translate({cam_speed * -delta_time, 0, 0});
    } 
    if (os::is_input_down(os::INPUT_CODE_D)) {
        view.translate({cam_speed * delta_time, 0, 0});
    }

    if (renderer->get_window()->is_input_down(os::INPUT_CODE_F)) {
        log_info("FPS: {:0.0f}\nFrametime: {:0.4f}ms", 1 / frame_time.get_seconds(), frame_time.get_milliseconds());
    }

    ImGui::SetCurrentContext((ImGuiContext*)imgui_renderer->_imgui_context);

    imgui_renderer->new_frame(delta_time);


    ImGui::Begin("Test");

    ImGui::SliderFloat("pitch", &long_player->pitch, 0.1f, 3.f);
    ImGui::SliderFloat("gain", &long_player->gain, 0.0f, 2.0f);
    ImGui::SliderFloat("min_gain", &long_player->min_gain, 0.0f, 2.0f);
    ImGui::SliderFloat("max_gain", &long_player->max_gain, 0.0f, 2.0f);
    ImGui::SliderFloat("max_distance", &long_player->max_distance, 1.0f, 100.0f);
    ImGui::SliderFloat("rolloff_factor", &long_player->rolloff_factor, 0.0f, 10.0f);
    ImGui::SliderFloat("cone_outer_gain", &long_player->cone_outer_gain, 0.0f, 1.0f);
    ImGui::SliderFloat("cone_inner_angle", &long_player->cone_inner_angle, 0.0f, 180.0f);
    ImGui::SliderFloat("cone_outer_angle", &long_player->cone_outer_angle, 0.0f, 180.0f);
    ImGui::SliderFloat("reference_distance", &long_player->reference_distance, 1.0f, 50.0f);

    ImGui::Checkbox("looping", &long_player->looping);

    ImGui::DragFloat4("position", long_player->position);
    ImGui::DragFloat3("velocity", long_player->velocity);
    ImGui::DragFloat3("direction", long_player->direction);

    if (long_player->get_state() == engine::audio::PLAYER_STATE_PLAYING) {
        if (ImGui::Button("Pause")) {
            long_player->pause();
        }
    }
    if (long_player->get_state() != engine::audio::PLAYER_STATE_PLAYING) {
        if (ImGui::Button("Play")) {
            long_player->play();
        }
    }

    long_player->apply();

    ImGui::End();

    render_thing(renderer->get_window());


    imgui_renderer->render();

    renderer->get_window()->poll_events();
    renderer->get_window()->swap_buffers();
    renderer->swap_command_buffers();

    if (renderer->get_window()->exit_flag()) {
		return 1; // Exit signal
    }
    return 0; // Keep running
}

export_function(int) deinit() {
    renderer->destroy(hshader);
	renderer->destroy(hfubo);
	renderer->destroy(hvbo);
	renderer->destroy(hibo);
	renderer->destroy(hlayout);
	renderer->destroy(htexture);
    ST_DELETE(renderer);
    log_info("Game Renderer shutdown");
    return 0;
}