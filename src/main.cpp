#include <algorithm>

#include <raylib-cpp.hpp>

#include "wave_sim.hpp"
#include "wave_sim_renderer.hpp"

namespace rl = raylib;

struct Sizes {
    float interface_scale = 1.0f;
    int toolbar_height = static_cast<int>(100 * interface_scale);
    int window_width = static_cast<int>(800 * interface_scale);
    int window_height = window_width + toolbar_height;
    int sim_size = 1600;
};

std::optional<Vector2i> mouse_to_sim(const Sizes& sizes, const rl::Vector2 mouse_pos)
{
    if (!(mouse_pos.x >= 0 && mouse_pos.x < static_cast<float>(sizes.window_width) && mouse_pos.y >= 0
          && mouse_pos.y < static_cast<float>(sizes.window_height))) {
        return std::nullopt;
    }
    if (mouse_pos.y <= static_cast<float>(sizes.toolbar_height)) {
        return std::nullopt;
    }
    const rl::Vector2 mouse_pos_sim = mouse_pos - rl::Vector2(0.0f, static_cast<float>(sizes.toolbar_height));
    const rl::Vector2 sim_pos_f
        = mouse_pos_sim * static_cast<float>(sizes.sim_size) / static_cast<float>(sizes.window_width);
    return Vector2i { static_cast<int>(sim_pos_f.x), static_cast<int>(sim_pos_f.y) };
}

Sizes resize_interface(Sizes sizes, const float scale)
{
    sizes.interface_scale = scale;
    sizes.toolbar_height = static_cast<int>(static_cast<float>(sizes.toolbar_height) * sizes.interface_scale);
    sizes.window_width = static_cast<int>(static_cast<float>(sizes.window_width) * sizes.interface_scale);
    sizes.window_height = sizes.window_width + sizes.toolbar_height;
    return sizes;
}

int main()
{
    Sizes sizes;
    rl::Window window { sizes.window_width, sizes.window_height, "Wave Simulation" };
    if (rl::Vector2 dpi_scale = GetWindowScaleDPI(); sizes.interface_scale != dpi_scale.x) {
        sizes = resize_interface(sizes, dpi_scale.x);
    }
    window.SetSize(sizes.window_width, sizes.window_height);

    auto sim_props = WaveSim::Properties {
        .size = sizes.sim_size,
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
        const rl::Vector2 mouse_pos = GetMousePosition();
        if (const std::optional<Vector2i> sim_pos = mouse_to_sim(sizes, mouse_pos); sim_pos.has_value()) {
            if (!IsKeyDown(KEY_SPACE) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                wave_sim.add_at(sim_pos.value(), 10.0);
            }
            if (IsKeyDown(KEY_SPACE) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                constexpr int radius = 10;
                for (int x = -radius; x < radius; ++x) {
                    for (int y = -radius; y < radius; ++y) {
                        if (Vector2i pos { sim_pos->x + x, sim_pos->y + y };
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
                        if (Vector2i pos { sim_pos->x + x, sim_pos->y + y };
                            wave_sim.in_bounds(pos) && std::sqrt(x * x + y * y) <= radius && wave_sim.fixed_at(pos)) {
                            wave_sim.set_at(pos, 0.0);
                            wave_sim.set_fixed_at(pos, false);
                        }
                    }
                }
            }
        }

        wave_sim.update();
        sim_renderer.update(wave_sim);

        BeginDrawing();
        ClearBackground(WHITE);
        DrawTexturePro(
            sim_renderer.texture(),
            { 0.0f, 0.0f, static_cast<float>(sim_props.size), static_cast<float>(sim_props.size) },
            { 0.0f,
              static_cast<float>(sizes.toolbar_height),
              static_cast<float>(sizes.window_width),
              static_cast<float>(sizes.window_width) },
            { 0.0f, 0.0f },
            0.0f,
            WHITE);
        DrawFPS(10, sizes.toolbar_height + 10);
        EndDrawing();
    }

    return EXIT_SUCCESS;
}
