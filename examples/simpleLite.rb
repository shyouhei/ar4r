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

# First edition by Urabe Shyouhei <shyouhei@netlab.jp> on 11 Jul, 2010.

# This is an (almost) literal translation of ARToolKit's example/simpleLite.
# So this is not a fluent Ruby code.  A more sophisticated one is mqo.rb.

require 'ar'
require 'opengl'
require 'glut'

VIEW_SCALEFACTOR = 0.025
VIEW_DISTANCE_MIN = 0.1
VIEW_DISTANCE_MAX = 100.0

$pref_windowed = true
$pref_width = 640
$pref_height = 480
$pref_depth = 32
$pref_refresh = 0

$image = nil
$threshold = 100
$count = 0

$width = 80
$centre = [0, 0]
$found = false
$draw_rotate = false
$draw_angle = 0

CubeVertices = [
	[1.0, 1.0, 1.0], [1.0, -1.0, 1.0], [-1.0, -1.0, 1.0], [-1.0, 1.0, 1.0],
	[1.0, 1.0, -1.0], [1.0, -1.0, -1.0], [-1.0, -1.0, -1.0], [-1.0, 1.0, -1.0]
]
CubeColours = [
	[1.0, 1.0, 1.0], [1.0, 1.0, 0.0], [0.0, 1.0, 0.0], [0.0, 1.0, 1.0],
	[1.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 0.0, 0.0], [0.0, 0.0, 1.0]
]
CubeFaces = [
	[3, 2, 1, 0], [2, 3, 7, 6], [0, 1, 5, 4],
	[3, 0, 4, 7], [1, 2, 6, 5], [4, 5, 6, 7]
]
$poly_list = nil

def draw_cube
	unless $poly_list
		$poly_list = GL.GenLists 1
		GL.NewList $poly_list, GL::COMPILE
		GL.Begin GL::QUADS
		6.times do |f|
			4.times do |i|
				r, g, b = CubeColours[CubeFaces[f][i]]
				x, y, z = CubeVertices[CubeFaces[f][i]]
				GL.Color3f r, g, b
				GL.Vertex3f x * 0.5, y * 0.5, z * 0.5
			end
		end
		GL.End
		GL.Color3f 0, 0, 0
		6.times do |f|
			GL.Begin GL::LINE_LOOP
			4.times do |i|
				x, y, z = CubeVertices[CubeFaces[f][i]]
				GL.Vertex3f x * 0.5, y * 0.5, z * 0.5
			end
			GL.End
		end
		GL.EndList
	end

	GL.PushMatrix
	GL.Translate 0, 0, 0.5
	GL.Rotate $draw_angle, 0, 0, 1
	GL.Disable GL::LIGHTING
	GL.CallList $poly_list
	GL.PopMatrix
end

def draw_cube_update delta
	if $draw_rotate
		$draw_angle += delta * 45.0
		$draw_angle -= 360 if $draw_angle > 360
	end
end

def setup_camera cparam_name, vconf
	AR.video_open vconf do
		x, y = AR.video_inq_size
		wparam = AR.param_load cparam_name
		cparam = wparam.change_size x, y
		cparam.disp
		AR.init_cparam cparam
		AR.video_cap_start do
			yield cparam
		end
	end
end

def setup_marker patt_name
	return AR.load_patt patt_name
end

def debug_report_mode ctx
	z = AR.fitting_mode
	x = AR.image_proc_mode
	c = ctx.draw_mode
	m = AR.template_matching_mode
	p = AR.matching_pca_mode
	STDERR.printf <<-end, z, x, c, m, p
         FittingMode (Z) : %s
            ProcMode (X) : %s
            DrawMode (C) : %s
TemplateMatchingMode (M) : %s
     MatchingPCAMode (P) : %s
	end
end

def quit
	# cleanup done almost automatically.
	exit 0
end

