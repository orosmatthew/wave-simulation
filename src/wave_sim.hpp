#pragma once

#include <array>

#include <BS_thread_pool.hpp>

#include "common.hpp"

class WaveSim {
public:
    struct Properties {
        int size = 512;
        double wave_speed = 0.5;
        double grid_spacing = 1.0;
        double timestep = 1.0;
        double loss = 0.999;
        double damping_strength = 0.2;
        double damping_width = 50;
    };

    explicit WaveSim(const Properties& props)
        : c_size(props.size)
        , c_wave_speed(props.wave_speed)
        , c_grid_spacing(props.grid_spacing)
        , c_timestep(props.timestep)
        , c_loss(props.loss)
        , c_damping_strength(props.damping_strength)
        , c_damping_width(props.damping_width)
        , m_buffer_past(c_size * c_size, 0.0)
        , m_buffer_present(c_size * c_size, 0.0)
        , m_buffer_future(c_size * c_size, 0.0)
        , m_buffed_fixed(c_size * c_size, false)
    {
    }

    void set_at(const Vector2i pos, const double value)
    {
        m_buffer_present[pos_to_idx(pos)] = value;
    }

    void set_fixed_at(const Vector2i pos, const bool fixed)
    {
        m_buffed_fixed[pos_to_idx(pos)] = fixed;
    }

    bool fixed_at(const Vector2i pos) const
    {
        return fixed_at_idx(pos_to_idx(pos));
    }

    bool fixed_at_idx(const size_t idx) const
    {
        return m_buffed_fixed[idx];
    }

    void add_at(const Vector2i pos, const double value)
    {
        m_buffer_present[pos_to_idx(pos)] += value;
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
                if (!m_buffed_fixed[i]) {
                    m_buffer_future[i] = future_at_idx(i);
                    m_buffer_future[i] *= c_loss;
                }
                else {
                    m_buffer_future[i] = m_buffer_present[i];
                }
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

    [[nodiscard]] bool in_bounds(const Vector2i pos) const
    {
        return pos.x >= 0 && pos.x < c_size && pos.y >= 0 && pos.y < c_size;
    }

private:
    static Vector2i opposite_neighbor(const Vector2i n)
    {
        Vector2i opp { 0, 0 };
        if (n.x != 0) {
            opp.x = -n.x;
        }
        if (n.y != 0) {
            opp.y = -n.y;
        }
        return opp;
    }

    double damping_at_idx(const size_t idx) const
    {
        const auto [x, y] = idx_to_pos(idx);
        double damping = 0.0;
        if (x < c_damping_width) { // left
            damping = c_damping_strength * (c_damping_width - x) / c_damping_width;
        }
        if (y < c_damping_width) { // top
            damping = c_damping_strength * (c_damping_width - y) / c_damping_width;
        }
        if (x >= c_size - c_damping_width) { // right
            damping = c_damping_strength * (x - (c_size - c_damping_width)) / c_damping_width;
        }
        if (y >= c_size - c_damping_width) { // bottom
            damping = c_damping_strength * (y - (c_size - c_damping_width)) / c_damping_width;
        }
        return damping;
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
        double future = c_wave_speed * c_wave_speed * spatial_derivative_at_idx(idx) * c_timestep * c_timestep
            - m_buffer_past[idx] + 2.0 * m_buffer_present[idx];
        future -= 2.0 * damping_at_idx(idx) * (m_buffer_present[idx] - m_buffer_past[idx]);
        return future;
    }

    const int c_size;
    const double c_wave_speed;
    const double c_grid_spacing;
    const double c_timestep;
    const double c_loss;
    const double c_damping_strength;
    const double c_damping_width;
    std::vector<double> m_buffer_past;
    std::vector<double> m_buffer_present;
    std::vector<double> m_buffer_future;
    std::vector<bool> m_buffed_fixed;
    BS::thread_pool m_thread_pool;
};