#pragma once

#ifndef PLATFORM_WEB
#include "BS_thread_pool.hpp"
#endif
#include "raylib-cpp.hpp"

#include "common.hpp"
#include "schrodinger_sim.hpp"

#include <iostream>

class SchrodingerRenderer {
public:
    explicit SchrodingerRenderer(const int size)
        : c_size(size)
        , m_image(c_size, c_size, BLACK)
        , m_texture(m_image)
    {
    }

    void update(const SchrodingerSim& sim)
    {
        // double sum = 0;
        // for (int i = 0; i < c_size * c_size; ++i) {
        //     sum += std::pow(std::abs(sim.value_at_idx(i)), 2.0);
        // }
        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();
        for (int i = 0; i < c_size * c_size; ++i) {
            const double sim_value = std::norm(sim.value_at_idx(i));
            if (sim_value > max) {
                max = sim_value;
            }
            if (sim_value < min) {
                min = sim_value;
            }
        }
        auto update_at = [&](const int i) {
            const auto [x, y] = sim.idx_to_pos(i);
            const double sim_value = std::norm(sim.value_at_idx(i)); // / sum;
            const Color color
                = { static_cast<unsigned char>(std::clamp((sim_value - min) / (max), 0.0, 1.0) * 255),
                    static_cast<unsigned char>(std::clamp((sim_value - min) / (max), 0.0, 1.0) * 255),
                    static_cast<unsigned char>(std::clamp((sim_value - min) / (max), 0.0, 1.0) * 255),
                    255 };

            static_cast<unsigned char*>(m_image.data)[(y * m_image.width + x) * 4] = color.r;
            static_cast<unsigned char*>(m_image.data)[(y * m_image.width + x) * 4 + 1] = color.g;
            static_cast<unsigned char*>(m_image.data)[(y * m_image.width + x) * 4 + 2] = color.b;
            static_cast<unsigned char*>(m_image.data)[(y * m_image.width + x) * 4 + 3] = color.a;
        };

#ifndef PLATFORM_WEB
        m_thread_pool.detach_blocks<int>(0, c_size * c_size, [&](const int start, const int end) {
            for (int i = start; i < end; ++i) {
                update_at(i);
            }
        });
        m_thread_pool.wait();
#else
        for (int i = 0; i < c_size * c_size; ++i) {
            update_at(i);
        }
#endif
        m_texture.Update(m_image.GetData());
        // std::cout << "TOTAL: " << total << std::endl;
    }

    [[nodiscard]] const raylib::Texture& texture() const
    {
        return m_texture;
    }

private:
    const int c_size;
    raylib::Image m_image;
    raylib::Texture m_texture;
#ifndef PLATFORM_WEB
    BS::thread_pool m_thread_pool;
#endif
};