def keyboard key, x, y
	case key
	when 0x1B, ?Q, ?q
		quit
	when ?\s # space
		$draw_rotate = ! $draw_rotate
	when ?C, ?c
		if $ctx.draw_mode == :AR_DRAW_BY_GL_DRAW_PIXELS
			$ctx.draw_mode = :AR_DRAW_BY_TEXTURE_MAPPING
			$ctx.texmap_mode = :AR_DRAW_TEXTURE_FULL_IMAGE
		elsif $ctx.texmap_mode == :AR_DRAW_TEXTURE_FULL_IMAGE
			$ctx.texmap_mode = :AR_DRAW_TEXTURE_HALF_IMAGE
		else
			$ctx.draw_mode = :AR_DRAW_BY_GL_DRAW_PIXELS
		end
		t = Time.now
		STDERR.printf "*** Camera - %f (frame/sec)\n",
			$count / (t - ($t || Time.at(0))).to_f
		$count = 0
		$t = t
		debug_report_mode $ctx
	when ?D, ?d
		AR.debug = ! AR.debug
	when ??, ?/
		puts <<-end
Keys:
 q or [esc]    Quit demo.
 c             Change arglDrawMode and arglTexmapMode.
 d             Activate / deactivate debug mode.
 ? or /        Show this help.

Additionally, the ARVideo library supplied the following help text:
		end
		AR.video_disp_option
	end
end

def idle
	$ms_prev ||= 0
	ms = GLUT.Get GLUT::ELAPSED_TIME
	s_elapsed = (ms - $ms_prev) * 0.001
	return if s_elapsed < 0.01
	$ms_prev = ms

	draw_cube_update(s_elapsed);
	if image = AR.video_get_image
		$image = image
		$count += 1
		marker_info = image.detect_marker $threshold
		if k = marker_info.max_by {|i| i.cf }
			$trans = k.trans_mat $centre, $width
			$found = true
		else
			$found = false
		end
		GLUT.PostRedisplay
	end
end

def visibility visible
	if visible == GLUT::VISIBLE
		GLUT::IdleFunc proc { idle }
	else
		GLUT::IdleFunc nil
	end
end

def reshape w, h
	GL.Clear GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT
	GL.Viewport 0, 0, w, h	
	GL.MatrixMode GL::PROJECTION
	GL.LoadIdentity
	GL.MatrixMode GL::MODELVIEW
	GL.LoadIdentity
end

def display
	GL.DrawBuffer GL::BACK
	GL.Clear GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT
	
	$ctx.disp_image $image, $param, 1.0
	AR.video_cap_next
	$image = nil
				
	unless $camera_frustum
		x = $param.camera_frustum_rh VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX
		$camera_frustum = x
	end
	GL.MatrixMode GL::PROJECTION
	GL.LoadMatrix $camera_frustum
	GL.MatrixMode GL::MODELVIEW
		
	GL.LoadIdentity

	if $found
		m = $trans.camera_view_rh VIEW_SCALEFACTOR
		GL.LoadMatrix m

		draw_cube
	end
	GLUT.SwapBuffers
end

GLUT.Init(*ARGV)
setup_camera "Data/camera_para.dat", "Data\\WDM_camera_flipV.xml" do
	|cparam|
	$param = cparam
	GLUT.InitDisplayMode GLUT::DOUBLE | GLUT::RGBA | GLUT::DEPTH
	if !$pref_windowed
		mode = sprinrf "%ix%i:%i", $pref_width, $pref_eight, $pref_depth
		mode += sprintf "@%i", $pref_refresh if $pref_refresh.nonzero?
		GLUT.GameModeStrimg mode
		GLUT.EnterGameMode
	else
		GLUT.InitWindowSize $pref_width, $pref_height
		GLUT.CreateWindow $0
	end
	$ctx = AR.gl_setup_for_current_context
	debug_report_mode $ctx
	setup_marker "Data/patt.hiro"

	GLUT.DisplayFunc proc { display }
	GLUT.ReshapeFunc proc {|w, h| reshape w, h }
	GLUT.VisibilityFunc proc {|x| visibility x }
	GLUT.KeyboardFunc proc {|k, x, y| keyboard k, x, y }

	GLUT.MainLoop
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
