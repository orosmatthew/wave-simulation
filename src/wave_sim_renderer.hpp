#pragma once

#include "BS_thread_pool.hpp"
#include "raylib-cpp.hpp"

#include "common.hpp"
#include "wave_sim.hpp"

class WaveSimRenderer {
public:
    explicit WaveSimRenderer(const int size)
        : c_size(size)
        , m_image(c_size, c_size, BLACK)
        , m_texture(m_image)
    {
    }

    void update(const WaveSim& sim)
    {
        m_thread_pool.detach_blocks<int>(0, c_size * c_size, [&](const int start, const int end) {
            for (int i = start; i < end; ++i) {
                const double sim_value = sim.value_at_idx(i);
                const auto [x, y] = sim.idx_to_pos(i);
                m_image.DrawPixel(
                    x,
                    y,
                    { static_cast<unsigned char>(std::clamp((sim_value + 0.5) / (0.5 * 2), 0.0, 1.0) * 255),
                      static_cast<unsigned char>(std::clamp((sim_value + 0.5) / (0.5 * 2), 0.0, 1.0) * 255),
                      static_cast<unsigned char>(std::clamp((sim_value + 0.5) / (0.5 * 2), 0.0, 1.0) * 255),
                      255 });
            }
        });
        m_thread_pool.wait();
        m_texture.Update(m_image.GetData());
    }

    const raylib::Texture& texture() const
    {
        return m_texture;
    }

private:
    const int c_size;
    raylib::Image m_image;
    raylib::Texture m_texture;
    BS::thread_pool m_thread_pool;
};