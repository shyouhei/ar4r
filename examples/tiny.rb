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

# First edition by Urabe Shyouhei <shyouhei@netlab.jp> on 12 Jul, 2010.

# This is an abetnent tool to say "Hey! AR can be this simple in Ruby!"

require 'ar'
require 'glut'
require 'opengl'

GLUT.Init(*ARGV)
AR.video_open "Data\\WDM_camera_flipV.xml" do
	patt_id = AR.load_patt "Data\\patt.hiro"
	x, y = AR.video_inq_size
	cparam = AR.param_load("Data\\camera_para.dat").change_size x, y
	AR.init_cparam cparam
	cparam.arg_init 1, false, 0, 0, false
	AR.video_cap_start do
		AR.g_main_loop do
			next unless img = AR.video_get_image
			begin
				AR.g_draw_mode_2d
				img.arg_disp_image 0, 0
				AR.video_cap_next
				next unless m = img.detect_marker(100)
				next unless k = m.max_by {|i| i.cf }
				next unless t = k.trans_mat([0, 0], 80)
				AR.g_draw_mode_3d
				AR.g_draw_3d_camera 0, 0
				GL.MatrixMode GL::MODELVIEW
				GL.LoadMatrix t.arg_conv_gl_para
				GL.Translate 0, 0, 25.0
				GL.Rotate 90, 1, 0, 0
				GLUT.WireTeapot 50.0
			ensure
				AR.g_swap_buffers
			end
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
