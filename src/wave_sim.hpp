#pragma once

#include <array>

#include "common.hpp"

class WaveSim {
public:
    WaveSim(
        const int size, const float wave_speed, const float grid_spacing, const float timestep, const float loss = 1.0f)
        : c_size(size)
        , c_wave_speed(wave_speed)
        , c_grid_spacing(grid_spacing)
        , c_timestep(timestep)
        , c_loss(loss)
        , m_buffer_past(c_size * c_size, 0.0f)
        , m_buffer_present(c_size * c_size, 0.0f)
        , m_buffer_future(c_size * c_size, 0.0f)
    {
    }

    void set_at(const Vector2i pos, const float value)
    {
        m_buffer_present[pos_to_idx(pos)] = value;
    }

    [[nodiscard]] float value_at(const Vector2i pos) const
    {
        return m_buffer_present[pos_to_idx(pos)];
    }
    void update()
    {
        for (int i = 0; i < c_size * c_size; ++i) {
            m_buffer_future[i] = future_at(idx_to_pos(i));
            m_buffer_future[i] *= c_loss;
        }
        std::swap(m_buffer_past, m_buffer_present);
        std::swap(m_buffer_present, m_buffer_future);
    }

    [[nodiscard]] size_t pos_to_idx(const Vector2i pos) const
    {
        return pos.y * c_size + pos.x;
    }

    [[nodiscard]] Vector2i idx_to_pos(const size_t idx) const
    {
        const int y = static_cast<int>(idx / c_size);
        const int x = static_cast<int>(idx % c_size);
        return { x, y };
    }

private:
    [[nodiscard]] bool in_bounds(const Vector2i pos) const
    {
        return pos.x >= 0 && pos.x < c_size && pos.y >= 0 && pos.y < c_size;
    }

    [[nodiscard]] float spatial_derivative_at(const Vector2i pos) const
    {
        constexpr std::array<Vector2i, 4> neighbors { { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } } };
        float neighbor_sum = 0.0f;
        for (const auto& [x, y] : neighbors) {
            if (const Vector2i neighbor { pos.x + x, pos.y + y }; in_bounds(neighbor)) {
                neighbor_sum += m_buffer_present[pos_to_idx(neighbor)];
            }
        }
        const float numerator = neighbor_sum - 4.0f * m_buffer_present[pos_to_idx(pos)];
        const float denominator = c_grid_spacing * c_grid_spacing;
        return numerator / denominator;
    }

    [[nodiscard]] float future_at(const Vector2i pos) const
    {
        return c_wave_speed * c_wave_speed * spatial_derivative_at(pos) * c_timestep * c_timestep
            - m_buffer_past[pos_to_idx(pos)] + 2.0f * m_buffer_present[pos_to_idx(pos)];
    }

    const int c_size;
    const float c_wave_speed;
    const float c_grid_spacing;
    const float c_timestep;
    const float c_loss;
    std::vector<float> m_buffer_past;
    std::vector<float> m_buffer_present;
    std::vector<float> m_buffer_future;
};