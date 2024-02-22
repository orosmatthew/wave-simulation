#include <algorithm>

#include <BS_thread_pool.hpp>
#include <raylib-cpp.hpp>

#include "wave_sim.hpp"
#include "wave_sim_renderer.hpp"

namespace rl = raylib;

int main()
{
    constexpr int window_size = 1200;
    const rl::Window window { window_size, window_size, "Wave Simulation" };

    constexpr auto sim_props = WaveSim::Properties {
        .size = 2048,
        .wave_speed = 0.5,
        .grid_spacing = 1.0,
        .timestep = 1.0,
        .loss = 0.999,
        .damping_strength = 0.1,
        .damping_width = 100
    };
    WaveSim wave_sim(sim_props);

    WaveSimRenderer sim_renderer(sim_props.size);

    // SetTargetFPS(60.0f);

    while (!window.ShouldClose()) {

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (rl::Vector2 mouse_pos = GetMousePosition();
                mouse_pos.x >= 0 && mouse_pos.x < window_size && mouse_pos.y >= 0 && mouse_pos.y < window_size) {
                mouse_pos *= sim_props.size;
                mouse_pos /= window_size;
                wave_sim.add_at({ static_cast<int>(mouse_pos.x), static_cast<int>(mouse_pos.y) }, 10.0);
            }
        }

        wave_sim.update();
        sim_renderer.update(wave_sim);

        BeginDrawing();
        DrawTexturePro(
            sim_renderer.texture(),
            { 0.0f, 0.0f, sim_props.size, sim_props.size },
            { 0.0f, 0.0f, window_size, window_size },
            { 0.0f, 0.0f },
            0.0f,
            WHITE);
        DrawFPS(10.0f, 10.0f);
        EndDrawing();
    }

    return EXIT_SUCCESS;
}
