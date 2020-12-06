// element.hpp
/*
neoGFX Design Studio
Copyright(C) 2020 Leigh Johnston

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
#include <algorithm>
#include <boost/format.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/optional.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/string.hpp>
#include <neolib/task/event.hpp>
#include <neogfx/core/units.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/tools/DesignStudio/symbol.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>

namespace neogfx::DesignStudio
{
    struct user_interface {};

    template <typename Type>
    inline element_group default_element_group()
    {
        if constexpr (std::is_base_of_v<app, Type>)
            return element_group::App;
        else if constexpr (std::is_base_of_v<action, Type>)
            return element_group::Action;
        else if constexpr (std::is_base_of_v<i_menu, Type>)
            return element_group::Menu;
        else if constexpr (std::is_base_of_v<i_widget, Type>)
            return element_group::Widget;
        else if constexpr (std::is_base_of_v<i_layout, Type>)
            return element_group::Layout;
        else if constexpr (std::is_base_of_v<i_project, Type>)
            return element_group::Project;
        else if constexpr (std::is_base_of_v<user_interface, Type>)
            return element_group::UserInterface;
        else
            return element_group::Unknown;
    }

    template <typename BaseType>
    class element_variant
    {
    public:
        typedef BaseType base_type;
    };

    template <typename Type, typename Base = i_element>
    class element : public neolib::reference_counted<Base>
    {
        typedef element<Base> self_type;
        typedef neolib::reference_counted<Base> base_type;
    public:
        define_declared_event(ModeChanged, mode_changed)
        define_declared_event(SelectionChanged, selection_changed)
    public:
        using typename i_element::no_parent;
        using typename i_element::no_caddy;
    public:
        typedef abstract_t<Base> abstract_type;
        typedef neolib::vector<ref_ptr<abstract_type>> children_t;
    public:
        element(i_element_library const& aLibrary, std::string const& aType, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iParent { nullptr }, 
            iGroup{ aGroup }, 
            iType{ aType }
        {
        }
        element(i_element_library const& aLibrary, std::string const& aType, std::string const& aId, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iParent{ nullptr }, 
            iGroup{ aGroup }, 
            iType{ aType }, 
            iId{ aId }
        {
        }
        element(i_element_library const& aLibrary, i_element& aParent, std::string const& aType, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iParent{ &aParent },
            iGroup{ aGroup }, 
            iType{ aType }
        {
            parent().add_child(*this);
        }
        element(i_element_library const& aLibrary, i_element& aParent, std::string const& aType, std::string const& aId, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iParent{ &aParent },
            iGroup{ aGroup },
            iType{ aType }, 
            iId{ aId }
        {
            parent().add_child(*this);
        }
        ~element()
        {
        }
    public:
        i_element_library const& library() const override
        {
            return iLibrary;
        }
        element_group group() const override
        {
            return iGroup;
        }
        neolib::i_string const& type() const override
        {
            return iType;
        }
        neolib::i_string const& id() const override
        {
            return iId;
        }
    public:
        i_element const& root() const override
        {
            i_element const* e = this;
            while (e->has_parent())
                e = &e->parent();
            return *e;
        }
        i_element& root() override
        {
            return const_cast<i_element&>(to_const(*this).root());
        }
        bool has_parent() const override
        {
            return iParent != nullptr;
        }
        i_element const& parent() const override
        {
            if (has_parent())
                return *iParent;
            throw no_parent();
        }
        i_element& parent() override
        {
            return const_cast<i_element&>(to_const(*this).parent());
        }
        void set_parent(i_element& aParent) override
        {
            iParent = &aParent;
        }
        children_t const& children() const override
        {
            return iChildren;
        }
        children_t& children() override
        {
            return iChildren;
        }
        void add_child(i_element& aChild) override
        {
            if (aChild.group() != element_group::App || group() != element_group::UserInterface)
                children().push_back(neolib::ref_ptr<i_element>{ &aChild });
            else
            {
                aChild.set_parent(parent());
                parent().children().insert(std::prev(parent().children().end()), neolib::ref_ptr<i_element>{ &aChild });
            }
        }
        void remove_child(i_element& aChild) override
        {
            auto existing = std::find_if(children().begin(), children().end(), [&](auto&& e) { return &*e == &aChild; });
            if (existing != children().end())
                children().erase(existing);
        }
    public:
        bool has_caddy() const override
        {
            return iCaddy != nullptr;
        }
        i_element_caddy& caddy() const override
        {
            if (has_caddy())
                return *iCaddy;
            throw no_caddy();
        }
        void set_caddy(i_element_caddy& aCaddy) override
        {
            iCaddy = aCaddy;
        }
        bool has_layout_item() const override
        {
            return iLayoutItem != nullptr;
        }
        void layout_item(i_ref_ptr<i_layout_item>& aItem) const override
        {
            if (!iLayoutItem)
            {
                if constexpr (std::is_base_of_v<i_widget, Type>)
                {
                    if constexpr (std::is_constructible_v<Type, neolib::i_string&>)
                        iLayoutItem = make_ref<Type>(iId.to_std_string());
                    else if constexpr (std::is_default_constructible_v<Type>)
                        iLayoutItem = make_ref<Type>();
                    else
                    {
                        // todo: widget creation for the other widget types
                    }
                }
            }
            aItem = iLayoutItem;
        }
    public:
        element_mode mode() const override
        {
            return iMode;
        }
        void set_mode(element_mode aMode) override
        {
            if (iMode != aMode)
            {
                iMode = aMode;
                if (mode() == element_mode::Edit)
                {
                    root().visit([&](i_element& aElement)
                    {
                        if (&aElement != this && aElement.mode() == element_mode::Edit)
                            aElement.set_mode(element_mode::None);
                    });
                }
                ModeChanged.trigger();
            }
        }
        bool is_selected() const override
        {
            return iSelected;
        }
        void select(bool aSelected = true, bool aDeselectRest = true) override
        {
            if (iSelected != aSelected)
            {
                iSelected = aSelected;
                SelectionChanged.trigger();
            }
            if (aDeselectRest)
            {
                root().visit([&](i_element& aElement)
                {
                    if (&aElement != this && aElement.is_selected())
                        aElement.select(false, false);
                });
            }
        }
    public:
        const i_element_library& iLibrary;
        i_element* iParent;
        element_group iGroup;
        neolib::string iType;
        neolib::string iId;
        children_t iChildren;
        mutable ref_ptr<i_layout_item> iLayoutItem;
        ref_ptr<i_element_caddy> iCaddy;
        element_mode iMode = element_mode::None;
        bool iSelected = false;
    };
}
