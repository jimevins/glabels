/*
 *  label-image.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "label-image.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#include "pixbuf-util.h"
#include "file-util.h"
#include "pixmaps/checkerboard.xpm"

#include "debug.h"


#define MIN_IMAGE_SIZE 1.0


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef enum {
        FILE_TYPE_NONE,
        FILE_TYPE_PIXBUF,
        FILE_TYPE_SVG
} FileType;


struct _glLabelImagePrivate {

        glTextNode       *filename;

        FileType          type;

        GdkPixbuf        *pixbuf;
        RsvgHandle       *svg_handle;
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GdkPixbuf *default_pixbuf = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_image_finalize      (GObject           *object);

static void copy                         (glLabelObject     *dst_object,
                                          glLabelObject     *src_object);

static void set_size                     (glLabelObject     *object,
                                          gdouble            w,
                                          gdouble            h,
                                          gboolean           checkpoint);

static void draw_object                  (glLabelObject     *object,
                                          cairo_t           *cr,
                                          gboolean           screen_flag,
                                          glMergeRecord     *record);

static void draw_shadow                  (glLabelObject     *object,
                                          cairo_t           *cr,
                                          gboolean           screen_flag,
                                          glMergeRecord     *record);

static gboolean object_at                (glLabelObject     *object,
                                          cairo_t           *cr,
                                          gdouble            x_pixels,
                                          gdouble            y_pixels);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelImage, gl_label_image, GL_TYPE_LABEL_OBJECT)


static void
gl_label_image_class_init (glLabelImageClass *class)
{
        GObjectClass       *object_class       = G_OBJECT_CLASS (class);
        glLabelObjectClass *label_object_class = GL_LABEL_OBJECT_CLASS (class);
        GdkPixbuf          *pixbuf;

        gl_label_image_parent_class = g_type_class_peek_parent (class);

        label_object_class->copy              = copy;
        label_object_class->set_size          = set_size;
        label_object_class->draw_object       = draw_object;
        label_object_class->draw_shadow       = draw_shadow;
        label_object_class->object_at         = object_at;

        object_class->finalize = gl_label_image_finalize;

        if ( default_pixbuf == NULL ) {
                pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)checkerboard_xpm);
                default_pixbuf =
                        gdk_pixbuf_scale_simple (pixbuf, 128, 128, GDK_INTERP_NEAREST);
                g_object_unref (pixbuf);
        }
}


static void
gl_label_image_init (glLabelImage *this)
{
        this->priv = g_new0 (glLabelImagePrivate, 1);

        this->priv->filename = g_new0 (glTextNode, 1);

        this->priv->type       = FILE_TYPE_NONE;
        this->priv->pixbuf     = NULL;
        this->priv->svg_handle = NULL;
}


static void
gl_label_image_finalize (GObject *object)
{
        glLabelObject *lobject = GL_LABEL_OBJECT (object);
        glLabelImage  *this  = GL_LABEL_IMAGE (object);
        glLabel       *label;
        GHashTable    *cache;

        g_return_if_fail (object && GL_IS_LABEL_IMAGE (object));

        if (!this->priv->filename->field_flag) {
                
                label = gl_label_object_get_parent (lobject);

                switch ( this->priv->type )
                {

                case FILE_TYPE_PIXBUF:
                        cache = gl_label_get_pixbuf_cache (label);
                        gl_pixbuf_cache_remove_pixbuf (cache, this->priv->filename->data);
                        break;

                case FILE_TYPE_SVG:
                        cache = gl_label_get_svg_cache (label);
                        gl_svg_cache_remove_svg (cache, this->priv->filename->data);
                        break;

                default:
                        break;

                }

        }
        gl_text_node_free (&this->priv->filename);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_label_image_parent_class)->finalize (object);
}


/*****************************************************************************/
/* NEW label "image" object.                                                 */
/*****************************************************************************/
GObject *
gl_label_image_new (glLabel *label,
                    gboolean checkpoint)
{
        glLabelImage *this;

        this = g_object_new (gl_label_image_get_type(), NULL);

        if (label != NULL)
        {
                if ( checkpoint )
                {
                        gl_label_checkpoint (label, _("Create image object"));
                }

                gl_label_add_object (label, GL_LABEL_OBJECT (this));
                gl_label_object_set_parent (GL_LABEL_OBJECT (this), label);
        }

        return G_OBJECT (this);
}


