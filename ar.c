/* Copyright(c) 2010 Network Applied Communication Lab.  All rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of  this software and  associated documentation  files (the  "Software"), to
 * deal in  the software without restriction, including  without limitation the
 * rights to use, copy,  modify, merge, publish, distribute, sublicense, and/or
 * sell copies of  the software, and to permit persons to  whom the software is
 * furnished to do so, subject to the following conditions:
 *
 *        The above copyright notice and this permission notice shall be
 *        included in all copies or substantial portions of the software.
 *
 * THE SOFTWARE IS  PROVIDED "AS IS", WITHOUT WARRANTY OF  ANY KIND, EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT  LIMITED TO  THE WARRANTIES  OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHOR  OR COPYRIGHT  HOLDERS  BE LIABLE  FOR  ANY CLAIM,  DAMAGES OR  OTHER
 * LIABILITY,  WHETHER IN  AN ACTION  OF CONTRACT,  TORT OR  OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Note however, the ARToolKit library  itself is licensed under either the GNU
 * General Public License or  another dedicated commercial license.  The author
 * of this  binding did  not read the  latter when  the author wrote  this, but
 * believes this binding can fit to  both.  This license is at least compatible
 * with the GPL.
 */

/* First edition by Urabe Shyouhei <shyouhei@netlab.jp> on 11 Jul, 2010. */

/**
 * @file ar.c
 * @brief ar4r main implementation.
 */

#include <ruby.h>
#include "extconf.h"
#include <AR/ar.h>
#include <AR/video.h>
#ifdef HAVE_AR_GSUB_LITE_H
#include <AR/gsub_lite.h>
#define GSUB_LITE 1
#endif
#ifdef HAVE_AR_GSUB_H
#include <AR/gsub.h>
#define GSUB 1
#endif

/**
 * @variable
 * @brief the class AR::Param
 */
static VALUE param = Qundef;

/**
 * @variable
 * @brief the class AR::GL_CONTEXT_SETTINGS_REF.
 */
static VALUE glcsr = Qundef;

/**
 * @variable
 * @brief the class AR::Image
 */
static VALUE image = Qundef;

/**
 * @variable
 * @brief the class AR::MarkerInfo
 */
static VALUE marker = Qundef;

/* :nodoc: */
static VALUE trans_mat = Qundef;

/**
 * @function
 * @brief wrapper for arLoadPatt
 * @param[in] ar not used
 * @param[in] fname the file to load
 * @returns a new instance of patten id
 */
static VALUE load_patt _((VALUE ar, VALUE fname));

/**
 * @function
 * @brief Get a new parameter from a file
 * @param[in] ar not used
 * @param[in] fname the file to load
 * @returns a new instance of AR::Param.
 */
static VALUE param_new _((VALUE ar, VALUE fname));

/**
 * @function
 * @brief Change resolution
 * @param[in] self old parameter
 * @param[in] x width
 * @param[in] y height
 * @returns a new instance of AR::Param.
 */
static VALUE param_change_size _((VALUE self, VALUE x, VALUE y));

/**
 * @function
 * @brief wrapper for arParamDisp
 * @param[in] self something to display
 * @returns self
 */
static VALUE param_disp _((VALUE self));

#ifdef GSUB_LITE
/**
 * @function
 * @brief wrapper for arglCameraFrustum.
 * @param[in] self the parameter
 * @param[in] focal_max max distance
 * @param[in] forcal_min min distance
 * @returns 4 x 4 matrix that can directly passed to GL::LoadMatrixd
 */
static VALUE camera_frustum _((VALUE self, VALUE focal_max, VALUE focal_min));
#endif

#ifdef GSUB_LITE
/**
 * @function
 * @brief wrapper for arglCameraView
 * @param[in] self the transformation matrix
 * @param[in] scale scale factor
 * @returns 4 x 4 matrix that can directly passed to GL::LoadMatrixd
 */
static VALUE camera_view _((VALUE self, VALUE scale));
#endif

/**
 * @function
 * @brief wrapper for arInitCparam
 * @param[in] ar not used at all
 * @param[in] cparam parameter to use
 * @returns ar
 */
static VALUE ar_init_cparam _((VALUE ar, VALUE cparam));

/**
 * @function
 * @brief Get a new context
 * @param[in] ar not used
 * @returns a new instance of klass.
 */
