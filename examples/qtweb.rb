# Copyright(c) 2010 Network Applied Communication Lab.  All rights Reserved.
#
# Permission is hereby granted, free of  charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the software without  restriction, including without limitation the rights
# to  use, copy, modify,  merge, publish,  distribute, sublicense,  and/or sell
# copies  of the  software,  and to  permit  persons to  whom  the software  is
# furnished to do so, subject to the following conditions:
#
#        The above copyright notice and this permission notice shall be
#        included in all copies or substantial portions of the software.
#
# THE SOFTWARE  IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY  KIND, EXPRESS OR
# IMPLIED,  INCLUDING BUT  NOT LIMITED  TO THE  WARRANTIES  OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHOR  OR  COPYRIGHT HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# First edition by Urabe Shyouhei <shyouhei@netlab.jp> on 1 Sep., 2010.

# ARToolKit with Korundum/Qtbindings sample.

require 'ar'
require 'opengl'
require 'rubygems'
require 'Qt4'
require 'qtwebkit'

$a = Qt::Application.new ARGV
abort "usage: #$0 URL" if ARGV.empty?

class ARWebKit < Qt::WebPage
	Qt::WebSettings.global_settings.set_attribute Qt::WebSettings::AutoLoadImages, true
	Qt::WebSettings.global_settings.set_attribute Qt::WebSettings::JavascriptEnabled, true
	Qt::WebSettings.global_settings.set_attribute Qt::WebSettings::PluginsEnabled, true

	Manager = Qt::NetworkAccessManager.new $a
	c = Qt::NetworkDiskCache.new Manager
	l = Qt::DesktopServices.storage_location Qt::DesktopServices::CacheLocation
	c.set_cache_directory l
	Manager.set_cache c

	TextureSize = Qt::Size.new 512, 512
	WindowSize = Qt::Size.new 680, 512 # 4:3
	t = Qt::Color.new 0, 0, 0, 0
	Transparent = Qt::Brush.new t
	Signal1 = SIGNAL 'loadProgress(int)'
	Signal2 = SIGNAL 'loadFinished(bool)'
	slots 'rasterize(int)'
	slots 'rasterize(bool)'
	Slot1 = SLOT 'rasterize(int)'
	Slot2 = SLOT 'rasterize(bool)'

	attr_reader :image

	def initialize driver
		super nil
		@driver = driver
		connect self, Signal1, self, Slot1
		connect self, Signal2, self, Slot2
		set_network_access_manager Manager

		p = palette
		p.set_brush Qt::Palette::Base, Transparent
		set_viewport_size WindowSize
		set_palette p
		@image = Qt::Image.new TextureSize, Qt::Image::Format_ARGB32_Premultiplied
	end

	def load str
		u = Qt::Url.new str
		main_frame.load u
	end

	def rasterize _ = nil
		@image.fill Qt::transparent
		p = Qt::Painter.new @image
		p.set_render_hint Qt::Painter::Antialiasing
		p.set_render_hint Qt::Painter::TextAntialiasing
		p.set_render_hint Qt::Painter::SmoothPixmapTransform
		p.set_composition_mode Qt::Painter::CompositionMode_SourceOver
		p.scale 0.75, 1
		main_frame.render p
		p.end
		@driver.ping
	end
end

class ARWidget < Qt::GLWidget
	attr_reader :sizeHint

	def initialize
		super
		@web = ARWebKit.new self
		x, y = AR.video_inq_size
		@sizeHint = Qt::Size.new x, y
		resize sizeHint
		@patt = AR.load_patt "Data\\Patt.hiro"
		wparam = AR.param_load "Data\\camera_para.dat"
		@cparam = wparam.change_size x, y
		AR.init_cparam @cparam
		@repaint_request = false
	end

	def initializeGL
		@name, = GL.GenTextures 1
		@list = GL.GenLists 1
		GL.NewList @list, GL::COMPILE
			GL.Enable GL::BLEND
			GL.BlendFunc GL::SRC_ALPHA, GL::ONE_MINUS_SRC_ALPHA
			GL.Enable GL::TEXTURE_2D
			GL.BindTexture GL::TEXTURE_2D, @name
			GL.Begin GL::QUADS
				height = 4.0
				width = height / 3 * 4
				gap = 0.2
				[[[1, 0], [+width/2, 0, gap]],
				 [[1, 1], [+width/2, 0, gap + height]],
				 [[0, 1], [-width/2, 0, gap + height]],
				 [[0, 0], [-width/2, 0, gap]]].each do |((cx, cy), (px, py, pz))|
					GL.TexCoord2f cx, cy
					GL.Vertex3f px, py, pz
				end
			GL.End
		GL.EndList

		GL.Color4f 1, 1, 1, 0

		@camera_frustum = @cparam.camera_frustum_rh 0.1, 100
		@ctx = AR.gl_setup_for_current_context
		@web.load ARGV[0]
		start_timer 1000/15
	end

	def timerEvent ev
		updateGL
	end

	def resizeGL x, y
		GL.Viewport 0, 0, x, y
		GL.MatrixMode GL::PROJECTION
		GL.LoadIdentity
		GL.MatrixMode GL::MODELVIEW
		GL.LoadIdentity
	end

	def paintGL
		return unless image = AR.video_get_image
		GL.DrawBuffer GL::BACK
		GL.Clear GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT

		GL.Disable GL::BLEND
		@ctx.disp_image image, @cparam, 1.0
		AR.video_cap_next

		return unless marker_info = image.detect_marker(80)
		return unless k = marker_info.select {|i| i.id == @patt }.max_by {|i| i.cf }
		return unless trans = k.trans_mat([0, 0], 2, @hysteresis)
		m = trans.camera_view_rh 1
		@hysteresis = trans

		GL.MatrixMode GL::PROJECTION
		GL.LoadMatrix @camera_frustum
		GL.MatrixMode GL::MODELVIEW
		GL.LoadMatrix m

		GL.PushMatrix
		GL.CallList @list
		GL.PopMatrix
	end

	def ping
		i = Qt::GLWidget.convertToGLFormat @web.image
		GL.BindTexture GL::TEXTURE_2D, @name
		GL.PixelStore GL::UNPACK_ALIGNMENT, 4
		GL.TexImage2D GL::TEXTURE_2D, 0, GL::RGBA, i.width, i.height, 0, \
			GL::RGBA, GL::UNSIGNED_BYTE, i.bits
		GL.TexEnv GL::TEXTURE_ENV, GL::TEXTURE_ENV_MODE, GL::REPLACE #GL::DECAL
		GL.TexParameter GL::TEXTURE_2D, GL::TEXTURE_MIN_FILTER, GL::LINEAR
		GL.TexParameter GL::TEXTURE_2D, GL::TEXTURE_MAG_FILTER, GL::LINEAR
	end
end

AR.video_open "Data\\WDM_camera_flipV.xml" do
	ARWidget.new.show#_maximized
	AR.video_cap_start do
		$a.exec
	end
end

# Local Variables:
# mode: ruby
# coding: utf-8
# indent-tabs-mode: t
# tab-width: 3
# ruby-indent-level: 3
# fill-column: 79
# default-justification: full
# End:
# vi: ts=3 sw=3
