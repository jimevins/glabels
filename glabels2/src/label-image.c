/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_image.c:  GLabels label image object
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include "label-image.h"

#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include <glib/gmessages.h>

#include "pixmaps/checkerboard.xpm"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelImagePrivate {
	glTextNode       *filename;
	GdkPixbuf        *pixbuf;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static guint instance = 0;

static GdkPixbuf *default_pixbuf = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_image_finalize      (GObject           *object);

static void copy                         (glLabelObject     *dst_object,
					  glLabelObject     *src_object);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelImage, gl_label_image, GL_TYPE_LABEL_OBJECT);

static void
gl_label_image_class_init (glLabelImageClass *class)
{
	GObjectClass       *object_class       = G_OBJECT_CLASS (class);
	glLabelObjectClass *label_object_class = GL_LABEL_OBJECT_CLASS (class);

	gl_label_image_parent_class = g_type_class_peek_parent (class);

	label_object_class->copy = copy;

	object_class->finalize = gl_label_image_finalize;
}

static void
gl_label_image_init (glLabelImage *limage)
{
	if ( default_pixbuf == NULL ) {
		default_pixbuf =
			gdk_pixbuf_new_from_xpm_data ((const char **)checkerboard_xpm);
	}

	limage->priv = g_new0 (glLabelImagePrivate, 1);

	limage->priv->filename = g_new0 (glTextNode, 1);

	limage->priv->pixbuf = default_pixbuf;
}

static void
gl_label_image_finalize (GObject *object)
{
	glLabelObject *lobject = GL_LABEL_OBJECT (object);
	glLabelImage  *limage  = GL_LABEL_IMAGE (object);;
	GHashTable    *pixbuf_cache;

	g_return_if_fail (object && GL_IS_LABEL_IMAGE (object));

	if (!limage->priv->filename->field_flag) {
		pixbuf_cache = gl_label_get_pixbuf_cache (lobject->parent);
		gl_pixbuf_cache_remove_pixbuf (pixbuf_cache,
					       limage->priv->filename->data);
	}
	gl_text_node_free (&limage->priv->filename);
	g_free (limage->priv);

	G_OBJECT_CLASS (gl_label_image_parent_class)->finalize (object);
}

/*****************************************************************************/
/* NEW label "image" object.                                                 */
/*****************************************************************************/
GObject *
gl_label_image_new (glLabel *label)
{
	glLabelImage *limage;

	limage = g_object_new (gl_label_image_get_type(), NULL);

	gl_label_object_set_parent (GL_LABEL_OBJECT(limage), label);

	return G_OBJECT (limage);
}

/*****************************************************************************/
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
	glLabelImage     *limage     = (glLabelImage *)src_object;
	glLabelImage     *new_limage = (glLabelImage *)dst_object;
	glTextNode       *filename;
	GdkPixbuf        *pixbuf;
	GHashTable       *pixbuf_cache;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (limage && GL_IS_LABEL_IMAGE (limage));
	g_return_if_fail (new_limage && GL_IS_LABEL_IMAGE (new_limage));

	filename = gl_label_image_get_filename (limage);

	/* Make sure destination label has data suitably cached. */
	if ( !filename->field_flag && (filename->data != NULL) ) {
		pixbuf = limage->priv->pixbuf;
		if ( pixbuf != default_pixbuf ) {
			pixbuf_cache = gl_label_get_pixbuf_cache (dst_object->parent);
			gl_pixbuf_cache_add_pixbuf (pixbuf_cache, filename->data, pixbuf);
		}
	}

	gl_label_image_set_filename (new_limage, filename);
	gl_text_node_free (&filename);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_image_set_filename (glLabelImage *limage,
			     glTextNode   *filename)
{
	glTextNode  *old_filename;
	GHashTable  *pixbuf_cache;
	GdkPixbuf   *pixbuf;
	gdouble      image_w, image_h, aspect_ratio, w, h;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (limage && GL_IS_LABEL_IMAGE (limage));
	g_return_if_fail (filename != NULL);

	old_filename = limage->priv->filename;

	/* If Unchanged don't do anything */
	if ( gl_text_node_equal (filename, old_filename ) ) {
		return;
	}

	pixbuf_cache = gl_label_get_pixbuf_cache (GL_LABEL_OBJECT(limage)->parent);

	/* Remove reference to previous pixbuf from cache, if needed. */
	if ( !old_filename->field_flag && (old_filename->data != NULL) ) {
		gl_pixbuf_cache_remove_pixbuf (pixbuf_cache, old_filename->data);
	}

	/* Set new filename. */
	limage->priv->filename = gl_text_node_dup(filename);
	gl_text_node_free (&old_filename);

	/* Now set the pixbuf. */
	if ( filename->field_flag || (filename->data == NULL) ) {

		limage->priv->pixbuf = default_pixbuf;

	} else {

		pixbuf = gl_pixbuf_cache_get_pixbuf (pixbuf_cache, filename->data);

		if (pixbuf != NULL) {
			limage->priv->pixbuf = pixbuf;
		} else {
			limage->priv->pixbuf = default_pixbuf;
		}
	}

	/* Treat current size as a bounding box, scale image to maintain aspect
	 * ratio while fitting it in this bounding box. */
	image_w = gdk_pixbuf_get_width (limage->priv->pixbuf);
	image_h = gdk_pixbuf_get_height (limage->priv->pixbuf);
	aspect_ratio = image_h / image_w;
	gl_label_object_get_size (GL_LABEL_OBJECT(limage), &w, &h);
	if ( h > w*aspect_ratio ) {
		h = w * aspect_ratio;
	} else {
		w = h / aspect_ratio;
	}
	gl_label_object_set_size (GL_LABEL_OBJECT(limage), w, h);

	gl_label_object_emit_changed (GL_LABEL_OBJECT(limage));

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
glTextNode *
gl_label_image_get_filename (glLabelImage *limage)
{
	g_return_val_if_fail (limage && GL_IS_LABEL_IMAGE (limage), NULL);

	return gl_text_node_dup (limage->priv->filename);
}

const GdkPixbuf *
gl_label_image_get_pixbuf (glLabelImage  *limage,
			   glMergeRecord *record)
{
	g_return_val_if_fail (limage && GL_IS_LABEL_IMAGE (limage), NULL);

	if ((record != NULL) && limage->priv->filename->field_flag) {

		GdkPixbuf   *pixbuf = NULL;
		gchar       *real_filename;

		/* Indirect filename, re-evaluate for given record. */

		real_filename = gl_merge_eval_key (record,
						   limage->priv->filename->data);

		if (real_filename != NULL) {
			pixbuf = gdk_pixbuf_new_from_file (real_filename, NULL);
		}
		if ( pixbuf != NULL ) {
			return pixbuf;
		} else {
			return default_pixbuf;
		}

	}

	return limage->priv->pixbuf;

}