/*****************************************************************************/
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
        glLabelImage     *src_limage = (glLabelImage *)src_object;
        glLabelImage     *new_limage = (glLabelImage *)dst_object;
        glTextNode       *filename;
        GdkPixbuf        *pixbuf;
        gchar            *contents;
        glLabel          *src_label, *dst_label;
        GHashTable       *cache;

        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (src_limage && GL_IS_LABEL_IMAGE (src_limage));
        g_return_if_fail (new_limage && GL_IS_LABEL_IMAGE (new_limage));

        filename = gl_label_image_get_filename (src_limage);

        /* Make sure destination label has data suitably cached. */
        if ( !filename->field_flag && (src_limage->priv->type != FILE_TYPE_NONE) )
        {
                src_label = gl_label_object_get_parent (src_object);
                dst_label = gl_label_object_get_parent (dst_object);

                switch ( src_limage->priv->type )
                {

                case FILE_TYPE_PIXBUF:
                        pixbuf = src_limage->priv->pixbuf;
                        if ( pixbuf != NULL ) {
                                cache = gl_label_get_pixbuf_cache (dst_label);
                                gl_pixbuf_cache_add_pixbuf (cache, filename->data, pixbuf);
                        }
                        break;

                case FILE_TYPE_SVG:
                        cache = gl_label_get_svg_cache (src_label);
                        contents = gl_svg_cache_get_contents (cache, filename->data);
                        if ( contents != NULL ) {
                                cache = gl_label_get_svg_cache (dst_label);
                                gl_svg_cache_add_svg (cache, filename->data, contents);
                                g_free (contents);
                        }
                        break;

                default:
                        break;

                }
        }

        gl_label_image_set_filename (new_limage, filename, FALSE);
        gl_text_node_free (&filename);

        gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set size method.                                                */
