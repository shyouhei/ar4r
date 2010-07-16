# Copyright(c) 2010 Network Applied Communication Lab.  All rights Reserved.
#
# Permission is hereby granted, free of  charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the code  without restriction, including without limitation  the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the software,  and to permit persons to whom the  software is furnished to
# do so, subject to the following conditions:
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

# First edition by Urabe Shyouhei <shyouhei@netlab.jp> on 11 Jul, 2010.

# ARToolKit with Ruby/SDL sample

require 'ar'
require 'opengl'
require 'sdl'

AR.video_open "Data\\WDM_camera_flipV.xml" do
	x, y = AR.video_inq_size
	patt = AR.load_patt "Data/patt.hiro"
	wparam = AR.param_load "Data/camera_para.dat"
	cparam = wparam.change_size x, y
	AR.init_cparam cparam
	SDL.init SDL::INIT_VIDEO
	SDL::GL.set_attr SDL::GL_RED_SIZE, 8
	SDL::GL.set_attr SDL::GL_GREEN_SIZE, 8
	SDL::GL.set_attr SDL::GL_BLUE_SIZE, 8
	SDL::GL.set_attr SDL::GL_DEPTH_SIZE, 8
	SDL::GL.set_attr SDL::GL_DOUBLEBUFFER,1
	SDL::Screen.open x, y, 32, SDL::OPENGL
	GL.Clear GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT
	GL.Viewport 0, 0, x, y
	GL.MatrixMode GL::PROJECTION
	GL.LoadIdentity
	GL.MatrixMode GL::MODELVIEW
	GL.LoadIdentity

	vertices = [
		[1, 1, 1], [1, -1, 1], [-1, -1, 1], [-1, 1, 1],
		[1, 1, -1], [1, -1, -1], [-1, -1, -1], [-1, 1, -1]
	]
	colours = [
		[1, 1, 1, 0.5], [1, 1, 0, 0.5], [0, 1, 0, 0.5], [0, 1, 1, 0.5],
		[1, 0, 1, 0.5], [1, 0, 0, 0.5], [0, 0, 0, 0.5], [0, 0, 1, 0.5]
	]
	faces = [
		[3, 2, 1, 0], [2, 3, 7, 6], [0, 1, 5, 4],
		[3, 0, 4, 7], [1, 2, 6, 5], [4, 5, 6, 7]
	]
	list = GL.GenLists 1
	GL.NewList list, GL::COMPILE
	GL.Begin GL::BLEND
	GL.BlendFunc GL::SRC_ALPHA, GL::ONE_MINUS_SRC_ALPHA
	GL.Begin GL::QUADS
	6.times do |f|
		4.times do |i|
			r, g, b, a = colours[faces[f][i]]
			x, y, z = vertices[faces[f][i]]
			GL.Color4f r, g, b, a
			GL.Vertex3f x * 0.5, y * 0.5, z * 0.5
		end
	end
	GL.End
	GL.End
	GL.Color3f 0, 0, 0
	6.times do |f|
		GL.Begin GL::LINE_LOOP
		4.times do |i|
			x, y, z = vertices[faces[f][i]]
			GL.Vertex3f x * 0.5, y * 0.5, z * 0.5
		end
		GL.End
	end
	GL.EndList

	AR.video_cap_start do
		x = cparam.camera_frustum_rh 0.1, 100
		ctx = AR.gl_setup_for_current_context
		loop do
			while ev = SDL::Event2.poll
				case ev when SDL::Event2::Quit, SDL::Event2::KeyDown
					exit
				end
			end
			next unless image = AR.video_get_image
			marker_info = image.detect_marker 100
			next unless k = marker_info.max_by {|i| i.cf }
			next unless trans = k.trans_mat([0, 0], 80)
			m = trans.camera_view_rh 0.025 # ???
			GL.DrawBuffer GL::BACK
			GL.Clear GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT

			ctx.disp_image image, cparam, 1.0
			AR.video_cap_next

			GL.MatrixMode GL::PROJECTION
			GL.LoadMatrix x
			GL.MatrixMode GL::MODELVIEW
			GL.LoadMatrix m

			GL.PushMatrix
			GL.Translate 0, 0, 0.5
			GL.CallList list
			GL.PopMatrix
			SDL::GL.swap_buffers
		end
	end
end
			
#
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
