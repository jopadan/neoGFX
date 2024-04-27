/*
  web_view_impl.hpp

  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.

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

#include <neogfx/neogfx.hpp>

#include "web_view_impl.hpp"

namespace neogfx
{
    web_view_impl::web_view_impl(i_widget& aParent) :
        base_type{ aParent }
    {
        init();
    }

    web_view_impl::web_view_impl(i_layout& aLayout) :
        base_type{ aLayout }
    {
        init();
    }

    void web_view_impl::init()
    {
        Painting([this](i_graphics_context& aGc)
        {
            aGc.fill_rect(client_rect(), color::Red);
            aGc.draw_text(point{}, u8"neogfx::web_view_impl 🎊🎉😎"_t, font().with_size(48),
                text_format{ color::White, text_effect{ text_effect_type::Outline, color::Black, 4.0_dip } });
        });
    }
}
