#include <algorithm>
#include <array>

#include <raylib-cpp.hpp>

namespace rl = raylib;

constexpr int c_sim_size = 512;
constexpr int c_window_size = 1200;

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

static float sim_spatial_derivative_at(
    const Vector2i& pos, const std::vector<float>& sim_present, const float grid_spacing)
{
    constexpr std::array<Vector2i, 4> neighbors { { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } } };
    float neighbor_sum = 0.0f;
    for (const auto& [x, y] : neighbors) {
        if (const Vector2i neighbor { pos.x + x, pos.y + y }; in_bounds(neighbor)) {
            neighbor_sum += sim_present[pos_to_idx(neighbor)];
        }
    }
    const float numerator = neighbor_sum - 4.0f * sim_present[pos_to_idx(pos)];
    const float denominator = grid_spacing * grid_spacing;
    return numerator / denominator;
}

static float sim_future_at(
    const Vector2i& pos,
    const float wave_speed,
    const float grid_spacing,
    const float timestep,
    const std::vector<float>& sim_present,
    const std::vector<float>& sim_past)
{
    return wave_speed * wave_speed * sim_spatial_derivative_at(pos, sim_present, grid_spacing) * timestep * timestep
        - sim_past[pos_to_idx(pos)] + 2.0f * sim_present[pos_to_idx(pos)];
}

int main()
{
    const rl::Window window { c_window_size, c_window_size, "Wave Simulation" };

    std::vector<float> sim_present;
    sim_present.resize(c_sim_size * c_sim_size, 0.0f);

    std::vector<float> sim_future;
    sim_future.resize(c_sim_size * c_sim_size, 0.0f);

    std::vector<float> sim_past;
    sim_past.resize(c_sim_size * c_sim_size, 0.0f);

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
                sim_present[idx] = 1.0f;
            }
        }

        float sim_min = std::numeric_limits<float>::max();
        float sim_max = std::numeric_limits<float>::min();
        for (int i = 0; i < c_sim_size * c_sim_size; ++i) {
            constexpr float sim_wave_speed = 0.5f;
            constexpr float sim_grid_spacing = 1.0f;
            constexpr float sim_timestep = 1.0f;
            sim_future[i]
                = sim_future_at(idx_to_pos(i), sim_wave_speed, sim_grid_spacing, sim_timestep, sim_present, sim_past);
            sim_future[i] *= 0.998f;
            if (sim_future[i] < sim_min) {
                sim_min = sim_future[i];
            }
            if (sim_future[i] > sim_max) {
                sim_max = sim_future[i];
            }
        }
        sim_past = sim_present;
        sim_present = sim_future;

        for (int i = 0; i < c_sim_size * c_sim_size; ++i) {
            const auto [x, y] = idx_to_pos(i);
            image.DrawPixel(
                x,
                y,
                { static_cast<unsigned char>(std::clamp((sim_present[i] + 0.5f) / (0.5f * 2), 0.0f, 1.0f) * 255),
                  static_cast<unsigned char>(std::clamp((sim_present[i] + 0.5f) / (0.5f * 2), 0.0f, 1.0f) * 255),
                  static_cast<unsigned char>(std::clamp((sim_present[i] + 0.5f) / (0.5f * 2), 0.0f, 1.0f) * 255),
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
