#pragma once

#include <algorithm>

class Sizes {
public:
    Sizes(const float scale, const int sim_size)
        : m_scale(std::clamp(scale, sc_min_scale, sc_max_scale))
        , m_toolbar_height(static_cast<int>(100 * m_scale))
        , m_window_width(static_cast<int>(800 * m_scale))
        , m_window_height(m_window_width + m_toolbar_height)
        , m_sim_size(sim_size)
    {
    }

    bool rescale(const float scale)
    {
        if (scale < sc_min_scale || scale > sc_max_scale) {
            return false;
        }
        const auto s = Sizes(scale, m_sim_size);
        *this = s;
        return true;
    }

    [[nodiscard]] int toolbar_height() const
    {
        return m_toolbar_height;
    }

    [[nodiscard]] int window_width() const
    {
        return m_window_width;
    }

    [[nodiscard]] int window_height() const
    {
        return m_window_height;
    }

    [[nodiscard]] int sim_size() const
    {
        return m_sim_size;
    }

    [[nodiscard]] float scale() const
    {
        return m_scale;
    }

private:
    static constexpr float sc_min_scale = 0.5f;
    static constexpr float sc_max_scale = 2.0f;
    float m_scale;
    int m_toolbar_height;
    int m_window_width;
    int m_window_height;
    int m_sim_size;
};