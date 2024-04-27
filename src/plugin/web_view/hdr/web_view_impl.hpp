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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/i_web_view.hpp>

namespace neogfx
{
    class web_view_impl : public widget<i_web_view>
    {
        using base_type = widget<i_web_view>;
    public:
        using abstract_type = i_web_view;
    public:
        web_view_impl(i_widget& aParent);
        web_view_impl(i_layout& aLayout);
    private:
        void init();
    };
}
