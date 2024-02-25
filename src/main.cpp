#include <optional>

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#include <raylib-cpp.hpp>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "res/roboto-regular.h"
#include "ui.hpp"
#include "wave_sim.hpp"
#include "wave_sim_renderer.hpp"

namespace rl = raylib;

constexpr int sim_size = 1024;
constexpr int base_font_size = 16;

static rl::Rectangle sim_screen_rect(const int toolbar_height)
{
    const int size = std::max(std::min(GetScreenWidth(), GetScreenHeight() - toolbar_height), 1);
    return { static_cast<float>(GetScreenWidth()) / 2.0f - static_cast<float>(size) / 2.0f,
             static_cast<float>(toolbar_height),
             static_cast<float>(size),
             static_cast<float>(size) };
}

static std::optional<Vector2i> mouse_to_sim(const rl::Vector2 mouse_pos, const int toolbar_height)
{
    const rl::Vector2 sim_pos_f = (mouse_pos - sim_screen_rect(toolbar_height).GetPosition()) * sim_size
        / sim_screen_rect(toolbar_height).GetWidth();
    return Vector2i { static_cast<int>(sim_pos_f.x), static_cast<int>(sim_pos_f.y) };
}

static void resize_font(rl::Font& font, const int size)
{
    font = LoadFontFromMemory(
        ".ttf", font_robot_regular_ttf_bin, static_cast<int>(font_robot_regular_ttf_bin_size), size, nullptr, 0);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, size);
}

static void handle_font_scale_inputs(rl::Font& font)
{
    std::optional<int> new_size;
    if (IsKeyPressed(KEY_EQUAL) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT)) {
        new_size = font.GetBaseSize() + 1;
    }
    if (IsKeyPressed(KEY_MINUS) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT)) {
        new_size = font.GetBaseSize() - 1;
    }
    if (new_size.has_value()) {
        resize_font(font, new_size.value());
    }
}

enum class Mode { none, interact, walls };

static void handle_sim_inputs(const Mode mode, WaveSim& wave_sim, const int toolbar_height)
{
    const rl::Vector2 mouse_pos = GetMousePosition();
    if (const std::optional<Vector2i> sim_pos = mouse_to_sim(mouse_pos, toolbar_height);
        sim_pos.has_value() && wave_sim.in_bounds(sim_pos.value())) {
        if (mode == Mode::walls) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                constexpr int radius = 10;
                for (int x = -radius; x < radius; ++x) {
                    for (int y = -radius; y < radius; ++y) {
                        if (const Vector2i pos { sim_pos->x + x, sim_pos->y + y };
                            wave_sim.in_bounds(pos) && std::sqrt(x * x + y * y) <= radius) {
                            wave_sim.set_at(pos, 0.0);
                            wave_sim.set_fixed_at(pos, true);
                        }
                    }
                }
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                constexpr int radius = 20;
                for (int x = -radius; x < radius; ++x) {
                    for (int y = -radius; y < radius; ++y) {
                        if (const Vector2i pos { sim_pos->x + x, sim_pos->y + y };
                            wave_sim.in_bounds(pos) && std::sqrt(x * x + y * y) <= radius && wave_sim.fixed_at(pos)) {
                            wave_sim.set_at(pos, 0.0);
                            wave_sim.set_fixed_at(pos, false);
                        }
                    }
                }
            }
        }
        else if (mode == Mode::interact) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                wave_sim.add_at(sim_pos.value(), 10.0);
            }
        }
    }
}

struct State {
    rl::Font font;
    float scale;
    WaveSim wave_sim;
    WaveSimRenderer sim_renderer;
    Mode mode;
    LabelledDropdown mode_dropdown;
    LabelledDropdown theme_dropdown;
    WaveSimRenderer::Theme renderer_theme;
    int show_fps;
};

