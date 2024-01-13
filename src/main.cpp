#include <iostream>

#include <raylib-cpp.hpp>

namespace rl = raylib;

constexpr int c_sim_size = 128;
constexpr int c_window_size = 800;

struct Vector2i {
    int x;
    int y;
};

static size_t pos_to_idx(const Vector2i pos)
{
    return pos.y * c_sim_size + pos.x;
}

static Vector2i idx_to_pos(const size_t idx)
{
    const int y = static_cast<int>(idx / c_sim_size);
    const int x = static_cast<int>(idx % c_sim_size);
    return { x, y };
}

static bool in_bounds(const Vector2i pos)
{
    return pos.x >= 0 && pos.x < c_sim_size && pos.y >= 0 && pos.y < c_sim_size;
}

int main()
{
    const rl::Window window { c_window_size, c_window_size, "Wave Simulation" };

    std::vector<float> sim;
    sim.resize(c_sim_size * c_sim_size, 0.0f);

    std::vector<float> temp_sim;
    temp_sim.resize(c_sim_size * c_sim_size, 0.0f);

    rl::Image image { c_sim_size, c_sim_size, BLACK };
    rl::Texture texture { image };

    SetTargetFPS(60.0f);

    while (!window.ShouldClose()) {

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (rl::Vector2 mouse_pos = GetMousePosition();
                mouse_pos.x >= 0 && mouse_pos.x < c_window_size && mouse_pos.y >= 0 && mouse_pos.y < c_window_size) {
                mouse_pos *= c_sim_size;
                mouse_pos /= c_window_size;
                const size_t idx = pos_to_idx({ static_cast<int>(mouse_pos.x), static_cast<int>(mouse_pos.y) });
                sim[idx] = 1.0f;
            }
        }

        std::ranges::copy(sim, temp_sim.begin());
        float sim_min = std::numeric_limits<float>::max();
        float sim_max = std::numeric_limits<float>::min();
        for (int i = 0; i < c_sim_size * c_sim_size; ++i) {
            constexpr float sim_timestep = 1.0f;
            constexpr float sim_grid_spacing = 1.0f;
            constexpr float sim_wave_speed = 0.8f;
            // Courant-Fridrichs-Lewy (CFL) condition for 2D
            static_assert(sim_wave_speed * sim_timestep / sim_grid_spacing <= 1 / 1.141f);
            const auto [x, y] = idx_to_pos(i);
            float neighbor_sum = 0.0f;
            for (constexpr std::array<Vector2i, 4> offsets { { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } } };
                 const auto [offset_x, offset_y] : offsets) {
                if (const Vector2i neighbor { x + offset_x, y + offset_y }; in_bounds(neighbor)) {
                    neighbor_sum += sim[pos_to_idx(neighbor)];
                }
            }
            const float laplacian = (neighbor_sum - 4 * sim[i]) / (sim_grid_spacing * sim_grid_spacing);
            temp_sim[i] += sim_wave_speed * sim_wave_speed * sim_timestep * sim_timestep * laplacian;
            if (temp_sim[i] < sim_min) {
                sim_min = temp_sim[i];
            }
            if (temp_sim[i] > sim_max) {
                sim_max = temp_sim[i];
            }
        }
        std::cout << sim_min << ", " << sim_max << std::endl;
        std::swap(sim, temp_sim);

        for (int i = 0; i < c_sim_size * c_sim_size; ++i) {
            const auto [x, y] = idx_to_pos(i);
            image.DrawPixel(
                x,
                y,
                { static_cast<unsigned char>((sim[i] - sim_min) / std::abs(sim_max - sim_min) * 255),
                  static_cast<unsigned char>((sim[i] - sim_min) / std::abs(sim_max - sim_min) * 255),
                  static_cast<unsigned char>((sim[i] - sim_min) / std::abs(sim_max - sim_min) * 255),
                  255 });
        }
        texture.Update(image.GetData());

        BeginDrawing();

        DrawTexturePro(
            texture,
            { 0.0f, 0.0f, c_sim_size, c_sim_size },
            { 0.0f, 0.0f, c_window_size, c_window_size },
            { 0.0f, 0.0f },
            0.0f,
            WHITE);

        DrawFPS(10.0f, 10.0f);
        EndDrawing();
    }

    return EXIT_SUCCESS;
}
