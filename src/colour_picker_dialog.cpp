// colour_picker_dialog.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include "colour_picker_dialog.hpp"

namespace neogfx
{
	colour_picker_dialog::colour_box::colour_box(colour_picker_dialog& aParent, const colour& aColour) :
		framed_widget(aParent, SolidFrame), iParent(aParent), iColour(aColour)
	{
		set_margins(neogfx::margins{});
	}

	size colour_picker_dialog::colour_box::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += size{ 16, 14 };
		return result;
	}

	size colour_picker_dialog::colour_box::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_picker_dialog::colour_box::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		aGraphicsContext.fill_rect(client_rect(false), iColour);
	}

	void colour_picker_dialog::colour_box::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		framed_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
			iParent.select_colour(iColour.with_alpha(iParent.selected_colour().alpha()));
	}

	colour_picker_dialog::x_picker::x_picker(colour_picker_dialog& aParent) :
		framed_widget(aParent.iRightTopLayout), iParent(aParent)
	{
		set_margins(neogfx::margins{});
		iParent.selection_changed([this]
		{
			update();
		});
	}

	size colour_picker_dialog::x_picker::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += size{ 32, 256 };
		return result;
	}

	size colour_picker_dialog::x_picker::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_picker_dialog::x_picker::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		scoped_units su(*this, UnitsPixels);
		rect cr = client_rect(false);
		for (uint32_t x = 0; x < cr.height(); ++x)
		{
			double nx = x / (cr.height() - 1.0);
			rect line{ cr.top_left() + point{ 0.0, cr.height() - x - 1.0 }, size{ cr.width(), 1.0 } };
			if (iParent.mode() == ModeHSV)
			{
				hsv_colour hsvColour{ nx * 359.0, 1.0, 1.0 };
				aGraphicsContext.fill_rect(line, hsvColour.to_rgb());
			}
		}
	}

	colour_picker_dialog::yz_picker::yz_picker(colour_picker_dialog& aParent) :
		framed_widget(aParent.iRightTopLayout), iParent(aParent), iTexture{ image{size{256, 256}, colour::Black} }, iTracking{false}
	{
		set_margins(neogfx::margins{});
		iParent.selection_changed([this]
		{
			update();
		});
	}

	size colour_picker_dialog::yz_picker::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += size{ 256, 256 };
		return result;
	}

	size colour_picker_dialog::yz_picker::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_picker_dialog::yz_picker::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		rect cr = client_rect(false);
		for (uint32_t y = 0; y < 256; ++y)
		{
			for (uint32_t z = 0; z < 256; ++z)
			{
				double ny = y / (255.0);
				double nz = z / (255.0);
				if (iParent.mode() == ModeHSV)
				{
					hsv_colour hsvColour = iParent.selected_colour_as_hsv(false);
					hsvColour.set_saturation(ny);
					hsvColour.set_value(nz);
					auto rgbColour = hsvColour.to_rgb();
					iPixels[255 - z][y][0] = rgbColour.red();
					iPixels[255 - z][y][1] = rgbColour.green();
					iPixels[255 - z][y][2] = rgbColour.blue();
					iPixels[255 - z][y][3] = 255; // alpha
				}
			}
		}
		iTexture.set_pixels(rect{ point{}, size{256, 256} }, &iPixels[0][0][0]);
		aGraphicsContext.draw_texture(cr.top_left(), iTexture);
		point cursor;
		if (iParent.mode() == ModeHSV)
			cursor = point{iParent.selected_colour_as_hsv(false).saturation() * 255.0, 255.0 - iParent.selected_colour_as_hsv(false).value() * 255.0 };
		aGraphicsContext.fill_circle(cr.top_left() + cursor, 4.0, iParent.selected_colour());
		aGraphicsContext.draw_circle(cr.top_left() + cursor, 4.0, pen{ iParent.selected_colour().light(0x80) ? colour::Black : colour::White });
	}

	void colour_picker_dialog::yz_picker::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		framed_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			select(aPosition - client_rect(false).top_left());
			iTracking = true;
		}
	}

	void colour_picker_dialog::yz_picker::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		framed_widget::mouse_button_released(aButton, aPosition);
		if (!capturing())
			iTracking = false;
	}

	void colour_picker_dialog::yz_picker::mouse_moved(const point& aPosition)
	{
		if (iTracking)
			select(aPosition - client_rect(false).top_left());
	}

	void colour_picker_dialog::yz_picker::select(const point& aCursorPos)
	{
		if (iParent.mode() == ModeHSV)
		{
			auto hsv = iParent.selected_colour_as_hsv(true);
			hsv.set_saturation(std::max(std::min(aCursorPos.x / 255.0, 1.0), 0.0));
			hsv.set_value(std::max(std::min((255.0 - aCursorPos.y) / 255.0, 1.0), 0.0));
			iParent.select_colour(hsv, *this);
		}
	}
	
	colour_picker_dialog::colour_selection::colour_selection(colour_picker_dialog& aParent) :
		framed_widget(aParent.iRightBottomLayout), iParent(aParent)
	{
		set_margins(neogfx::margins{});
		iParent.selection_changed([this]
		{
			update();
		});
	}

	size colour_picker_dialog::colour_selection::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += size{ 60, 80 };
		return result;
	}

	size colour_picker_dialog::colour_selection::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_picker_dialog::colour_selection::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		scoped_units su(*this, aGraphicsContext, UnitsPixels);
		rect cr = client_rect(false);
		const dimension ALPHA_PATTERN_SIZE = 4.0;
		for (coordinate x = 0; x < cr.width(); x += ALPHA_PATTERN_SIZE)
		{
			bool alt = false;
			if (static_cast<uint32_t>((x / ALPHA_PATTERN_SIZE)) % 2 == 1)
				alt = !alt;
			for (coordinate y = 0; y < cr.height(); y += ALPHA_PATTERN_SIZE)
			{
				aGraphicsContext.fill_rect(rect{ cr.top_left() + point{ x, y }, size{ ALPHA_PATTERN_SIZE, ALPHA_PATTERN_SIZE } }, alt ? colour{ 160, 160, 160 } : colour{ 255, 255, 255 });
				alt = !alt;
			}
		}
		rect top = cr;
		rect bottom = top;
		top.cy = top.cy / 2.0;
		bottom.y = top.bottom();
		bottom.cy = bottom.cy / 2.0;
		aGraphicsContext.fill_rect(top, iParent.selected_colour());
		aGraphicsContext.fill_rect(bottom, iParent.current_colour());
	}

	colour_picker_dialog::colour_picker_dialog(const colour& aCurrentColour) :
		dialog("Select Colour", Modal | Titlebar | Close),
		iMode(ModeHSV),
		iCurrentColour(aCurrentColour),
		iSelectedColour(aCurrentColour),
		iUpdatingWidgets(false),
		iLayout(*this),
		iLayout2(iLayout),
		iLeftLayout(iLayout2),
		iRightLayout(iLayout2),
		iRightTopLayout(iRightLayout),
		iRightBottomLayout(iRightLayout, alignment::Left | alignment::Top),
		iColourSelection(*this),
		iChannelLayout(iRightBottomLayout),
		iBasicColoursLabel{ iLeftLayout, "&Basic colours" },
		iBasicColoursLayout{ iLeftLayout },
		iSpacer{ iLeftLayout },
		iCustomColoursLayout{ iLeftLayout },
		iCustomColoursLabel{ iLeftLayout, "&Custom colours" },
		iYZPicker(*this),
		iXPicker(*this),
		iH{ *this, *this },
		iS{ *this, *this },
		iV{ *this, *this },
		iR{ *this, *this },
		iG{ *this, *this },
		iB{ *this, *this },
		iA{ *this, *this },
		iRgb{ *this },
		iAddToCustomColours{iRightLayout, "&Add to Custom Colours" }
	{
		init();
	}

	colour_picker_dialog::colour_picker_dialog(i_widget& aParent, const colour& aCurrentColour) :
		dialog(aParent, "Select Colour", Modal | Titlebar | Close),
		iMode(ModeHSV),
		iCurrentColour(aCurrentColour),
		iSelectedColour(aCurrentColour),
		iUpdatingWidgets(false),
		iLayout(*this),
		iLayout2(iLayout),
		iLeftLayout(iLayout2),
		iRightLayout(iLayout2),
		iRightTopLayout(iRightLayout),
		iRightBottomLayout(iRightLayout, alignment::Left | alignment::Top),
		iColourSelection(*this),
		iChannelLayout(iRightBottomLayout),
		iBasicColoursLabel{iLeftLayout, "&Basic colours"},
		iBasicColoursLayout{iLeftLayout},
		iSpacer{iLeftLayout},
		iCustomColoursLayout{iLeftLayout},
		iCustomColoursLabel{iLeftLayout, "&Custom colours"},
		iYZPicker(*this),
		iXPicker(*this),
		iH{ *this, *this },
		iS{ *this, *this },
		iV{ *this, *this },
		iR{ *this, *this },
		iG{ *this, *this },
		iB{ *this, *this },
		iA{ *this, *this },
		iRgb{ *this },
		iAddToCustomColours{iRightLayout, "&Add to Custom Colours"}
	{
		init();
	}

	colour_picker_dialog::~colour_picker_dialog()
	{
	}

	colour_picker_dialog::mode_e colour_picker_dialog::mode() const
	{
		return iMode;
	}

	void colour_picker_dialog::set_mode(mode_e aMode)
	{
		if (iMode != aMode)
		{
			iMode = aMode;
			update();
		}
	}

	colour colour_picker_dialog::current_colour() const
	{
		return iCurrentColour;
	}

	colour colour_picker_dialog::selected_colour() const
	{
		if (iSelectedColour.is<colour>())
			return static_variant_cast<const colour&>(iSelectedColour);
		else
			return static_variant_cast<const hsv_colour&>(iSelectedColour).to_rgb();
	}

	hsv_colour colour_picker_dialog::selected_colour_as_hsv() const
	{
		return selected_colour_as_hsv(true);
	}

	void colour_picker_dialog::select_colour(const colour& aColour)
	{
		select_colour(aColour, *this);
	}
		
	void colour_picker_dialog::init()
	{
		scoped_units su(static_cast<framed_widget&>(*this), UnitsPixels);
		static const std::set<colour> sBasicColours
		{
			colour::AliceBlue, colour::AntiqueWhite, colour::Aquamarine, colour::Azure, colour::Beige, colour::Bisque, colour::Black, colour::BlanchedAlmond, 
			colour::Blue, colour::BlueViolet, colour::Brown, colour::Burlywood, colour::CadetBlue, colour::Chartreuse, colour::Chocolate, colour::Coral, 
			colour::CornflowerBlue, colour::Cornsilk, colour::Cyan, colour::DarkBlue, colour::DarkCyan, colour::DarkGoldenrod, colour::DarkGray, colour::DarkGreen, 
			colour::DarkKhaki, colour::DarkMagenta, colour::DarkOliveGreen, colour::DarkOrange, colour::DarkOrchid, colour::DarkRed, colour::DarkSalmon, 
			colour::DarkSeaGreen, colour::DarkSlateBlue, colour::DarkSlateGray, colour::DarkTurquoise, colour::DarkViolet, colour::DebianRed, colour::DeepPink, 
			colour::DeepSkyBlue, colour::DimGray, colour::DodgerBlue, colour::Firebrick, colour::FloralWhite, colour::ForestGreen, colour::Gainsboro, 
			colour::GhostWhite, colour::Gold, colour::Goldenrod, colour::Gray, colour::Green, colour::GreenYellow, colour::Honeydew, colour::HotPink, 
			colour::IndianRed, colour::Ivory, colour::Khaki, colour::Lavender, colour::LavenderBlush, colour::LawnGreen, colour::LemonChiffon, colour::LightBlue, 
			colour::LightCoral, colour::LightCyan, colour::LightGoldenrod, colour::LightGoldenrodYellow, colour::LightGray, colour::LightGreen, colour::LightPink, 
			colour::LightSalmon, colour::LightSeaGreen, colour::LightSkyBlue, colour::LightSlateBlue, colour::LightSlateGray, colour::LightSteelBlue, 
			colour::LightYellow, colour::LimeGreen, colour::Linen, colour::Magenta, colour::Maroon, colour::MediumAquamarine, colour::MediumBlue, colour::MediumOrchid, 
			colour::MediumPurple, colour::MediumSeaGreen, colour::MediumSlateBlue, colour::MediumSpringGreen, colour::MediumTurquoise, colour::MediumVioletRed,
			colour::MidnightBlue, colour::MintCream, colour::MistyRose, colour::Moccasin, colour::NavajoWhite, colour::Navy, colour::NavyBlue, colour::OldLace, 
			colour::OliveDrab, colour::Orange, colour::OrangeRed, colour::Orchid, colour::PaleGoldenrod, colour::PaleGreen, colour::PaleTurquoise, colour::PaleVioletRed, 
			colour::PapayaWhip, colour::PeachPuff, colour::Peru, colour::Pink, colour::Plum, colour::PowderBlue, colour::Purple, colour::Red, colour::RosyBrown, 
			colour::RoyalBlue, colour::SaddleBrown, colour::Salmon, colour::SandyBrown, colour::SeaGreen, colour::Seashell, colour::Sienna, colour::SkyBlue, 
			colour::SlateBlue, colour::SlateGray, colour::Snow, colour::SpringGreen, colour::SteelBlue, colour::Tan, colour::Thistle, colour::Tomato, 
			colour::Turquoise, colour::Violet, colour::VioletRed, colour::Wheat, colour::White, colour::WhiteSmoke, colour::Yellow, colour::YellowGreen 
		};
		set_margins(neogfx::margins{16.0});
		iLayout.set_margins(neogfx::margins{});
		iLayout.set_spacing(16.0);
		iLayout2.set_margins(neogfx::margins{});
		iLayout2.set_spacing(16.0);
		iRightLayout.set_spacing(16.0);
		iRightTopLayout.set_spacing(16.0);
		iRightBottomLayout.set_spacing(8.0);
		iChannelLayout.set_spacing(8.0);
		iH.first.label().text().set_text("&Hue:"); iH.second.set_size_policy(size_policy::Minimum); iH.second.text_box().set_hint("359.9"); iH.second.set_minimum(0.0); iH.second.set_maximum(359.9); iH.second.set_step(1);
		iS.first.label().text().set_text("&Sat:"); iS.second.set_size_policy(size_policy::Minimum); iS.second.text_box().set_hint("100.0"); iS.second.set_minimum(0.0); iS.second.set_maximum(100.0); iS.second.set_step(1);
		iV.first.label().text().set_text("&Val:"); iV.second.set_size_policy(size_policy::Minimum); iV.second.text_box().set_hint("100.0"); iV.second.set_minimum(0.0); iV.second.set_maximum(100.0); iV.second.set_step(1);
		iR.first.label().text().set_text("&Red:"); iR.second.set_size_policy(size_policy::Minimum); iR.second.text_box().set_hint("255"); iR.second.set_minimum(0); iR.second.set_maximum(255); iR.second.set_step(1);
		iG.first.label().text().set_text("&Green:"); iG.second.set_size_policy(size_policy::Minimum); iG.second.text_box().set_hint("255"); iG.second.set_minimum(0); iG.second.set_maximum(255); iG.second.set_step(1);
		iB.first.label().text().set_text("&Blue:"); iB.second.set_size_policy(size_policy::Minimum); iB.second.text_box().set_hint("255"); iB.second.set_minimum(0); iB.second.set_maximum(255); iB.second.set_step(1);
		iA.first.label().text().set_text("&Alpha:"); iA.second.set_size_policy(size_policy::Minimum); iA.second.text_box().set_hint("255"); iA.second.set_minimum(0); iA.second.set_maximum(255); iA.second.set_step(1);
		iRgb.set_size_policy(size_policy::Minimum); iRgb.set_hint("#000000"); 
		iChannelLayout.set_dimensions(4, 4);
		iChannelLayout.add_item(iH.first); iChannelLayout.add_item(iH.second);
		iChannelLayout.add_item(iR.first); iChannelLayout.add_item(iR.second);
		iChannelLayout.add_item(iS.first); iChannelLayout.add_item(iS.second);
		iChannelLayout.add_item(iG.first); iChannelLayout.add_item(iG.second);
		iChannelLayout.add_item(iV.first); iChannelLayout.add_item(iV.second);
		iChannelLayout.add_item(iB.first); iChannelLayout.add_item(iB.second);
		iChannelLayout.add_span(grid_layout::cell_coordinates{ 0, 3 }, grid_layout::cell_coordinates{ 1, 3 });
		iChannelLayout.add_item(iRgb); iChannelLayout.add_spacer().set_size_policy(size_policy::Minimum);
		iChannelLayout.add_item(iA.first); iChannelLayout.add_item(iA.second);
		iBasicColoursLayout.set_dimensions(12, 12);
		for (auto const& basicColour : sBasicColours)
			iBasicColoursLayout.add_item(std::make_shared<colour_box>(*this, basicColour));
		iCustomColoursLayout.set_dimensions(2, 12);
		for (uint32_t i = 0; i < 24; ++i)
			iCustomColoursLayout.add_item(std::make_shared<colour_box>(*this, colour::White));
		button_box().add_button(dialog_button_box::Ok);
		button_box().add_button(dialog_button_box::Cancel);
		resize(minimum_size());
		rect desktopRect{ app::instance().surface_manager().desktop_rect(surface()) };
		move_surface((desktopRect.extents() - surface_size()) / 2.0);

		iH.second.value_changed([this]() { if (iUpdatingWidgets) return; auto c = selected_colour_as_hsv(); c.set_hue(iH.second.value()); select_colour(c, iH.second); });
		iS.second.value_changed([this]() { if (iUpdatingWidgets) return; auto c = selected_colour_as_hsv(); c.set_saturation(iS.second.value() / 100.0); select_colour(c, iS.second); });
		iV.second.value_changed([this]() { if (iUpdatingWidgets) return; auto c = selected_colour_as_hsv(); c.set_value(iV.second.value() / 100.0); select_colour(c, iV.second); });
		iR.second.value_changed([this]() { if (iUpdatingWidgets) return; select_colour(selected_colour().with_red(static_cast<colour::component>(iR.second.value())), iR.second); });
		iG.second.value_changed([this]() { if (iUpdatingWidgets) return; select_colour(selected_colour().with_green(static_cast<colour::component>(iG.second.value())), iG.second); });
		iB.second.value_changed([this]() { if (iUpdatingWidgets) return; select_colour(selected_colour().with_blue(static_cast<colour::component>(iB.second.value())), iB.second); });
		iA.second.value_changed([this]() { if (iUpdatingWidgets) return; select_colour(selected_colour().with_alpha(static_cast<colour::component>(iA.second.value())), iA.second); });
		iRgb.text_changed([this]() { if (iUpdatingWidgets) return; select_colour(colour{ iRgb.text() }, iRgb); });

		update_widgets(*this);
	}

	hsv_colour colour_picker_dialog::selected_colour_as_hsv(bool aChangeRepresentation) const
	{
		if (iSelectedColour.is<colour>())
		{
			hsv_colour result = static_variant_cast<const colour&>(iSelectedColour).to_hsv();
			if (aChangeRepresentation)
				iSelectedColour = result;
			return result;
		}
		else
			return static_variant_cast<const hsv_colour&>(iSelectedColour);
	}

	void colour_picker_dialog::select_colour(const representations& aColour, const i_widget& aUpdatingWidget)
	{
		if (iUpdatingWidgets)
			return;
		if (iSelectedColour != aColour)
		{
			iSelectedColour = aColour;
			update_widgets(aUpdatingWidget);
			selection_changed.trigger();
		}
	}

	void colour_picker_dialog::update_widgets(const i_widget& aUpdatingWidget)
	{
		if (iUpdatingWidgets)
			return;
		iUpdatingWidgets = true;
		if (&aUpdatingWidget != &iH.second)
			iH.second.set_value(static_cast<int32_t>(selected_colour_as_hsv(false).hue()));
		if (&aUpdatingWidget != &iS.second)
			iS.second.set_value(static_cast<int32_t>(selected_colour_as_hsv(false).saturation() * 100.0));
		if (&aUpdatingWidget != &iV.second)
			iV.second.set_value(static_cast<int32_t>(selected_colour_as_hsv(false).value() * 100.0));
		if (&aUpdatingWidget != &iR.second)
			iR.second.set_value(selected_colour().red());
		if (&aUpdatingWidget != &iG.second)
			iG.second.set_value(selected_colour().green());
		if (&aUpdatingWidget != &iB.second)
			iB.second.set_value(selected_colour().blue());
		if (&aUpdatingWidget != &iA.second)
			iA.second.set_value(selected_colour().alpha());
		if (&aUpdatingWidget != &iRgb)
			iRgb.set_text(selected_colour().to_hex_string());
		iUpdatingWidgets = false;
	}
}