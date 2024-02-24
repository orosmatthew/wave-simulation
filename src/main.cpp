#include <raylib-cpp.hpp>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "res/roboto-regular.h"
#include "wave_sim.hpp"
#include "wave_sim_renderer.hpp"

namespace rl = raylib;

constexpr int sim_size = 1600;

static int toolbar_height()
{
    constexpr float toolbar_ratio = 0.1f;
    return std::max(static_cast<int>(static_cast<float>(GetScreenHeight()) * toolbar_ratio), 100);
}

static rl::Rectangle sim_screen_rect()
{
    const int size = std::max(std::min(GetScreenWidth(), GetScreenHeight() - toolbar_height()), 1);
    return { static_cast<float>(GetScreenWidth()) / 2.0f - static_cast<float>(size) / 2.0f,
             static_cast<float>(toolbar_height()),
             static_cast<float>(size),
             static_cast<float>(size) };
}

static std::optional<Vector2i> mouse_to_sim(const rl::Vector2 mouse_pos)
{
    const rl::Vector2 sim_pos_f
        = (mouse_pos - sim_screen_rect().GetPosition()) * sim_size / sim_screen_rect().GetWidth();
    return Vector2i { static_cast<int>(sim_pos_f.x), static_cast<int>(sim_pos_f.y) };
}

void handle_font_scale_inputs(rl::Font& font)
{
    std::optional<int> new_size;
    if (IsKeyPressed(KEY_EQUAL) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT)) {
        new_size = font.GetBaseSize() + 1;
    }
    if (IsKeyPressed(KEY_MINUS) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT)) {
        new_size = font.GetBaseSize() - 1;
    }
    if (new_size.has_value()) {
        font = LoadFontFromMemory(
            ".ttf",
            font_robot_regular_ttf_bin,
            static_cast<int>(font_robot_regular_ttf_bin_size),
            new_size.value(),
            nullptr,
            0);
        GuiSetFont(font);
        GuiSetStyle(DEFAULT, TEXT_SIZE, new_size.value());
    }
}

static void handle_sim_inputs(WaveSim& wave_sim)
{
    const rl::Vector2 mouse_pos = GetMousePosition();
    if (const std::optional<Vector2i> sim_pos = mouse_to_sim(mouse_pos);
        sim_pos.has_value() && wave_sim.in_bounds(sim_pos.value())) {
        if (!IsKeyDown(KEY_SPACE) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            wave_sim.add_at(sim_pos.value(), 10.0);
        }
        if (IsKeyDown(KEY_SPACE) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
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
        if (IsKeyDown(KEY_SPACE) && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
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
}

static void draw(const WaveSimRenderer& sim_renderer)
{
    BeginDrawing();
    ClearBackground(LIGHTGRAY);
    DrawTexturePro(
        sim_renderer.texture(), { 0.0f, 0.0f, sim_size, sim_size }, sim_screen_rect(), { 0.0f, 0.0f }, 0.0f, WHITE);
    DrawFPS(10, toolbar_height() + 10);
    GuiButton({ 10.0f, 10.0f, 100.0f, 40.0f }, "Hello World!");
    EndDrawing();
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    rl::Window window { 600, 800, "Wave Simulation" };
    rl::Font roboto_font = LoadFontFromMemory(
        ".ttf", font_robot_regular_ttf_bin, static_cast<int>(font_robot_regular_ttf_bin_size), 18, nullptr, 0);
    GuiSetFont(roboto_font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);

    auto sim_props = WaveSim::Properties {
        .size = sim_size,
        .wave_speed = 0.5,
        .grid_spacing = 1.0,
        .timestep = 1.0,
        .loss = 0.9995,
        .damping_strength = 0.08,
        .damping_width = 100
    };
    WaveSim wave_sim(sim_props);

    WaveSimRenderer sim_renderer(sim_props.size);

    // SetTargetFPS(60.0f);

    while (!window.ShouldClose()) {
        handle_font_scale_inputs(roboto_font);

        handle_sim_inputs(wave_sim);
        wave_sim.update();
        sim_renderer.update(wave_sim);

        draw(sim_renderer);
    }

    return EXIT_SUCCESS;
}