/*---------------------------------------------------------------------------*/
static void
set_size (glLabelObject *object,
          gdouble        w,
          gdouble        h,
          gboolean       checkpoint)
{
        g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        if (w < MIN_IMAGE_SIZE)
        {
                w = MIN_IMAGE_SIZE;
        }

        if (h < MIN_IMAGE_SIZE)
        {
                h = MIN_IMAGE_SIZE;
        }

        GL_LABEL_OBJECT_CLASS (gl_label_image_parent_class)->set_size (object, w, h, checkpoint);
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_image_set_filename (glLabelImage *this,
                             glTextNode   *filename,
                             gboolean      checkpoint)
{
        glTextNode        *old_filename;
        glLabel           *label;
        GHashTable        *pixbuf_cache;
        GHashTable        *svg_cache;
        GdkPixbuf         *pixbuf;
        RsvgHandle        *svg_handle;
        RsvgDimensionData  svg_dim;
        gdouble            image_w, image_h, aspect_ratio, w, h;

        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (this && GL_IS_LABEL_IMAGE (this));
        g_return_if_fail (filename != NULL);

        old_filename = this->priv->filename;

        /* If Unchanged don't do anything */
        if ( gl_text_node_equal (filename, old_filename ) ) {
                return;
        }

        label = gl_label_object_get_parent (GL_LABEL_OBJECT (this));
        pixbuf_cache = gl_label_get_pixbuf_cache (label);
        svg_cache    = gl_label_get_svg_cache (label);

        if ( checkpoint )
        {
                gl_label_checkpoint (label, _("Set image"));
        }

        /* Set new filename. */
        this->priv->filename = gl_text_node_dup(filename);

        /* Remove reference to previous item. */
        switch (this->priv->type)
        {

        case FILE_TYPE_PIXBUF:
                if ( !old_filename->field_flag && (old_filename->data != NULL) ) {
                        gl_pixbuf_cache_remove_pixbuf (pixbuf_cache, old_filename->data);
                }
                break;

        case FILE_TYPE_SVG:
                if ( !old_filename->field_flag && (old_filename->data != NULL) ) {
                        gl_svg_cache_remove_svg (svg_cache, old_filename->data);
                }
                break;

        default:
                break;

        }

        gl_text_node_free (&old_filename);


        /* Now set the new file type and the pixbuf or svg_handle. */
        if ( !filename->field_flag && (filename->data != NULL) )
        {

                if ( gl_file_util_is_extension (filename->data, ".svg") )
                {
                        svg_handle = gl_svg_cache_get_handle (svg_cache, filename->data);

                        if (svg_handle != NULL)
                        {
                                this->priv->type       = FILE_TYPE_SVG;
                                this->priv->pixbuf     = NULL;
                                this->priv->svg_handle = svg_handle;
                        }
                        else
                        {
                                this->priv->type       = FILE_TYPE_NONE;
                                this->priv->pixbuf     = NULL;
                                this->priv->svg_handle = NULL;
                        }

                }
                else
                {

                        pixbuf = gl_pixbuf_cache_get_pixbuf (pixbuf_cache, filename->data);

                        if (pixbuf != NULL)
                        {
                                this->priv->type       = FILE_TYPE_PIXBUF;
                                this->priv->pixbuf     = pixbuf;
                                this->priv->svg_handle = NULL;
                        }
                        else
                        {
                                this->priv->type       = FILE_TYPE_NONE;
                                this->priv->pixbuf     = NULL;
                                this->priv->svg_handle = NULL;
                        }

                }
        }
        else
        {
                this->priv->type       = FILE_TYPE_NONE;
                this->priv->pixbuf     = NULL;
                this->priv->svg_handle = NULL;
        }


        /* Treat current size as a bounding box, scale image to maintain aspect
         * ratio while fitting it in this bounding box. */
        switch (this->priv->type)
        {

        case FILE_TYPE_PIXBUF:
                image_w = gdk_pixbuf_get_width (this->priv->pixbuf);
                image_h = gdk_pixbuf_get_height (this->priv->pixbuf);
                break;

        case FILE_TYPE_SVG:
                rsvg_handle_get_dimensions (this->priv->svg_handle, &svg_dim);
                image_w = svg_dim.width;
                image_h = svg_dim.height;
                break;

        default:
                image_w = gdk_pixbuf_get_width (default_pixbuf);
                image_h = gdk_pixbuf_get_height (default_pixbuf);
                break;

        }
        aspect_ratio = image_h / image_w;
        gl_label_object_get_size (GL_LABEL_OBJECT(this), &w, &h);
        if ( h > w*aspect_ratio ) {
                h = w * aspect_ratio;
        } else {
                w = h / aspect_ratio;
        }
        gl_label_object_set_size (GL_LABEL_OBJECT(this), w, h, FALSE);

        gl_label_object_emit_changed (GL_LABEL_OBJECT(this));

        gl_debug (DEBUG_LABEL, "END");
}


void
gl_label_image_set_pixbuf (glLabelImage  *this,
                           GdkPixbuf     *pixbuf,
                           gboolean       checkpoint)
{
        glTextNode  *old_filename;
        glLabel     *label;
        GHashTable  *pixbuf_cache;
        GHashTable  *svg_cache;
        gchar       *cs;
        gchar       *name;
        gdouble      image_w, image_h;

        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (this && GL_IS_LABEL_IMAGE (this));
        g_return_if_fail (pixbuf && GDK_IS_PIXBUF (pixbuf));

        old_filename = this->priv->filename;

        label = gl_label_object_get_parent (GL_LABEL_OBJECT (this));

        if ( checkpoint )
        {
                gl_label_checkpoint (label, _("Set image"));
        }

        pixbuf_cache = gl_label_get_pixbuf_cache (label);
        svg_cache = gl_label_get_svg_cache (label);

        /* Remove reference to previous item. */
        switch (this->priv->type)
        {

        case FILE_TYPE_PIXBUF:
                if ( !old_filename->field_flag && (old_filename->data != NULL) ) {
                        gl_pixbuf_cache_remove_pixbuf (pixbuf_cache, old_filename->data);
                }
                break;

        case FILE_TYPE_SVG:
                if ( !old_filename->field_flag && (old_filename->data != NULL) ) {
                        gl_svg_cache_remove_svg (svg_cache, old_filename->data);
                }
                break;

        default:
                break;

        }

        /* Set new filename. */
        cs = g_compute_checksum_for_data (G_CHECKSUM_MD5,
                                          gdk_pixbuf_get_pixels (pixbuf),
                                          gdk_pixbuf_get_rowstride (pixbuf)*gdk_pixbuf_get_height (pixbuf));
        name = g_strdup_printf ("%s.bitmap", cs);
        this->priv->filename = gl_text_node_new_from_text(name);
        gl_text_node_free (&old_filename);

        this->priv->pixbuf = g_object_ref (pixbuf);
        gl_pixbuf_cache_add_pixbuf (pixbuf_cache, name, pixbuf);

        g_free (cs);
        g_free (name);

        this->priv->type       = FILE_TYPE_PIXBUF;
        this->priv->svg_handle = NULL;

        image_w = gdk_pixbuf_get_width (this->priv->pixbuf);
        image_h = gdk_pixbuf_get_height (this->priv->pixbuf);
        gl_label_object_set_size (GL_LABEL_OBJECT(this), image_w, image_h, FALSE);

        gl_label_object_emit_changed (GL_LABEL_OBJECT(this));

        gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
GdkPixbuf *
gl_label_image_get_pixbuf (glLabelImage  *this,
                           glMergeRecord *record)
{
        g_return_val_if_fail (this && GL_IS_LABEL_IMAGE (this), NULL);

        if ((record != NULL) && this->priv->filename->field_flag)
        {

                GdkPixbuf   *pixbuf = NULL;
                gchar       *real_filename;

                /* Indirect filename, re-evaluate for given record. */

                real_filename = gl_merge_eval_key (record,
						   this->priv->filename->data);

                if (real_filename != NULL)
                {
                        pixbuf = gdk_pixbuf_new_from_file (real_filename, NULL);
                }
                return pixbuf;
        }

        if ( this->priv->type == FILE_TYPE_PIXBUF )
        {
                return g_object_ref (this->priv->pixbuf);
        }
        else
        {
                return NULL;
        }
}


RsvgHandle *
gl_label_image_get_svg_handle (glLabelImage  *this,
                               glMergeRecord *record)
{
	g_return_val_if_fail (this && GL_IS_LABEL_IMAGE (this), NULL);

	if ((record != NULL) && this->priv->filename->field_flag)
        {

		RsvgHandle  *svg_handle = NULL;
		gchar       *real_filename;

		/* Indirect filename, re-evaluate for given record. */

		real_filename = gl_merge_eval_key (record,
						   this->priv->filename->data);

		if (real_filename != NULL)
                {
                        if ( gl_file_util_is_extension (real_filename, ".svg") )
                        {
                                svg_handle = rsvg_handle_new_from_file (real_filename, NULL);
                        }
		}
                return svg_handle;
	}

        if ( this->priv->type == FILE_TYPE_SVG )
        {
                return g_object_ref (this->priv->svg_handle);
        }
        else
        {
                return NULL;
        }
}


static FileType
get_type (glLabelImage  *this,
          glMergeRecord *record)
{
	g_return_val_if_fail (this && GL_IS_LABEL_IMAGE (this), FALSE);

	if ((record != NULL) && this->priv->filename->field_flag)
        {
		gchar       *real_filename;

		real_filename = gl_merge_eval_key (record,
						   this->priv->filename->data);

                if ( gl_file_util_is_extension (real_filename, ".svg") )
                {
                        return FILE_TYPE_SVG;
                }
                else
                {
                        /* Assume a pixbuf compat file.  If not, queries for
                           pixbufs should return NULL and do the right thing. */
                        return FILE_TYPE_PIXBUF;
                }
        }
        else
        {
                return (this->priv->type);
        }
}


glTextNode *
gl_label_image_get_filename (glLabelImage *this)
{
        g_return_val_if_fail (this && GL_IS_LABEL_IMAGE (this), NULL);

        return gl_text_node_dup (this->priv->filename);
}


void
gl_label_image_get_base_size (glLabelImage *this,
                              gdouble      *w,
                              gdouble      *h)
{
        RsvgDimensionData  svg_dim;

        g_return_if_fail (this && GL_IS_LABEL_IMAGE (this));
        g_return_if_fail (w != NULL);
        g_return_if_fail (h != NULL);

        switch (this->priv->type)
        {

        case FILE_TYPE_PIXBUF:
                *w = gdk_pixbuf_get_width (this->priv->pixbuf);
                *h = gdk_pixbuf_get_height (this->priv->pixbuf);
                break;

        case FILE_TYPE_SVG:
                rsvg_handle_get_dimensions (this->priv->svg_handle, &svg_dim);
                *w = svg_dim.width;
                *h = svg_dim.height;
                break;

        default:
                *w = gdk_pixbuf_get_width (default_pixbuf);
                *h = gdk_pixbuf_get_height (default_pixbuf);
                break;

        }
}


/*****************************************************************************/
/* Draw object method.                                                       */
/*****************************************************************************/
static void
draw_object (glLabelObject *object,
             cairo_t       *cr,
             gboolean       screen_flag,
             glMergeRecord *record)
{
        glLabelImage      *this = GL_LABEL_IMAGE (object);
        gdouble            w, h;
        gdouble            image_w, image_h;
        GdkPixbuf         *pixbuf;
        RsvgHandle        *svg_handle;
        RsvgDimensionData  svg_dim;

        gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_size (object, &w, &h);

        cairo_save (cr);

        switch (get_type (this, record))
        {

        case FILE_TYPE_PIXBUF:
                pixbuf = gl_label_image_get_pixbuf (this, record);
                if ( pixbuf )
                {
                        image_w = gdk_pixbuf_get_width (pixbuf);
                        image_h = gdk_pixbuf_get_height (pixbuf);
                        cairo_rectangle (cr, 0.0, 0.0, w, h);
                        cairo_scale (cr, w/image_w, h/image_h);
                        gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
                        cairo_fill (cr);
                        g_object_unref (pixbuf);
                }
                break;

        case FILE_TYPE_SVG:
                svg_handle = gl_label_image_get_svg_handle (this, record);
                if ( svg_handle )
                {
                        rsvg_handle_get_dimensions (svg_handle, &svg_dim);
                        cairo_scale (cr, w/svg_dim.width, h/svg_dim.height);
                        rsvg_handle_render_cairo (svg_handle, cr);
                }
                break;

        default:
                cairo_rectangle (cr, 0.0, 0.0, w, h);
                image_w = gdk_pixbuf_get_width (default_pixbuf);
                image_h = gdk_pixbuf_get_height (default_pixbuf);
                cairo_scale (cr, w/image_w, h/image_h);
                gdk_cairo_set_source_pixbuf (cr, default_pixbuf, 0, 0);
                cairo_fill (cr);
                break;

        }

        cairo_restore (cr);

        gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Draw shadow method.                                                       */
/*****************************************************************************/
static void
draw_shadow (glLabelObject *object,
             cairo_t       *cr,
             gboolean       screen_flag,
             glMergeRecord *record)
{
        glLabelImage    *this = GL_LABEL_IMAGE (object);
        gdouble          w, h;
        GdkPixbuf       *pixbuf;
        GdkPixbuf       *shadow_pixbuf;
        gdouble          image_w, image_h;
        glColorNode     *shadow_color_node;
        guint            shadow_color;
        gdouble          shadow_opacity;

        gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_size (object, &w, &h);

        shadow_color_node = gl_label_object_get_shadow_color (object);
        shadow_color = gl_color_node_expand (shadow_color_node, record);
        if (shadow_color_node->field_flag && screen_flag)
        {
                shadow_color = GL_COLOR_SHADOW_MERGE_DEFAULT;
        }
        shadow_opacity = gl_label_object_get_shadow_opacity (object);

        cairo_save (cr);

        switch (get_type (this, record))
        {

        case FILE_TYPE_PIXBUF:
                pixbuf = gl_label_image_get_pixbuf (this, record);
                if ( pixbuf )
                {
                        image_w = gdk_pixbuf_get_width (pixbuf);
                        image_h = gdk_pixbuf_get_height (pixbuf);

                        shadow_pixbuf = gl_pixbuf_util_create_shadow_pixbuf (pixbuf,
                                                                             shadow_color, shadow_opacity);
                        cairo_rectangle (cr, 0.0, 0.0, w, h);
                        cairo_scale (cr, w/image_w, h/image_h);
                        gdk_cairo_set_source_pixbuf (cr, (GdkPixbuf *)shadow_pixbuf, 0, 0);
                        cairo_fill (cr);

                        g_object_unref (G_OBJECT (shadow_pixbuf));
                        g_object_unref (G_OBJECT (pixbuf));
                }
                break;

        case FILE_TYPE_SVG:
                /* FIXME: no shadow support, yet. */
                break;

        default:
                shadow_color = gl_color_set_opacity (shadow_color, shadow_opacity);

                cairo_rectangle (cr, 0.0, 0.0, w, h);
                cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (shadow_color));
                cairo_fill (cr);
                break;
        }

        cairo_restore (cr);

        gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Is object at coordinates?                                                 */
/*****************************************************************************/
static gboolean
object_at (glLabelObject *object,
           cairo_t       *cr,
           gdouble        x,
           gdouble        y)
{
        gdouble           w, h;

        gl_label_object_get_size (object, &w, &h);

        cairo_new_path (cr);
        cairo_rectangle (cr, 0.0, 0.0, w, h);

        if (cairo_in_fill (cr, x, y))
        {
                return TRUE;
        }

        return FALSE;
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
