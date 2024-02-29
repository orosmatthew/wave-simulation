#include <chrono>
#include <optional>
#include <thread>

#include <raylib-cpp.hpp>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "res/roboto-regular.h"
#include "schrodinger_renderer.hpp"
#include "schrodinger_sim.hpp"
#include "ui.hpp"

namespace rl = raylib;

constexpr int sim_size = 256;
constexpr int base_font_size = 16;

static rl::Rectangle sim_screen_rect(const int toolbar_height)
{
    const int size = std::max(std::min(GetScreenWidth(), GetScreenHeight() - toolbar_height), 1);
    return { static_cast<float>(GetScreenWidth()) / 2.0f - static_cast<float>(size) / 2.0f,
             static_cast<float>(toolbar_height),
             static_cast<float>(size),
             static_cast<float>(size) };
}

static std::optional<Vector2i> mouse_to_sim(const rl::Vector2 mouse_pos, const int toolbar_height)
{
    const rl::Vector2 sim_pos_f = (mouse_pos - sim_screen_rect(toolbar_height).GetPosition()) * sim_size
        / sim_screen_rect(toolbar_height).GetWidth();
    return Vector2i { static_cast<int>(sim_pos_f.x), static_cast<int>(sim_pos_f.y) };
}

static void resize_font(rl::Font& font, const int size)
{
    font = LoadFontFromMemory(
        ".ttf", font_robot_regular_ttf_bin, static_cast<int>(font_robot_regular_ttf_bin_size), size, nullptr, 0);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, size);
}

static void handle_font_scale_inputs(rl::Font& font)
{
    std::optional<int> new_size;
    if (IsKeyPressed(KEY_EQUAL) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT)) {
        new_size = font.GetBaseSize() + 1;
    }
    if (IsKeyPressed(KEY_MINUS) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT)) {
        new_size = font.GetBaseSize() - 1;
    }
    if (new_size.has_value()) {
        resize_font(font, new_size.value());
    }
}

enum class Mode { none, interact, walls };

static void handle_sim_inputs(const Mode mode, SchrodingerSim& sim, const int toolbar_height)
{
    const rl::Vector2 mouse_pos = GetMousePosition();
    if (const std::optional<Vector2i> sim_pos = mouse_to_sim(mouse_pos, toolbar_height);
        sim_pos.has_value() && sim.in_bounds(sim_pos.value())) {
        if (mode == Mode::walls) {
            // if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            //     constexpr int radius = 10;
            //     for (int x = -radius; x < radius; ++x) {
            //         for (int y = -radius; y < radius; ++y) {
            //             if (const Vector2i pos { sim_pos->x + x, sim_pos->y + y };
            //                 sim.in_bounds(pos) && std::sqrt(x * x + y * y) <= radius) {
            //                 sim.set_at(pos, 0.0);
            //                 sim.set_fixed_at(pos, true);
            //             }
            //         }
            //     }
            // }
            // if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            //     constexpr int radius = 20;
            //     for (int x = -radius; x < radius; ++x) {
            //         for (int y = -radius; y < radius; ++y) {
            //             if (const Vector2i pos { sim_pos->x + x, sim_pos->y + y };
            //                 sim.in_bounds(pos) && std::sqrt(x * x + y * y) <= radius && sim.fixed_at(pos)) {
            //                 sim.set_at(pos, 0.0);
            //                 sim.set_fixed_at(pos, false);
            //             }
            //         }
            //     }
            // }
        }
        else if (mode == Mode::interact) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                sim.set_at(sim_pos.value(), std::complex(100.0, 0.0));
            }
        }
    }
}

struct State {
    rl::Window window;
    rl::Font font;
    float scale;
    SchrodingerSim sim;
    SchrodingerRenderer sim_renderer;
    Mode mode;
    LabelledDropdown theme_dropdown;
    LabelledDropdown mode_dropdown;
    SchrodingerRenderer::Theme renderer_theme;
    int show_fps;
    bool init;
    std::atomic<bool> should_exit;
    std::chrono::time_point<std::chrono::steady_clock> frame_count_start;
    std::atomic<int> frame_count;
    std::thread sim_thread;
};

void init_packet(SchrodingerSim& sim)
{
    sim.lock_write();
    constexpr auto i = std::complex(0.0, 1.0);
    for (int j = 0; j < sim_size * sim_size; ++j) {
        constexpr auto a = 1.0;
        constexpr auto x0 = 128;
        constexpr auto y0 = 128;
        constexpr auto sigma_x = 10.0;
        constexpr auto sigma_y = 10.0;
        constexpr auto mom_x = 2.0;
        constexpr auto mom_y = 0.0;
        const auto [x, y] = sim.idx_to_pos(j);
        const auto x_term = std::exp(-std::pow(x - x0, 2.0) / (2.0 * std::pow(sigma_x, 2.0)));
        const auto y_term = std::exp(-std::pow(y - y0, 2.0) / (2.0 * std::pow(sigma_y, 2.0)));
        const auto pos = x_term * y_term;
        const auto mom = std::exp(i * (mom_x * x + mom_y * y));
        sim.set_at({ x, y }, a * pos * mom);
    }
    sim.unlock_write();
}