static VALUE glcsr_new _((VALUE ar));

/**
 * @function
 * @brief Cleanup.
 * @param[out] self this object
 */
static void glcsr_free _((void* self));

/**
 * @function
 * @brief wrapper for arVideoOpen()
 * @param[in] ar not used at all
 * @param[in] vconf path to a video configuration file
 * @returns ar
 */
static VALUE video_open _((VALUE ar, VALUE path));

/**
 * @function
 * @brief wrapper for arVideoClose()
 * @param[in] ar not used at all
 * @returns ar
 */
static VALUE video_close _((VALUE ar));

/**
 * @function
 * @brief wrapper for arVideoInqSize
 * @param[in] ar not used at all
 * @returns an array of size 2, first=xsize last=ysize
 */
static VALUE video_inq_size _((VALUE ar));

/**
 * @function
 * @brief wrapper for arVideoCapStart
 * @param[in] ar not used at all
 * @returns ar
 */
static VALUE video_cap_start _((VALUE ar));

/**
 * @function
 * @brief wrapper for arVideoCapStop
 * @param[in] ar not used at all
 * @returns ar
 */
static VALUE video_cap_stop _((VALUE ar));

/**
 * @function
 * @brief wrapper for arVideoDispOption
 * @param[in] ar not used at all
 * @returns ar
 */
static VALUE video_disp_option _((VALUE ar));

/**
 * @function
 * @brief wrapper for arVideoGetImage
 * @param[in] ar not used at all
 * @returns a new AR::Image
 */
static VALUE video_get_image _((VALUE ar));

/**
 * @function
 * @brief wrapper for arVideoCapNext
 * @param[in] ar not used at all
 * @returns ar
 */
static VALUE video_cap_next _((VALUE ar));

/**
 * @function
 * @brief wrapper for arDetectMarker
 * @param[in] self the image in question
 * @param[in] th binarization threshold 0-255
 * @returns an array of AR::MarkerInfo
 */
static VALUE detect_marker _((VALUE ar, VALUE th));

/**
 * @function
 * @brief wrapper for arGetTransMat
 * @param[in] self the marker info
 * @param[in] argc #of VALUEs in argv
 * @param[in] argv centre of pattern, width of pattern, history function if any
 * @returns 3 x 4 matrix.
 */
static VALUE get_trans_mat _((int argc, VALUE* argv, VALUE self));

#ifdef GSUB_LITE
/**
 * @function
 * @brief wrapper for arglDispImage
 * @param[in] ctx display context
 * @param[in] img image to display
 * @param[in] param parameters
 * @param[in] zoom zoom
 * @returns ctx.
 */
static VALUE gl_disp_image _((VALUE ctx, VALUE img, VALUE param, VALUE zoom));
#endif

#ifdef GSUB
/**
 * @function
 * @brief wrapper for argInit
 * @param[in] para display parameter
 * @param[in] zoom zoom factor
 * @param[in] fullp fullscreen mode
 * @param[in] x width
 * @param[in] y height
 * @param[in] flag ??
 * @returns para
 */
static VALUE g_init _((VALUE para, VALUE zoom, VALUE fullp,
                       VALUE x, VALUE y, VALUE flag));

/**
 * @function
 * @brief wrapper for argMainLoop
 * @param[in] ar not used at all
 * @returns ar
 */
static VALUE g_main_loop _((VALUE ar));

/**
 * @function
 * @brief wrapper for argSwapBuffers
 * @param[in] ar not used at all
 * @returns ar
 */
static VALUE g_swap_buffers _((VALUE ar));

/**
 * @function
 * @brief wrapper for argDrawMode2D
 * @param[in] ar not used at all
 * @returns ar
 */
static VALUE g_draw_mode_2d _((VALUE ar));

/**
 * @function
 * @brief wrapper for argDisppayImage
 * @param[in] img the image to display
 * @param[in] x ???
 * @param[in] y ???
 * @returns img
 */
static VALUE g_disp_image _((VALUE img, VALUE x, VALUE y));

/**
 * @function
 * @brief wrapper for argDraw3dCamera
 * @param[in] ar not used
 * @param[in] x rendering view width
 * @param[in] y rendering view height
 * @returns ar
 */
static VALUE g_draw_3d_camera _((VALUE ar, VALUE x, VALUE y));

