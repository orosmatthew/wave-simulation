#pragma once

#include <array>

#include <BS_thread_pool.hpp>

#include "common.hpp"

class WaveSim {
public:
    WaveSim(
        const int size,
        const double wave_speed,
        const double grid_spacing,
        const double timestep,
        const double loss = 1.0)
        : c_size(size)
        , c_wave_speed(wave_speed)
        , c_grid_spacing(grid_spacing)
        , c_timestep(timestep)
        , c_loss(loss)
        , m_buffer_past(c_size * c_size, 0.0)
        , m_buffer_present(c_size * c_size, 0.0)
        , m_buffer_future(c_size * c_size, 0.0)
    {
    }

    void set_at(const Vector2i pos, const double value)
    {
        m_buffer_present[pos_to_idx(pos)] = value;
    }

    [[nodiscard]] double value_at(const Vector2i pos) const
    {
        return value_at_idx(pos_to_idx(pos));
    }

    [[nodiscard]] double value_at_idx(const size_t idx) const
    {
        return m_buffer_present[idx];
    }

    void update()
    {
        m_thread_pool.detach_blocks<int>(0, c_size * c_size, [&](const int start, const int end) {
            for (int i = start; i < end; ++i) {
                m_buffer_future[i] = future_at_idx(i);
                m_buffer_future[i] *= c_loss;
            }
        });
        m_thread_pool.wait();
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

    [[nodiscard]] double spatial_derivative_at_idx(const size_t idx) const
    {

        constexpr std::array<Vector2i, 4> neighbors { { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } } };
        double neighbor_sum = 0.0;
        const auto [x, y] = idx_to_pos(idx);
        for (const auto& [n_x, n_y] : neighbors) {
            if (const Vector2i neighbor { x + n_x, y + n_y }; in_bounds(neighbor)) {
                neighbor_sum += m_buffer_present[pos_to_idx(neighbor)];
            }
        }
        const double numerator = neighbor_sum - 4.0 * m_buffer_present[idx];
        const double denominator = c_grid_spacing * c_grid_spacing;
        return numerator / denominator;
    }

    [[nodiscard]] double future_at_idx(const size_t idx) const
    {
        return c_wave_speed * c_wave_speed * spatial_derivative_at_idx(idx) * c_timestep * c_timestep
            - m_buffer_past[idx] + 2.0 * m_buffer_present[idx];
    }

    const int c_size;
    const double c_wave_speed;
    const double c_grid_spacing;
    const double c_timestep;
    const double c_loss;
    std::vector<double> m_buffer_past;
    std::vector<double> m_buffer_present;
    std::vector<double> m_buffer_future;
    BS::thread_pool m_thread_pool;
};