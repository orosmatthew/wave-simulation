#pragma once

class LabelledDropdown {
public:
    explicit LabelledDropdown(std::string label)
        : m_label(std::move(label))
        , m_active(0)
        , m_edit_mode(false)
    {
    }

    void set_items(const std::vector<std::string>& items)
    {
        m_items_str = "";
        for (int i = 0; i < items.size(); ++i) {
            m_items_str += items[i] + (i == items.size() - 1 ? "" : ";");
        }
    }

    [[nodiscard]] int active() const
    {
        return m_active;
    }

    void set_active(const int value)
    {
        m_active = value;
    }

    void draw_and_update(const Rectangle rect)
    {
        GuiLabel({ rect.x, rect.y, rect.width, rect.height / 2.0f }, m_label.c_str());
        if (GuiDropdownBox(
                { rect.x, rect.y + rect.height / 2.0f, rect.width, rect.height / 2.0f },
                m_items_str.c_str(),
                &m_active,
                m_edit_mode)) {
            m_edit_mode = !m_edit_mode;
        }
    }

private:
    static constexpr float sc_label_padding = 5.0f;
    std::string m_label;
    std::string m_items_str;
    int m_active;
    bool m_edit_mode;
};