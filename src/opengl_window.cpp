// opengl_window.cpp
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
#include "opengl_window.hpp"
#include "app.hpp"
#ifdef _WIN32
#include <D2d1.h>
#endif
#include <GL/glew.h>
#include <GL/GL.h>

namespace neogfx
{
	opengl_window::opengl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler) :
		native_window(aRenderingEngine, aSurfaceManager),
		iEventHandler(aEventHandler),
		iRenderer(app::instance(), [this](neolib::callback_timer&){ render(); }, 1000 / 60, true),
		iFrameRate(50),
		iLastFrameTime(0),
		iRendered(false),
		iRendering(false)
	{
#ifdef _WIN32
		ID2D1Factory* m_pDirect2dFactory;
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
		FLOAT dpiX, dpiY;
		m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);
		iPixelDensityDpi = size(static_cast<size::dimension_type>(dpiX), static_cast<size::dimension_type>(dpiY));
#endif
	}

	opengl_window::~opengl_window()
	{
	}

	bool opengl_window::using_frame_buffer() const
	{
		return true;
	}

	void opengl_window::limit_frame_rate(uint32_t aFps)
	{
		iFrameRate = aFps;
	}

	void opengl_window::clear_rendering_flag()
	{
		iRendered = false;
	}

	void opengl_window::invalidate_surface(const rect& aInvalidatedRect)
	{
		if (std::find(iInvalidatedRects.begin(), iInvalidatedRects.end(), aInvalidatedRect) == iInvalidatedRects.end())
			iInvalidatedRects.push_back(aInvalidatedRect);
	}

	size opengl_window::extents() const
	{
		return surface_size();
	}
	
	dimension opengl_window::horizontal_dpi() const
	{
		return iPixelDensityDpi.cx;
	}
	
	dimension opengl_window::vertical_dpi() const
	{
		return iPixelDensityDpi.cy;
	}

	dimension opengl_window::em_size() const
	{
		return 0;
	}

	void opengl_window::render()
	{
		if (!iRenderer.waiting())
			iRenderer.again();
		if (iRendered || iRendering || processing_event() || iInvalidatedRects.empty())
			return;
		uint64_t now = app::instance().program_elapsed_ms();
		if (iFrameRate != boost::none && now - iLastFrameTime < 1000 / *iFrameRate)
			return;
		
		iRendering = true;
		iLastFrameTime = now;
		
		rect invalidatedRect = iInvalidatedRects[0];
		for (const auto& ir : iInvalidatedRects)
		{
			invalidatedRect = invalidatedRect.combine(ir);
		}
		iInvalidatedRects.clear();
		invalidatedRect.cx = std::min(invalidatedRect.cx, surface_size().cx - invalidatedRect.x);
		invalidatedRect.cy = std::min(invalidatedRect.cy, surface_size().cy - invalidatedRect.y);
		
		static bool initialized = false;
		if (!initialized)
		{
			rendering_engine().initialize();
			initialized = true;
		}

		activate_context();

		glCheck(glViewport(0, 0, static_cast<GLsizei>(extents().cx), static_cast<GLsizei>(extents().cy)));
		glCheck(glMatrixMode(GL_PROJECTION));
		glCheck(glLoadIdentity());
		glCheck(glScalef(1.0, -1.0, 1.0));
		glCheck(glMatrixMode(GL_MODELVIEW));
		glCheck(glLoadIdentity());
		glCheck(glOrtho(0.0, extents().cx, 0.0, extents().cy, -1.0, 1.0));
		glCheck(glEnableClientState(GL_VERTEX_ARRAY));
		glCheck(glEnableClientState(GL_COLOR_ARRAY));
		glCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glEnable(GL_MULTISAMPLE));
		glCheck(glEnable(GL_BLEND));
		if (iFrameBufferSize.cx < static_cast<double>(extents().cx) || iFrameBufferSize.cy < static_cast<double>(extents().cy))
		{
			if (iFrameBufferSize != size{})
			{
				glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
				glCheck(glDeleteTextures(1, &iFrameBufferTexture));
				glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
			}
			iFrameBufferSize = size(
				iFrameBufferSize.cx < extents().cx ? extents().cx * 1.5f : iFrameBufferSize.cx,
				iFrameBufferSize.cy < extents().cy ? extents().cy * 1.5f : iFrameBufferSize.cy);
			glCheck(glGenFramebuffers(1, &iFrameBuffer));
			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
			glCheck(glGenTextures(1, &iFrameBufferTexture));
			glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, iFrameBufferTexture));
			glCheck(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, static_cast<GLsizei>(iFrameBufferSize.cx), static_cast<GLsizei>(iFrameBufferSize.cy), true));
			glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, iFrameBufferTexture, 0));
			glCheck(glGenRenderbuffers(1, &iDepthStencilBuffer));
			glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
			glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(iFrameBufferSize.cx), static_cast<GLsizei>(iFrameBufferSize.cy)));
			glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
			glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
		}
		else
		{
			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
			glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, iFrameBufferTexture));
			glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
		}
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_NO_ERROR && status != GL_FRAMEBUFFER_COMPLETE)
			throw failed_to_create_framebuffer(status);
		glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, iFrameBuffer));
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));

		glCheck(iEventHandler.native_window_render(invalidatedRect));

		glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
		glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, iFrameBuffer));
		glCheck(glBlitFramebuffer(0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), 0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), GL_COLOR_BUFFER_BIT, GL_NEAREST));

		display();
		deactivate_context();
		
		iRendering = false;
		iRendered = true;
	}

	bool opengl_window::rendered() const
	{
		return iRendered;
	}

	i_native_window_event_handler& opengl_window::event_handler() const
	{
		return iEventHandler;
	}
}