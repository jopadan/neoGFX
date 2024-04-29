/*
  web_view_canvas.hpp

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

#include <cef/include/views/cef_window.h>

#include <neogfx/gui/window/i_native_window.hpp>

#include "web_view_canvas.hpp"

namespace neogfx
{
    web_view_canvas::web_view_canvas(i_widget& aParent, i_optional<i_string> const& aUrl) :
        base_type{ aParent },
        iUrl{ aUrl.has_value() ? decltype(iUrl){ aUrl.value().to_std_string_view() } : std::nullopt }
    {
        init();
    }

    web_view_canvas::web_view_canvas(i_layout& aLayout, i_optional<i_string> const& aUrl) :
        base_type{ aLayout },
        iUrl{ aUrl.has_value() ? decltype(iUrl){ aUrl.value().to_std_string_view() } : std::nullopt }
    {
        init();
    }

    web_view_canvas::~web_view_canvas()
    {
    }

    logical_coordinate_system web_view_canvas::logical_coordinate_system() const
    {
        return neogfx::logical_coordinate_system::AutomaticGame;
    }

    void web_view_canvas::resized()
    {
        base_type::resized();

        iBrowser->GetHost()->WasResized();
    }

    void web_view_canvas::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);

        auto const clientRect = client_rect(false);
        aGc.draw_texture(clientRect, back_buffer(), rect{ point{}, clientRect.extents() });
    }

    void web_view_canvas::load_url(i_string const& aUrl)
    {
        iUrl = aUrl.to_std_string_view();
        iBrowser->GetMainFrame()->LoadURL(CefString{ aUrl.to_std_string() });
    }

    void web_view_canvas::init()
    {
        set_margin(neogfx::margin{});
        set_padding(neogfx::padding{});

        CefWindowInfo window_info;
#ifdef _WIN32
        window_info.SetAsWindowless(static_cast<HWND>(root().native_window().native_handle()));
#else
        window_info.SetAsWindowless(nullptr);
#endif

        CefBrowserSettings browser_settings;

        iBrowser = CefBrowserHost::CreateBrowserSync(window_info, this, !iUrl ? CefString{} : CefString{ iUrl.value() }, browser_settings, nullptr, nullptr);
    }

    i_texture& web_view_canvas::back_buffer() const
    {
        scoped_units_context suc{ *this };
        auto const clientRect = client_rect(false);
        auto const desiredBackBufferExtents = dpi_scale(clientRect.extents());
        auto const currentBackBufferExtents = iBackBuffer ? iBackBuffer->extents() : size{};
        if (!iBackBuffer || 
            currentBackBufferExtents.cx < desiredBackBufferExtents.cx || 
            currentBackBufferExtents.cy < desiredBackBufferExtents.cy)
            iBackBuffer = service<i_texture_manager>().create_texture(
                desiredBackBufferExtents.max(currentBackBufferExtents), 1.0, texture_sampling::Normal);
        return *iBackBuffer;
    }

    CefRefPtr<CefRenderHandler> web_view_canvas::GetRenderHandler()
    {
        return this;
    }

    bool web_view_canvas::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
    {
        scoped_units_context suc{ *this };
        CefRect view_rect;
        GetViewRect(browser, view_rect);
        screen_info.device_scale_factor = 2.0f;
        screen_info.rect = view_rect;
        screen_info.available_rect = view_rect;        
        return true;
    }

    void web_view_canvas::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
    {
        scoped_units_context suc{ *this };
        auto const clientRect = client_rect(false);
        basic_size<int> const viewExtents = (clientRect.extents() / dpi_scale(1.0f)).ceil();
        rect = CefRect(0, 0, viewExtents.cx, viewExtents.cy);
    }

    void web_view_canvas::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
    {
        back_buffer().set_pixels(rect{ point{}, basic_size<int>{ width, height } }, buffer, texture_data_format::BGRA);
        update();
    }
}
