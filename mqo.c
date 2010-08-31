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
 * @file mqo.c
 * @brief very tiny GLMetaseq wrapper
 */

#include <ruby.h>
#include "extconf.h"
#ifdef HAVE_JPEGLIB_H
#include <jpeglib.h>
#define D_JPEG
#endif
#ifdef HAVE_PNG_H
#include <png.h>
#define D_PNG
#endif
#include "GLMetaseq.h"

/**
 * @function
 * @brief wrapper for mqoInit
 * @param[in] self not used at all
 * @returns self.
 */
static VALUE m_init _((VALUE self));

/**
 * @function
 * @brief wrapper for mqoCleanup
 * @param[in] self not used at all
 * @returns self.
 */
static VALUE m_cleanup _((VALUE self));

/**
 * @function
 * @brief allocates a new instance
 * @param[in] klass the klass to associate
 * @returns a newly allocated klass instance
 */
static VALUE m_alloc _((VALUE klass));

/**
 * @function
 * @brief deallocates an instance
 * @param[in] ptr memory region to terminate
 */
static void m_dealloc _((void* ptr));

/**
 * @function
 * @brief wrapper for mqoCreateModel
 * @param[out] self object to load into
 * @param[in] path path to read file form
 * @param[in] zoom zoom factor
 * @returns self
 */
static VALUE m_create_model _((VALUE self, VALUE path, VALUE zoom));

/**
 * @function
 * @brief wrapper for mqoCallModel
 * @param[in] self object to call
 * @returns self
 */
static VALUE m_call_model _((VALUE self));

VALUE
m_init(self)
    VALUE self;
{
    mqoInit();
    if(rb_block_given_p())
        rb_ensure(rb_yield, self, m_cleanup, self);
    return self;
}

VALUE
m_cleanup(self)
    VALUE self;
{
    mqoCleanup();
    return self;
}

VALUE
m_alloc(klass)
    VALUE klass;
{
    /* Fake, because resource allocation should be done from initialize. */
    return Data_Wrap_Struct(klass, 0, m_dealloc, 0);
}

void
m_dealloc(ptr)
    void* ptr;
{
    if(ptr)
        mqoDeleteModel(ptr);
}

VALUE
m_create_model(self, path, zoom)
    VALUE self, path, zoom;
{
    if(DATA_PTR(self))
        rb_raise(rb_eArgError, "initialize twice");
    else
        DATA_PTR(self) = mqoCreateModel(StringValueCStr(path), NUM2DBL(zoom));
    return self;
}

VALUE
m_call_model(self)
    VALUE self;
{
    if(!DATA_PTR(self))
        rb_raise(rb_eArgError, "unlinitialized");
    mqoCallModel(DATA_PTR(self));
    return self;
}

Init_mqo(void)
{
    VALUE m = rb_define_class("MQO", rb_cData);
    rb_define_singleton_method(m, "init", m_init, 0);
    rb_define_singleton_method(m, "cleanup", m_cleanup, 0);
    rb_define_alloc_func(m, m_alloc);
    rb_define_method(m, "initialize", m_create_model, 2);
    rb_define_method(m, "call", m_call_model, 0);
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