void loop(void* state)
{
    auto* s = static_cast<State*>(state);

    const int toolbar_height = static_cast<int>(std::round(100.0f * s->scale));
    handle_font_scale_inputs(s->font);

    if (IsKeyPressed(KEY_C)) {
        s->wave_sim.clear();
    }

    if (IsKeyPressed(KEY_N)) {
        s->mode = Mode::none;
        s->mode_dropdown.set_active(static_cast<int>(s->mode));
    }
    else if (IsKeyPressed(KEY_I)) {
        s->mode = Mode::interact;
        s->mode_dropdown.set_active(static_cast<int>(s->mode));
    }
    else if (IsKeyPressed(KEY_W)) {
        s->mode = Mode::walls;
        s->mode_dropdown.set_active(static_cast<int>(s->mode));
    }

    handle_sim_inputs(s->mode, s->wave_sim, toolbar_height);
    s->wave_sim.update();
    s->sim_renderer.update(s->wave_sim, s->renderer_theme);

    BeginDrawing();
    ClearBackground(LIGHTGRAY);
    DrawTexturePro(
        s->sim_renderer.texture(),
        { 0.0f, 0.0f, sim_size, sim_size },
        sim_screen_rect(toolbar_height),
        { 0.0f, 0.0f },
        0.0f,
        WHITE);
    if (s->show_fps) {
        DrawFPS(10, toolbar_height + 10);
    }

#ifndef PLATFORM_WEB
    if (const float scale = GetWindowScaleDPI().x; scale != s->scale) {
        s->scale = scale;
        resize_font(s->font, static_cast<int>(std::round(static_cast<float>(base_font_size) * s->scale)));
    }
#endif
    {
        const float ui_height = 25.0f * s->scale;
        const float ui_padding = 10.0f * s->scale;

        float offset_x = ui_padding;
        if (GuiButton({ ui_padding, ui_padding, 70.0f * s->scale, ui_height }, "Clear [C]")) {
            s->wave_sim.clear();
        }
        offset_x += 70.0f * s->scale + ui_padding;
        GuiToggleSlider({ offset_x, ui_padding, 60.0f * s->scale, ui_height }, "FPS;FPS", &s->show_fps);

        offset_x = ui_padding;
        s->theme_dropdown.draw_and_update(
            { offset_x, ui_height + ui_padding * 1.5f, 110.0f * s->scale, ui_height * 2.0f });
        offset_x += 110.0f * s->scale + ui_padding;
        s->renderer_theme = static_cast<WaveSimRenderer::Theme>(s->theme_dropdown.active());
        s->mode_dropdown.draw_and_update(
            { offset_x, ui_height + ui_padding * 1.5f, 90.0f * s->scale, ui_height * 2.0f });
        s->mode = static_cast<Mode>(s->mode_dropdown.active());
    }

    EndDrawing();
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    rl::Window window { 600, 700, "Wave Simulation" };
    window.SetMinSize(600, 700);
    const int font_size = static_cast<int>(std::round(static_cast<float>(base_font_size) * GetWindowScaleDPI().x));
    rl::Font font = LoadFontFromMemory(
        ".ttf", font_robot_regular_ttf_bin, static_cast<int>(font_robot_regular_ttf_bin_size), font_size, nullptr, 0);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, font_size);

    constexpr auto sim_props = WaveSim::Properties {
        .size = sim_size,
        .wave_speed = 0.5,
        .grid_spacing = 1.0,
        .timestep = 1.0,
        .loss = 0.9995,
        .damping_strength = 0.08,
        .damping_width = 100
    };

    auto mode = Mode::interact;

    LabelledDropdown theme_dropdown("Theme");
    theme_dropdown.set_items({ "Grayscale", "Grayscale ABS" });
    LabelledDropdown mode_dropdown("Mode");
    mode_dropdown.set_items({ "None [N]", "Interact [I]", "Walls [W]" });
    mode_dropdown.set_active(static_cast<int>(mode));

    State state { .font = std::move(font),
                  .scale = 1.0f,
                  .wave_sim = WaveSim(sim_props),
                  .sim_renderer = WaveSimRenderer(sim_props.size),
                  .mode = mode,
                  .mode_dropdown = std::move(mode_dropdown),
                  .theme_dropdown = std::move(theme_dropdown),
                  .renderer_theme = WaveSimRenderer::Theme::grayscale,
                  .show_fps = 0 };

#ifdef PLATFORM_WEB
    emscripten_set_main_loop_arg(loop, &state, 0, 1);
#else
    while (!window.ShouldClose()) {
        loop(&state);
    }
#endif
    return EXIT_SUCCESS;
}
