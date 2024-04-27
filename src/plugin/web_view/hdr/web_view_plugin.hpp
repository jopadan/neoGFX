/*
  web_view_plugin.hpp

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

#include <neolib/app/i_application.hpp>
#include <neolib/plugin/i_plugin.hpp>

#include <neogfx/gui/widget/i_web_view.hpp>

namespace neogfx
{
    class web_view_plugin : public neolib::reference_counted<neolib::i_plugin>
    {
        // construction
    public:
        web_view_plugin(neolib::i_application& aApplication);
        // i_discoverable
    public:
        bool discover(const neolib::uuid& aId, void*& aObject) final;
        // i_plugin
    public:
        const neolib::uuid& id() const final;
        const neolib::i_string& name() const final;
        const neolib::i_string& description() const final;
        const neolib::i_version& version() const final;
        const neolib::i_string& copyright() const final;
        bool load() final;
        bool unload() final;
        bool loaded() const final;
        bool open_uri(const neolib::i_string& aUri) final;
    private:
        ref_ptr<i_web_view_factory> iFactory;
    };

    class web_view_factory : public neolib::reference_counted<i_web_view_factory>
    {
    public:
        void create_canvas(i_widget& aParent, i_ref_ptr<i_web_view>& aWebView) final;
        void create_canvas(i_layout& aLayout, i_ref_ptr<i_web_view>& aWebView) final;
    };
}
