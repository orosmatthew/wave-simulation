#include <algorithm>

#include <raylib-cpp.hpp>

#include "wave_sim.hpp"

namespace rl = raylib;

int main()
{
    constexpr int window_size = 1200;
    const rl::Window window { window_size, window_size, "Wave Simulation" };

    constexpr int sim_size = 512;
    constexpr float sim_wave_speed = 0.5f;
    constexpr float sim_grid_spacing = 1.0f;
    constexpr float sim_timestep = 1.0f;
    constexpr float sim_loss = 0.998f;
    WaveSim wave_sim(sim_size, sim_wave_speed, sim_grid_spacing, sim_timestep, sim_loss);

    rl::Image image { sim_size, sim_size, BLACK };
    rl::Texture texture { image };

    SetTargetFPS(60.0f);

    while (!window.ShouldClose()) {

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (rl::Vector2 mouse_pos = GetMousePosition();
                mouse_pos.x >= 0 && mouse_pos.x < window_size && mouse_pos.y >= 0 && mouse_pos.y < window_size) {
                mouse_pos *= sim_size;
                mouse_pos /= window_size;
                wave_sim.set_at({ static_cast<int>(mouse_pos.x), static_cast<int>(mouse_pos.y) }, 1.0f);
            }
        }

        wave_sim.update();

        for (int x = 0; x < sim_size; ++x) {
            for (int y = 0; y < sim_size; ++y) {
                const float sim_value = wave_sim.value_at({ x, y });
                image.DrawPixel(
                    x,
                    y,
                    { static_cast<unsigned char>(std::clamp((sim_value + 0.5f) / (0.5f * 2), 0.0f, 1.0f) * 255),
                      static_cast<unsigned char>(std::clamp((sim_value + 0.5f) / (0.5f * 2), 0.0f, 1.0f) * 255),
                      static_cast<unsigned char>(std::clamp((sim_value + 0.5f) / (0.5f * 2), 0.0f, 1.0f) * 255),
                      255 });
            }
        }

        texture.Update(image.GetData());

        BeginDrawing();

        DrawTexturePro(
            texture,
            { 0.0f, 0.0f, sim_size, sim_size },
            { 0.0f, 0.0f, window_size, window_size },
            { 0.0f, 0.0f },
            0.0f,
            WHITE);

        DrawFPS(10.0f, 10.0f);
        EndDrawing();
    }

    return EXIT_SUCCESS;
}
