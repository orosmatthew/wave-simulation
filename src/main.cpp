#include <algorithm>

#include <BS_thread_pool.hpp>
#include <raylib-cpp.hpp>

#include "wave_sim.hpp"

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

    rl::Image image { sim_props.size, sim_props.size, BLACK };
    rl::Texture texture { image };

    // SetTargetFPS(60.0f);

    BS::thread_pool thread_pool;

    while (!window.ShouldClose()) {

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (rl::Vector2 mouse_pos = GetMousePosition();
                mouse_pos.x >= 0 && mouse_pos.x < window_size && mouse_pos.y >= 0 && mouse_pos.y < window_size) {
                mouse_pos *= sim_props.size;
                mouse_pos /= window_size;
                wave_sim.set_at({ static_cast<int>(mouse_pos.x), static_cast<int>(mouse_pos.y) }, 10.0);
            }
        }

        wave_sim.update();

        thread_pool.detach_blocks<int>(0, sim_props.size * sim_props.size, [&](const int start, const int end) {
            for (int i = start; i < end; ++i) {
                const double sim_value = wave_sim.value_at_idx(i);
                const auto [x, y] = wave_sim.idx_to_pos(i);
                image.DrawPixel(
                    x,
                    y,
                    { static_cast<unsigned char>(std::clamp((sim_value + 0.5) / (0.5 * 2), 0.0, 1.0) * 255),
                      static_cast<unsigned char>(std::clamp((sim_value + 0.5) / (0.5 * 2), 0.0, 1.0) * 255),
                      static_cast<unsigned char>(std::clamp((sim_value + 0.5) / (0.5 * 2), 0.0, 1.0) * 255),
                      255 });
            }
        });
        thread_pool.wait();

        texture.Update(image.GetData());

        BeginDrawing();

        DrawTexturePro(
            texture,
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