/**
 * @function
 * @brief wrapper for argConvGlpara
 * @param[in] trans an AR::TransMat instance
 * @returns 4 x 4 matrix to pass to glLoadMatrix()
 */
static VALUE g_conv_gl_para _((VALUE trans));
#endif

/* internal use only. forget. */
#define accessor(x, y, z, w)                            \
static VALUE get_ ## x _((VALUE ar));                   \
static VALUE set_ ## x _((VALUE ar, VALUE val));        \
                                                        \
VALUE                                                   \
get_ ## x(ar)                                           \
    VALUE ar;                                           \
{                                                       \
    switch(ar ## y) {                                   \
    case z: return ID2SYM(rb_intern(# z));              \
    case w: return ID2SYM(rb_intern(# w));              \
    default: rb_raise(rb_eRuntimeError,                 \
                      "unknown value %d for ar" #y,     \
                      ar ## y);                         \
    }                                                   \
}                                                       \
                                                        \
VALUE                                                   \
set_ ## x(ar, val)                                      \
    VALUE ar, val;                                      \
{                                                       \
    ID sym = 0;                                         \
    if(rb_obj_is_kind_of(val, rb_cInteger)) {           \
        ar ## y = NUM2INT(val);                         \
        return val;                                     \
    }                                                   \
    else if(rb_obj_is_kind_of(val, rb_cSymbol)) {       \
        sym = SYM2ID(val);                              \
    }                                                   \
    else {                                              \
        const char* ptr = StringValueCStr(val);         \
        sym = rb_intern(ptr);                           \
    }                                                   \
    if(sym == rb_intern(#z))                            \
        ar ## y = z;                                    \
    else if(sym == rb_intern(#w))                       \
        ar ## y = w;                                    \
    else                                                \
        rb_raise(rb_eArgError,                          \
                 "wrong value for ar" #y ":%p",         \
                 val);                                  \
    return val;                                         \
}

accessor(fitting_mode, FittingMode,
         AR_FITTING_TO_INPUT, AR_FITTING_TO_IDEAL)
accessor(image_proc_mode, ImageProcMode,
         AR_IMAGE_PROC_IN_FULL, AR_IMAGE_PROC_IN_HALF)
accessor(template_matching_mode, TemplateMatchingMode,
         AR_TEMPLATE_MATCHING_COLOR, AR_TEMPLATE_MATCHING_BW)
accessor(matching_pca_mode, MatchingPCAMode,
         AR_MATCHING_WITHOUT_PCA, AR_MATCHING_WITH_PCA)
#undef accessor

VALUE
get_debug(ar)
    VALUE ar;
{
    return 2 * !!arDebug;
}

VALUE
set_debug(ar, val)
    VALUE ar, val;
{
    arDebug = RTEST(val);
    return val;
}

VALUE
load_patt(ar, fname)
    VALUE ar, fname;
{
    int i = arLoadPatt(StringValueCStr(fname));
    if(i < 0) rb_raise(rb_eRuntimeError, "arLoadPatt() failed");
    return INT2NUM(i);
}

VALUE
param_new(self, fname)
    VALUE self, fname;
{
    ARParam* r = ALLOC(ARParam);
    int i = arParamLoad(StringValueCStr(fname), 1, r);
    if(i < 0) rb_raise(rb_eRuntimeError, "arParamLoad() failed");
    return Data_Wrap_Struct(param, 0, -1, r);
}

VALUE
param_change_size(self, x, y)
    VALUE self, x, y;
{
    ARParam* p = ALLOC(ARParam);
    arParamChangeSize(DATA_PTR(self), NUM2INT(x), NUM2INT(y), p);
    return Data_Wrap_Struct(param, 0, -1, p);
}

VALUE
param_disp(self)
    VALUE self;
{
    arParamDisp(DATA_PTR(self));
    return self;
}

#ifdef GSUB_LITE
#define camera_frustum_(x, y)                                           \
VALUE                                                                   \
camera_frustum ## x(param, max, min)                                    \
    VALUE param, max, min;                                              \
{                                                                       \
    const ARParam* p = DATA_PTR(param);                                 \
    GLdouble proj[16];                                                  \
    VALUE elt[16];                                                      \
    int i;                                                              \
    arglCameraFrustum ## y(p, NUM2DBL(min), NUM2DBL(max), proj);        \
    for(i=0; i<16; i++)                                                 \
        elt[i] = rb_float_new(proj[i]);                                 \
    return rb_ary_new4(16, elt);                                        \
}
camera_frustum_(,)
camera_frustum_(_rh,RH)
#undef camera_frustm_
#endif

#ifdef GSUB_LITE
#define camera_view_(x, y)                              \
VALUE                                                   \
camera_view ## x(mat, sc)                               \
    VALUE mat, sc;                                      \
{                                                       \
    const void* para = DATA_PTR(mat);                   \
    GLdouble proj[16];                                  \
    VALUE elt[16];                                      \
    int i;                                              \
    arglCameraView ## y(para, proj, NUM2DBL(sc));       \
    for(i=0; i<16; i++)                                 \
        elt[i] =  rb_float_new(proj[i]);                \
    return rb_ary_new4(16, elt);                        \
}
camera_view_(,)
camera_view_(_rh, RH)
#endif

VALUE
ar_init_cparam(ar, cparam)
    VALUE ar, cparam;
{
    /* arInitCparam() always returns zero... */
    arInitCparam(DATA_PTR(cparam));
    return ar;
}

#ifdef GSUB_LITE
VALUE
glcsr_new(ar)
    VALUE ar;
{
    ARGL_CONTEXT_SETTINGS_REF r = arglSetupForCurrentContext();
    if(r) {
        return Data_Wrap_Struct(glcsr, 0, glcsr_free, r);
    }
    else {
        rb_raise(rb_eRuntimeError, "cannot arglSetupForCurrentContext");
    }
}

void
glcsr_free(self)
    void* self;
{
    arglCleanup(self);
}

/* The argl seriel API is totally brain-damagd. fxxk. */

VALUE
get_distortion_compensation(ctx)
    VALUE ctx;
{
    int i;
    arglDistortionCompensationGet(DATA_PTR(ctx), &i);
    return 2 * !!i;
}
VALUE
set_distortion_compensation(ctx, val)
    VALUE ctx, val;
{
    arglDistortionCompensationSet(DATA_PTR(ctx), RTEST(val));
    return val;
}

#define accessor(x, y, z, w)                                             \
static VALUE get_ ## x _((VALUE ar));                                   \
static VALUE set_ ## x _((VALUE ar, VALUE val));                        \
                                                                        \
VALUE                                                                   \
get_ ## x(ctx)                                                          \
    VALUE ctx;                                                          \
{                                                                       \
    int i = argl ## y ## Get(DATA_PTR(ctx));                            \
    switch(i) {                                                         \
    case z: return ID2SYM(rb_intern(#z));                               \
    case w: return ID2SYM(rb_intern(#w));                               \
    default:                                                            \
        rb_raise(rb_eRuntimeError,                                      \
                 "unknown value %d for arlg" #y "Get:", i);             \
    }                                                                   \
}                                                                       \
                                                                        \
VALUE                                                                   \
set_ ## x(ctx, val)                                                     \
    VALUE ctx, val;                                                     \
{                                                                       \
    ID sym = 0;                                                         \
    if(rb_obj_is_kind_of(val, rb_cInteger)) {                           \
        argl ## y ## Set(DATA_PTR(ctx), NUM2INT(val));                  \
        return val;                                                     \
    }                                                                   \
    else if(rb_obj_is_kind_of(val, rb_cSymbol)) {                       \
        sym = SYM2ID(val);                                              \
    }                                                                   \
    else {                                                              \
        const char* ptr = StringValueCStr(val);                         \
        sym = rb_intern(ptr);                                           \
    }                                                                   \
    if(sym == rb_intern(#z))                                            \
        arglDrawModeSet(DATA_PTR(ctx), z);                              \
    if(sym == rb_intern(#w))                                            \
        arglDrawModeSet(DATA_PTR(ctx), w);                              \
    else                                                                \
        rb_raise(rb_eArgError,                                          \
                 "wrong value for argl" #y "Set:%p", val);              \
    return val;                                                         \
}
accessor(draw_mode, DrawMode,
         AR_DRAW_BY_TEXTURE_MAPPING, AR_DRAW_BY_GL_DRAW_PIXELS)
accessor(texmap_mode, TexmapMode,
         AR_DRAW_TEXTURE_FULL_IMAGE, AR_DRAW_TEXTURE_HALF_IMAGE)
#undef accessor

VALUE
get_pixel_format(ctx)
    VALUE ctx;
{
    AR_PIXEL_FORMAT f;
    int bpp;
    if(arglPixelFormatGet(DATA_PTR(ctx), &f, &bpp)) {
        VALUE x;
        switch(f) {
#define b_(n) case AR_PIXEL_FORMAT_ ## n:                        \
            x = ID2SYM(rb_intern("AR_PIXEL_FORMAT_" #n));       \
            break
            b_(RGB);
            b_(BGR);
            b_(RGBA);
            b_(BGRA);
            b_(ABGR);
            b_(MONO);
            b_(ARGB);
            b_(UYVY);
            b_(YUY2);
#undef b_
        default:
            rb_raise(rb_eRuntimeError, "unknown pixel format %d", (int)f);
        }
        return rb_ary_new3(2, x, INT2NUM(bpp));
    }
    rb_raise(rb_eRuntimeError, "argPixelFormatGet error");
}

VALUE
set_pixel_format(ctx, fmt)
    VALUE ctx, fmt;
{
    ID x = 0;
    if(rb_obj_is_kind_of(fmt, rb_cInteger)) {
        arglPixelFormatSet(DATA_PTR(ctx), (AR_PIXEL_FORMAT)NUM2INT(fmt));
        return fmt;
    }
    else if(rb_obj_is_kind_of(fmt, rb_cSymbol)) {
        x = SYM2ID(fmt);
    }
    else {
        const char* ptr = StringValueCStr(fmt);
        x = rb_intern(ptr);
    }
    if(0) ; /* hack */
#define b_(n) else if(x == rb_intern("AR_PIXEL_FORMAT_" #n)) \
        arglPixelFormatSet(DATA_PTR(ctx), AR_PIXEL_FORMAT_ ## n)
    b_(RGB);
    b_(BGR);
    b_(RGBA);
    b_(BGRA);
    b_(ABGR);
    b_(MONO);
    b_(ARGB);
    b_(UYVY);
    b_(YUY2);
#undef b_
    else {
        rb_raise(rb_eArgError, "unknown format: %p", fmt);
    }
    return fmt;
}

VALUE
get_tex_rectangle(ctx)
    VALUE ctx;
{
    return 2 * !!arglTexRectangleGet(DATA_PTR(ctx));
}

VALUE
set_tex_rectangle(ctx, val)
    VALUE ctx, val;
{
    arglTexRectangleSet(DATA_PTR(ctx), RTEST(val));
    return val;
}
#endif

VALUE
video_open(ar, path)
    VALUE ar, path;
{
    if(arVideoOpen(StringValueCStr(path)) < 0)
        rb_raise(rb_eRuntimeError, "arVideoOpen() failed");
    if(rb_block_given_p())
        rb_ensure(rb_yield, ar, video_close, ar);
    return ar;
}

#define defsimple(x, y) VALUE x(ar) VALUE ar; { y(); return ar; }

defsimple(video_close, arVideoClose)

VALUE
video_inq_size(ar)
    VALUE ar;
{
    int x, y;
    if(arVideoInqSize(&x, &y) < 0)
        rb_raise(rb_eRuntimeError, "arVideoInqSize() failed");
    return rb_ary_new3(2, INT2FIX(x), INT2FIX(y));
}

VALUE
video_cap_start(ar)
    VALUE ar;
{
    if(arVideoCapStart() < 0)
        rb_raise(rb_eRuntimeError, "arVideoCapStart() failed");
    if(rb_block_given_p())
        rb_ensure(rb_yield, ar, video_cap_stop, ar);
    return ar;
}

defsimple(video_cap_stop, arVideoCapStop)
defsimple(video_disp_option, arVideoDispOption)

VALUE
video_get_image(ar)
    VALUE ar;
{
    ARUint8* ptr = arVideoGetImage();
    if(ptr) return Data_Wrap_Struct(image, 0, 0, ptr);
    else return Qnil;
}

VALUE
video_cap_next(ar)
    VALUE ar;
{
    if(arVideoCapNext() >= 0) return ar;
    rb_raise(rb_eRuntimeError, "arVideCapNext() failed");
}

VALUE
detect_marker(img, th)
    VALUE img, th;
{
    ARMarkerInfo* info;
    int i, num;
    VALUE ret;
    if(arDetectMarker(DATA_PTR(img), NUM2INT(th), &info, &num) < 0)
        rb_raise(rb_eRuntimeError, "arDetectMarker() failed");
    ret = rb_ary_new2(num);
    for(i=0; i<num; i++)
        rb_ary_store(ret, i, Data_Wrap_Struct(marker, 0, 0, &info[i]));
    return ret;
}

VALUE
marker_get_id(m)
    VALUE m;
{
    const ARMarkerInfo* i = DATA_PTR(m);
    return INT2NUM(i->id);
}

VALUE
marker_get_cf(m)
    VALUE m;
{
    const ARMarkerInfo* i = DATA_PTR(m);
    return rb_float_new(i->cf);
}

VALUE
marker_get_dir(m)
    VALUE m;
{
    const ARMarkerInfo* i = DATA_PTR(m);
    return INT2NUM(i->dir);
}

VALUE
marker_get_pos(m)
    VALUE m;
{
    const ARMarkerInfo* i = DATA_PTR(m);
    VALUE elt[2];
    int j;
    for(j=0; j<2; j++)
        elt[j] = rb_float_new(i->pos[j]);
    return rb_ary_new4(2, elt);
}

VALUE
marker_get_line(m)
    VALUE m;
{
    const ARMarkerInfo* i = DATA_PTR(m);
    VALUE elt[4][3];
    VALUE tmp[4];
    int j, k;
    for(j=0; j<4; j++)
        for(k=0; k<3; k++)
            elt[j][k] = rb_float_new(i->line[j][k]);
    for(j=0; j<4; j++)
        tmp[j] = rb_ary_new4(3, elt[j]);
    return rb_ary_new4(4, tmp);
}

VALUE
marker_get_vertex(m)
    VALUE m;
{
    const ARMarkerInfo* i = DATA_PTR(m);
    VALUE elt[4][2];
    VALUE tmp[4];
    int j, k;
    for(j=0; j<4; j++)
        for(k=0; k<2; k++)
            elt[j][k] = rb_float_new(i->vertex[j][k]);
    for(j=0; j<4; j++)
        tmp[j] = rb_ary_new4(2, elt[j]);
    return rb_ary_new4(4, tmp);
}

VALUE
get_trans_mat(argc, argv, marker)
    int argc;
    VALUE* argv;
    VALUE marker;
{
    ARMarkerInfo* m = DATA_PTR(marker);
    VALUE centre = Qnil, width = Qnil, hist = Qnil, c2 = Qnil;
    void* c1 = 0;
    double c3[2] = { 0, 0 };
    rb_scan_args(argc, argv, "21", &centre, &width, &hist);
    c2 = rb_ary_to_ary(centre);
    c1 = ALLOC_N(double, 12);   /* freed by GC */
    c3[0] = NUM2DBL(RARRAY_PTR(c2)[0]);
    c3[1] = NUM2DBL(RARRAY_PTR(c2)[1]);
    if(rb_obj_is_kind_of(hist, trans_mat)) {
        arGetTransMatCont(m, DATA_PTR(hist), c3, NUM2DBL(width), c1);
    }
    else {
        arGetTransMat(m, c3, NUM2DBL(width), c1);
    }
    return Data_Wrap_Struct(trans_mat, 0, -1, c1);
}

#ifdef GSUB_LITE
VALUE
gl_disp_image(ctx, img, pvx, zoom)
    VALUE ctx, img, pvx, zoom;
{
    const ARGL_CONTEXT_SETTINGS_REF c = DATA_PTR(ctx);
    if(img != Qnil &&!rb_obj_is_kind_of(img, image)) {
        rb_raise(rb_eTypeError, "AR::Image expected");
    }
    else {
        /* no const, according to the header... */
        ARUint8* i = img == Qnil? 0 : DATA_PTR(img);
        if(!rb_obj_is_kind_of(pvx, param)) {
            rb_raise(rb_eTypeError, "AR::Param expected");
        }
        else {
            const ARParam* p = DATA_PTR(pvx);
            arglDispImage(i, p, NUM2DBL(zoom), c);
        }
    }
    return ctx;
}
#endif

#ifdef GSUB
VALUE
g_init(self, z, f, x, y, flag)
    VALUE self, z, f, x, y, flag;
{
    ARParam* param = DATA_PTR(self);
    double zoom = NUM2DBL(z);
    int fullp = RTEST(f);
    int xwin = NUM2INT(x);
    int ywin = NUM2INT(y);
    int hmd_flag = RTEST(flag);
    argInit(param, zoom, fullp, xwin, ywin, hmd_flag);
    return self;
}

static void g_main_mouse _((int, int, int, int));
static void g_main_keyboard _((unsigned char, int, int));
static void g_main_visual _((void));

VALUE
g_main_loop(ar)
    VALUE ar;
{
    /* the design of  this loop is difeerent from  that of ruby-opengl becuase,
     * rb_block_proc/block call combination is hevier than an rb_yield call. */
    argMainLoop(g_main_mouse, g_main_keyboard, g_main_visual);
    return ar;
}

void
g_main_mouse(b, s, x, y)
    int b, s, x, y;
{
    rb_yield_values(5, ID2SYM(rb_intern("mouse")),
                    INT2NUM(b), INT2NUM(s), INT2NUM(x), INT2NUM(y));
}

void
g_main_keyboard(c, x, y)
    unsigned char c;
    int x, y;
{
    rb_yield_values(4, ID2SYM(rb_intern("key")),
                    INT2NUM(c), INT2NUM(x), INT2NUM(y));
}

void
g_main_visual(void)
{
    rb_yield(ID2SYM(rb_intern("visual")));
}

#define g_simple(x, y)\
    VALUE x(a) VALUE a; { y(); return a; }
g_simple(g_swap_buffers, argSwapBuffers)
g_simple(g_draw_mode_2d, argDrawMode2D)
g_simple(g_draw_mode_3d, argDrawMode3D)
#undef g_simple

VALUE
g_disp_image(img, x, y)
    VALUE img, x, y;
{
    argDispImage(DATA_PTR(img), NUM2INT(x), NUM2INT(y));
    return img;
}

VALUE
g_draw_3d_camera(ar, x, y)
    VALUE ar, x, y;
{
    argDraw3dCamera(NUM2INT(x), NUM2INT(y));
    return ar;
}

VALUE
g_conv_gl_para(trans)
    VALUE trans;
{
    void* para = DATA_PTR(trans);
    double proj[16];
    VALUE elt[16];
    int i;
    argConvGlpara(para, proj);
    for(i=0; i<16; i++)
        elt[i] = rb_float_new(proj[i]);
    return rb_ary_new4(16, elt);
}
#endif

extern void Init_mqo(void);

void
Init_ar(void)
{
    VALUE ar = rb_define_module("AR");
#define defconst(nam) rb_define_const(ar, #nam, INT2NUM(AR_ ## nam))
#define accessor(n)\
    rb_define_singleton_method(ar, #n, get_ ## n, 0);     \
    rb_define_singleton_method(ar, #n "=", set_ ## n, 1)
    accessor(fitting_mode);
    defconst(FITTING_TO_INPUT);
    defconst(FITTING_TO_IDEAL);

    accessor(image_proc_mode);
    defconst(IMAGE_PROC_IN_FULL);
    defconst(IMAGE_PROC_IN_HALF);

    accessor(template_matching_mode);
    defconst(TEMPLATE_MATCHING_COLOR);
    defconst(TEMPLATE_MATCHING_BW);

    accessor(matching_pca_mode);
    defconst(MATCHING_WITHOUT_PCA);
    defconst(MATCHING_WITH_PCA);

    accessor(debug);
#undef accessor
#define deffmt(nam) defconst(PIXEL_FORMAT_ ## nam)
    deffmt(RGB);
    deffmt(BGR);
    deffmt(RGBA);
    deffmt(BGRA);
    deffmt(ABGR);
    deffmt(MONO);
    deffmt(ARGB);
    deffmt(2vuy);
    deffmt(UYVY);
    deffmt(yuvs);
    deffmt(YUY2);
#undef deffmt

    rb_define_const(ar, "HEADER_VERSOIN",
                    rb_str_new2(AR_HEADER_VERSION_STRING));
    rb_define_singleton_method(ar, "load_patt", load_patt, 1);

#ifdef GSUB_LITE
    glcsr = rb_define_class_under(ar, "GL_CONTEXT_SETTINGS_REF", rb_cData);
    rb_undef_alloc_func(glcsr);
    rb_define_singleton_method(ar, "gl_setup_for_current_context", glcsr_new,
                               0);
    rb_define_method(glcsr, "disp_image", gl_disp_image, 3);
#define accessor(nam)                                     \
    rb_define_method(glcsr, #nam, get_ ## nam, 0);        \
    rb_define_method(glcsr, #nam "_get", get_ ## nam, 0); \
    rb_define_method(glcsr, #nam "=", set_ ## nam, 1);    \
    rb_define_method(glcsr, #nam "_set", set_ ## nam, 1)
    accessor(distortion_compensation);
    accessor(pixel_format);
    accessor(draw_mode);
    accessor(texmap_mode);
    accessor(tex_rectangle);
#undef accessor
    defconst(DRAW_BY_GL_DRAW_PIXELS);
    defconst(DRAW_BY_TEXTURE_MAPPING);
    defconst(DRAW_TEXTURE_HALF_IMAGE);
    defconst(DRAW_TEXTURE_FULL_IMAGE);
#endif

    param = rb_define_class_under(ar, "Param", rb_cData);
    rb_undef_alloc_func(param);
    rb_define_singleton_method(ar, "param_load", param_new, 1);
    rb_define_singleton_method(ar, "init_cparam", ar_init_cparam, 1);
    rb_define_method(param, "change_size", param_change_size, 2);
    rb_define_method(param, "disp", param_disp, 0);
#ifdef GSUB_LITE
    rb_define_method(param, "camera_frustum", camera_frustum, 2);
    rb_define_method(param, "camera_frustum_rh", camera_frustum_rh, 2);
#endif
#ifdef GSUB
    rb_define_method(param, "arg_init", g_init, 5);
#endif

    rb_define_singleton_method(ar, "video_open", video_open, 1);
    rb_define_singleton_method(ar, "video_close", video_close, 0);
    rb_define_singleton_method(ar, "video_inq_size", video_inq_size, 0);
    rb_define_singleton_method(ar, "video_cap_start", video_cap_start, 0);
    rb_define_singleton_method(ar, "video_cap_stop", video_cap_stop, 0);
    rb_define_singleton_method(ar, "video_disp_option", video_disp_option, 0);
    rb_define_singleton_method(ar, "video_get_image", video_get_image, 0);
    rb_define_singleton_method(ar, "video_cap_next", video_cap_next, 0);

    image = rb_define_class_under(ar, "Image", rb_cData);
    rb_undef_alloc_func(image);
    rb_define_method(image, "detect_marker", detect_marker, 1);
#ifdef GSUB
    rb_define_method(image, "arg_disp_image", g_disp_image, 2);
#endif

    marker = rb_define_class_under(ar, "MarkerInfo", rb_cData);
    rb_undef_alloc_func(marker);
    rb_define_method(marker, "trans_mat", get_trans_mat, -1);
    rb_define_method(marker, "id", marker_get_id, 0);
    rb_define_method(marker, "cf", marker_get_cf, 0);
    rb_define_method(marker, "dir", marker_get_dir, 0);
    rb_define_method(marker, "pos", marker_get_pos, 0);
    rb_define_method(marker, "line", marker_get_line, 0);
    rb_define_method(marker, "vertex", marker_get_vertex, 0);

    trans_mat = rb_define_class_under(ar, "TramsMat", rb_cData);
    rb_undef_alloc_func(trans_mat);
#ifdef GSUB_LITE
    rb_define_method(trans_mat, "camera_view", camera_view, 1);
    rb_define_method(trans_mat, "camera_view_rh", camera_view_rh, 1);
#endif
#ifdef GSUB
    rb_define_method(trans_mat, "arg_conv_gl_para", g_conv_gl_para, 0);
#endif

#ifdef GSUB
    rb_define_singleton_method(ar, "g_main_loop", g_main_loop, 0);
    rb_define_singleton_method(ar, "g_swap_buffers", g_swap_buffers, 0);
    rb_define_singleton_method(ar, "g_draw_mode_2d", g_draw_mode_2d, 0);
    rb_define_singleton_method(ar, "g_draw_mode_3d", g_draw_mode_3d, 0);
    rb_define_singleton_method(ar, "g_draw_3d_camera", g_draw_3d_camera, 2);
#endif
#undef defconst

    Init_mqo();
}

/*
 * Local Variables:
 * mode: C
 * coding: utf-8-unix
 * indent-tabs-mode: nil
 * tab-width: 8
 * fill-column: 79
 * default-justification: full
 * c-file-style: "Ruby"
 * c-doc-comment-style: javadoc
 * End:
 */