void loop(State* s)
{
    if (!s->init) {
        init_packet(s->sim);
        s->init = true;
    }

    const int toolbar_height = static_cast<int>(std::round(100.0f * s->scale));
    handle_font_scale_inputs(s->font);

    if (IsKeyPressed(KEY_C)) {
        s->sim.clear();
    }

    // if (IsKeyPressed(KEY_N)) {
    //     s->mode = Mode::none;
    //     s->mode_dropdown.set_active(static_cast<int>(s->mode));
    // }
    // else if (IsKeyPressed(KEY_I)) {
    //     s->mode = Mode::interact;
    //     s->mode_dropdown.set_active(static_cast<int>(s->mode));
    // }
    // else if (IsKeyPressed(KEY_W)) {
    //     s->mode = Mode::walls;
    //     s->mode_dropdown.set_active(static_cast<int>(s->mode));
    // }

    // handle_sim_inputs(s->mode, s->sim, toolbar_height);
    s->sim_renderer.update(s->sim, s->renderer_theme);

    BeginDrawing();
    ClearBackground(LIGHTGRAY);
    DrawTexturePro(
        s->sim_renderer.texture(),
        { 0.0f, 0.0f, sim_size, sim_size },
        sim_screen_rect(toolbar_height),
        { 0.0f, 0.0f },
        0.0f,
        WHITE);
    if (s->show_fps) {
        const float font_size = std::round(s->scale * static_cast<float>(base_font_size));
        rl::DrawTextEx(
            s->font,
            "FPS: " + std::to_string(GetFPS()),
            { 10.0f, static_cast<float>(toolbar_height) + 10.0f },
            font_size,
            1.0f,
            DARKGREEN);
        const auto now = std::chrono::steady_clock::now();
        const int tick_rate = static_cast<int>(std::round(
            static_cast<float>(s->frame_count) / std::chrono::duration<float>(now - s->frame_count_start).count()));
        rl::DrawTextEx(
            s->font,
            "Tick Rate: " + std::to_string(tick_rate),
            { 10.0f, static_cast<float>(toolbar_height) + 30.0f },
            font_size,
            1.0f,
            BLUE);
        s->frame_count = 0;
        s->frame_count_start = now;
    }

    if (const float scale = GetWindowScaleDPI().x; scale != s->scale) {
        s->scale = scale;
        resize_font(s->font, static_cast<int>(std::round(static_cast<float>(base_font_size) * s->scale)));
    }

    {
        const float ui_height = 25.0f * s->scale;
        const float ui_padding = 10.0f * s->scale;

        float offset_x = ui_padding;
        if (GuiButton({ ui_padding, ui_padding, 70.0f * s->scale, ui_height }, "Clear [C]")) {
            s->sim.clear();
        }
        offset_x += 70.0f * s->scale + ui_padding;
        GuiToggleSlider({ offset_x, ui_padding, 60.0f * s->scale, ui_height }, "FPS;FPS", &s->show_fps);

        offset_x = ui_padding;
        s->theme_dropdown.draw_and_update(
            { offset_x, ui_height + ui_padding * 1.5f, 110.0f * s->scale, ui_height * 2.0f });
        s->renderer_theme = static_cast<SchrodingerRenderer::Theme>(s->theme_dropdown.active());
        offset_x += 110.0f * s->scale + ui_padding;
        s->mode_dropdown.draw_and_update(
            { offset_x, ui_height + ui_padding * 1.5f, 90.0f * s->scale, ui_height * 2.0f });
        s->mode = static_cast<Mode>(s->mode_dropdown.active());
    }

    EndDrawing();
}

void sim_thread(void* state)
{
    auto* s = static_cast<State*>(state);
    while (!s->should_exit) {
        s->sim.update();
        s->frame_count += 1;
    }
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    rl::Window window { 600, 700, "Schrodinger Simulation" };
    window.SetMinSize(600, 700);
    const int font_size = static_cast<int>(std::round(static_cast<float>(base_font_size) * GetWindowScaleDPI().x));
    rl::Font font = LoadFontFromMemory(
        ".ttf", font_robot_regular_ttf_bin, static_cast<int>(font_robot_regular_ttf_bin_size), font_size, nullptr, 0);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, font_size);

    constexpr auto sim_props = SchrodingerSim::Properties {
        .size = sim_size, .grid_spacing = 1.0, .timestep = 0.002, .hbar = 1.0, .mass = 1.0
    };

    auto mode = Mode::interact;

    LabelledDropdown theme_dropdown("Theme");
    theme_dropdown.set_items({ "Probability", "Waves" });

    LabelledDropdown mode_dropdown("Mode");
    mode_dropdown.set_items({ "None [N]", "Interact [I]", "Walls [W]" });
    mode_dropdown.set_active(static_cast<int>(mode));

    State state {
        .window = window,
        .font = std::move(font),
        .scale = 1.0f,
        .sim = SchrodingerSim(sim_props),
        .sim_renderer = SchrodingerRenderer(sim_props.size),
        .mode = mode,
        .theme_dropdown = std::move(theme_dropdown),
        .mode_dropdown = std::move(mode_dropdown),
        .renderer_theme = SchrodingerRenderer::Theme::probability,
        .show_fps = 0,
        .init = false,
        .should_exit = false,
        .frame_count_start = std::chrono::steady_clock::now(),
        .frame_count = 0,
        .sim_thread = std::thread(sim_thread, &state),
    };
    while (!window.ShouldClose()) {
        loop(&state);
    }
    state.should_exit = true;
    state.sim_thread.join();
    return EXIT_SUCCESS;
}
