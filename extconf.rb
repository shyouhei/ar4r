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

require "mkmf"
dir_config "ARToolKit"
dir_config "OpenGL"
dir_config "GLUT"

# mandatory
have_header "AR/ar.h" or abort
have_library "libAR", "arGetVersion" or abort
have_header "AR/video.h" or abort
have_library "libARvideo", "arVideoOpen" or abort

# opengl
case RUBY_PLATFORM when /win32/, /cygwin/, /mingw32/
  have_library "opengl32" and
  have_library "glu32"
else
  have_library "Xi", "XAllowDeviceEvents" and
  have_library "Xext", "XMITMiscGetBugMode" and
  have_library "Xmu", "XmuAddCloseDisplayHook" and
  have_library "X11", "XOpenDisplay" and
  have_library "opengl" and
  have_library "glu"
end and
have_header "AR/gsub_lite.h" and
have_library "libARgsub_lite", "arglSetupForCurrentContext" and

# glut
case RUBY_PLATFORM when /win32/, /cygwin/, /mingw32/
  have_library "glut32"
else
  have_library "glut"
end and
have_header "AR/gsub.h" and
have_library "libARgsub", "argInit"

# mqo texture libs
have_header "jpeglib.h" and
have_library "jpeg"
have_header "png.h" and
have_library "png" and
have_header "zlib.h" and
have_library "z"

create_header
create_makefile "ar"
