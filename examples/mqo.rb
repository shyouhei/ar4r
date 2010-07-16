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

# First edition by Urabe Shyouhei <shyouhei@netlab.jp> on 16 Jul, 2010.

# Load a metasequoia data.

require 'optparse'
require 'ar'
require 'opengl'
require 'glut'

cparam_path = "Data/camera_para.dat"
patt_name   = "Data/patt.hiro"
vconf       = "Data/WDM_camera_flipV.xml"
scale       = 1 # AR unit / OpenGL unit
thresh      = 128 # binarization threshold
width       = 80

ARGV.options do |opt|
	opt.banner = "#$0 [opts ...] mqo_file_to_display"
	opt.on '--cparam=path', 'path to camera_para.dat' do |optarg|
		cparam_path = optarg
	end
	opt.on '--patt-name=path', 'path to a pattern file' do |optarg|
		patt_name = optarg
	end
	opt.on '--width=nn', 'pattern width in millimeters', Integer do |optarg|
		width = optarg
	end
	opt.on '--vconf=path', 'path to WDM_camera_flipV.xml' do |optarg|
		vconf = optarg
	end
	opt.parse!
end

GLUT.Init ARGV

AR.video_open vconf do
	pid = AR.load_patt patt_name
	x, y = AR.video_inq_size
	cparam = AR.param_load(cparam_path).change_size x, y
	AR.init_cparam cparam
	frustum = cparam.camera_frustum_rh 0.01, 1024
	mqo = image = found = matrix = nil

	GLUT.InitDisplayMode GLUT::DOUBLE | GLUT::RGBA | GLUT::DEPTH
	GLUT.InitWindowSize x, y
	GLUT.CreateWindow $0

	ctx = AR.gl_setup_for_current_context

	GLUT.VisibilityFunc proc {|v|
		p = GLUT::VISIBLE && proc do
			found = false
			next unless image = AR.video_get_image
			GLUT.PostRedisplay
			next unless m = image.detect_marker(thresh)
			next unless k = m.select{|i| i.id == pid }.max_by {|i| i.cf }
			next unless t = k.trans_mat([0, 0], width, matrix)
			found = true
			matrix = t
		end
		GLUT.IdleFunc p
	}

	GLUT.KeyboardFunc proc {|k, w, h| exit }

	GLUT.ReshapeFunc proc {|w, h|
		GL.Clear GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT
		GL.Viewport 0, 0, w, h	
		GL.MatrixMode GL::PROJECTION
		GL.LoadIdentity
		GL.MatrixMode GL::MODELVIEW
		GL.LoadIdentity
	}

	GLUT.DisplayFunc proc {
		next unless image
		begin
			GL.DrawBuffer GL::BACK
			GL.Clear GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT

			ctx.disp_image image, cparam, 1
			AR.video_cap_next
			next unless found

			GL.MatrixMode GL::PROJECTION
			GL.LoadMatrix frustum
			GL.MatrixMode GL::MODELVIEW
			GL.LoadIdentity

			GL.PushMatrix
				GL.Lightfv GL::LIGHT0, GL::DIFFUSE,  [0.9, 0.9, 0.9, 1.0]
				GL.Lightfv GL::LIGHT0, GL::SPECULAR, [1, 1, 1, 1]
				GL.Lightfv GL::LIGHT0, GL::AMBIENT,  [0.3, 0.3, 0.3, 0.1]
				GL.Lightfv GL::LIGHT0, GL::POSITION, [-100, 200, 200, 0]
				GL.Enable GL::LIGHTING
				GL.Enable GL::LIGHT0
			GL.PopMatrix

			GL.LoadMatrix matrix.camera_view_rh(scale)

			GL.Enable GL::DEPTH_TEST
			GL.DepthFunc GL::GREATER
			GL.ClearDepth 0

			GL.PushMatrix
				GL.Rotate 90, 1, 0, 0
				mqo.call
#				GL.Translate 0, 0, 0.5
#				GLUT.SolidTeapot 40
			GL.PopMatrix
		ensure
			GLUT.SwapBuffers
		end
	}

	AR.video_cap_start do
		MQO.init do
			mqo = MQO.new ARGV[0], scale
			GLUT.MainLoop
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
