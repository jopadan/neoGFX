// terminal.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2022 Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/widget/cursor.hpp>
#include <neogfx/gui/widget/i_terminal.hpp>

namespace neogfx
{
    class terminal : public scrollable_widget<framed_widget<widget<i_terminal>>>
    {
    public:
        define_declared_event(Input, input, i_string const&)
    private:
        typedef scrollable_widget<framed_widget<widget<i_terminal>>> base_type;
    private:
        struct attribute
        {
            color ink;
            color paper;
        };
        struct line
        {
            std::u32string text;
            mutable optional_glyph_text glyphs;
            std::vector<attribute> attributes;
        };
        typedef basic_size<std::uint32_t> text_size;
        typedef basic_point<std::uint32_t> cursor_pos;
    public:
        terminal();
        terminal(i_widget& aParent);
        terminal(i_layout& aLayout);
        ~terminal(); 
    public:
        neogfx::size_policy size_policy() const override;
    public:
        void paint(i_graphics_context& aGc) const override;
        color palette_color(color_role aColorRole) const override;
    public:
        void set_font(optional_font const& aFont) override;
    public:
        neogfx::focus_policy focus_policy() const override;
        void focus_gained(focus_reason aFocusReason) override;
        void focus_lost(focus_reason aFocusReason) override;
    public:
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        neogfx::mouse_cursor mouse_cursor() const override;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool text_input(i_string const& aText) override;
    public:
        void output(i_string const& aOutput) override;
        neogfx::cursor& cursor() const;
    private:
        void init();
        void animate();
        void set_cursor_pos(cursor_pos aCursorPos);
        rect cursor_rect() const;
        void draw_cursor(i_graphics_context& aGc) const;
    private:
        text_size iTerminalSize;
        text_size iBufferSize;
        cursor_pos iCursorPos;
        std::vector<line> iBuffer;
        mutable neogfx::cursor iCursor;
        uint64_t iCursorAnimationStartTime;
        widget_timer iAnimator;
        sink iSink;
    };
}