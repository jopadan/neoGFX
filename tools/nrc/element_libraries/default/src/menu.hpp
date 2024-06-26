// menu.hpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

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

#include <neogfx/tools/nrc/ui_element.hpp>

#include "action.hpp"

namespace neogfx::nrc
{
    class menu : public ui_element<>
    {
    public:
        menu(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            ui_element<>{ aParser, aParent, ui_element_type::Menu },
            iTitle{ aParent.parser().get_optional<neolib::string>("title") },
            iImage{ aParent.parser().get_optional<neolib::string>("image") }
        {
            add_header("neogfx/gui/widget/menu.hpp");
            add_data_names({ "title", "image", "action" });
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "action")
            {
                auto const& reference = aData.get<neolib::i_string>();
                if (reference == "separator")
                    new action_ref{ parser(), *this };
                else
                    new action_ref{ parser(), *this, reference };
            }
            ui_element<>::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            ui_element<>::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
        }
        void emit_preamble() const override
        {
            emit("  %1% %2%;\n", type_name(), id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if ((parent().type() & ui_element_type::Menu) == ui_element_type::Menu)
            {
                if (iTitle)
                    emit(",\n"
                        "   %1%{ %2%, \"%3%\"_t, menu_type::Popup }", id(), parent().id(), *iTitle);
                else
                    emit(",\n"
                        "   %1%{ %2% }", id(), parent().id());
            }
            else
            {
                if (iTitle)
                    emit(",\n"
                        "   %1%{ \"%3%\"_t, menu_type::Popup }", id(), parent().id(), *iTitle);
            }
            ui_element<>::emit_ctor();
        }
        void emit_body() const override
        {
            ui_element<>::emit_body();
            if (iImage)
                emit("   %1%.set_image(image{ \"%2%\" });\n", id(), *iImage);
        }
    protected:
        using ui_element<>::emit;
    private:
        neolib::optional<neolib::string> iTitle;
        neolib::optional<neolib::string> iImage;
    };
}
