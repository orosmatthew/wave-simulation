#pragma once

#include <complex>
#include <vector>

#include "BS_thread_pool.hpp"
#include "raylib-cpp.hpp"

#include "common.hpp"
#include "schrodinger_sim.hpp"

class SchrodingerRenderer {
public:
    enum class Theme {
        probability,
        waves,
    };

    explicit SchrodingerRenderer(const int size)
        : c_size(size)
        , m_image(c_size, c_size, BLACK)
        , m_texture(m_image)
        , m_buffer(c_size * c_size, 0.0)
    {
    }

    void update(SchrodingerSim& sim, const Theme theme)
    {
        double prob_min = std::numeric_limits<double>::max();
        double prob_max = std::numeric_limits<double>::min();
        double wave_min = std::numeric_limits<double>::max();
        double wave_max = std::numeric_limits<double>::min();
        sim.lock_read();
        for (int i = 0; i < c_size * c_size; ++i) {
            const auto sim_value = sim.value_at_idx(i);
            m_buffer[i] = sim_value;
            const auto abs = std::norm(sim_value);
            if (std::min(sim_value.real(), sim_value.imag()) < wave_min) {
                wave_min = std::max(sim_value.real(), sim_value.imag());
            }
            if (std::max(sim_value.real(), sim_value.imag()) > wave_max) {
                wave_max = std::max(sim_value.real(), sim_value.imag());
            }
            if (abs > prob_max) {
                prob_max = abs;
            }
            if (abs < prob_min) {
                prob_min = abs;
            }
        }
        sim.unlock_read();
        auto update_at = [&](const int i) {
            const auto [x, y] = sim.idx_to_pos(i);
            auto color = BLACK;
            if (theme == Theme::probability) {
                const double sim_value = std::norm(m_buffer[i]);
                color = { static_cast<unsigned char>(std::clamp((sim_value - prob_min) / prob_max, 0.0, 1.0) * 255),
                          static_cast<unsigned char>(std::clamp((sim_value - prob_min) / prob_max, 0.0, 1.0) * 255),
                          static_cast<unsigned char>(std::clamp((sim_value - prob_min) / prob_max, 0.0, 1.0) * 255),
                          255 };
            }
            else if (theme == Theme::waves) {
                const std::complex<double> sim_value = m_buffer[i];
                color = {
                    static_cast<unsigned char>(std::clamp((sim_value.real() - wave_min) / wave_max, 0.0, 1.0) * 255),
                    static_cast<unsigned char>(std::clamp((sim_value.imag() - wave_min) / wave_max, 0.0, 1.0) * 255),
                    0,
                    255
                };
            }
            static_cast<unsigned char*>(m_image.data)[(y * m_image.width + x) * 4] = color.r;
            static_cast<unsigned char*>(m_image.data)[(y * m_image.width + x) * 4 + 1] = color.g;
            static_cast<unsigned char*>(m_image.data)[(y * m_image.width + x) * 4 + 2] = color.b;
            static_cast<unsigned char*>(m_image.data)[(y * m_image.width + x) * 4 + 3] = color.a;
        };

        m_thread_pool.detach_blocks<int>(0, c_size * c_size, [&](const int start, const int end) {
            for (int i = start; i < end; ++i) {
                update_at(i);
            }
        });
        m_thread_pool.wait();
        m_texture.Update(m_image.GetData());
    }

    [[nodiscard]] const raylib::Texture& texture() const
    {
        return m_texture;
    }

private:
    const int c_size;
    raylib::Image m_image;
    raylib::Texture m_texture;
    std::vector<std::complex<double>> m_buffer;
    BS::thread_pool m_thread_pool;
};