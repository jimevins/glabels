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

#include <glib.h>

#include "label-image.h"

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

static GObjectClass *parent_class = NULL;

static guint instance = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_image_class_init    (glLabelImageClass *klass);
static void gl_label_image_instance_init (glLabelImage      *limage);
static void gl_label_image_finalize      (GObject           *object);

static void copy                         (glLabelObject     *dst_object,
					  glLabelObject     *src_object);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_image_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glLabelImageClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_image_class_init,
			NULL,
			NULL,
			sizeof (glLabelImage),
			0,
			(GInstanceInitFunc) gl_label_image_instance_init,
		};

		type = g_type_register_static (GL_TYPE_LABEL_OBJECT,
					       "glLabelImage", &info, 0);
	}

	return type;
}

static void
gl_label_image_class_init (glLabelImageClass *klass)
{
	GObjectClass       *object_class       = (GObjectClass *) klass;
	glLabelObjectClass *label_object_class = (glLabelObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	label_object_class->copy = copy;

	object_class->finalize = gl_label_image_finalize;
}

static void
gl_label_image_instance_init (glLabelImage *limage)
{
	limage->private = g_new0 (glLabelImagePrivate, 1);

	limage->private->filename = g_new0 (glTextNode, 1);

	limage->private->pixbuf =
		gdk_pixbuf_new_from_xpm_data ((const char **)checkerboard_xpm);
}

static void
gl_label_image_finalize (GObject *object)
{
	glLabelImage *limage;

	g_return_if_fail (object && GL_IS_LABEL_IMAGE (object));

	limage = GL_LABEL_IMAGE (object);

	gl_text_node_free (&limage->private->filename);
	g_object_unref (G_OBJECT(limage->private->pixbuf));
	g_free (limage->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*****************************************************************************/
/* NEW label "image" object.                                               */
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
	glLabelImage *limage     = (glLabelImage *)src_object;
	glLabelImage *new_limage = (glLabelImage *)dst_object;
	glTextNode   *filename;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (limage && GL_IS_LABEL_IMAGE (limage));
	g_return_if_fail (new_limage && GL_IS_LABEL_IMAGE (new_limage));

	filename = gl_label_image_get_filename (limage);
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
	GdkPixbuf *pixbuf;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (limage && GL_IS_LABEL_IMAGE (limage));

	if ( (filename == NULL) || filename->field_flag || (filename->data == NULL) ) {

		gl_text_node_free (&limage->private->filename);
		limage->private->filename = gl_text_node_dup(filename);

		g_object_unref (limage->private->pixbuf);
		limage->private->pixbuf =
			gdk_pixbuf_new_from_xpm_data ((const char **)
						      checkerboard_xpm);

		gl_label_object_emit_changed (GL_LABEL_OBJECT(limage));
	} else {

		if ( limage->private->filename == NULL) {

			limage->private->filename = gl_text_node_dup (filename);

			pixbuf = gdk_pixbuf_new_from_file (filename->data, NULL);
			g_object_unref (limage->private->pixbuf);
			if ( pixbuf != NULL ) {
				limage->private->pixbuf = pixbuf;
			} else {
				limage->private->pixbuf =
					gdk_pixbuf_new_from_xpm_data ((const char **)
								      checkerboard_xpm);
			}

			gl_label_object_emit_changed (GL_LABEL_OBJECT(limage));

		} else if ( (limage->private->filename->data == NULL) ||
			    strcmp (limage->private->filename->data, filename->data) != 0) {

			gl_text_node_free (&limage->private->filename);
			limage->private->filename = gl_text_node_dup (filename);

			pixbuf = gdk_pixbuf_new_from_file (filename->data, NULL);
			g_object_unref (limage->private->pixbuf);
			if ( pixbuf != NULL ) {
				limage->private->pixbuf = pixbuf;
			} else {
				limage->private->pixbuf =
					gdk_pixbuf_new_from_xpm_data ((const char **)
								      checkerboard_xpm);
			}

			gl_label_object_emit_changed (GL_LABEL_OBJECT(limage));
		}

	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
glTextNode *
gl_label_image_get_filename (glLabelImage *limage)
{
	g_return_val_if_fail (limage && GL_IS_LABEL_IMAGE (limage), NULL);

	return gl_text_node_dup (limage->private->filename);
}

const GdkPixbuf *
gl_label_image_get_pixbuf (glLabelImage  *limage,
			   glMergeRecord *record)
{
	GdkPixbuf *pixbuf = NULL;
	gchar     *real_filename;

	g_return_val_if_fail (limage && GL_IS_LABEL_IMAGE (limage), NULL);

	if ((record != NULL) && limage->private->filename->field_flag) {
		
		/* Indirect filename, re-evaluate for given record. */

		real_filename = gl_merge_eval_key (record,
						   limage->private->filename->data);

		if (real_filename != NULL) {
			pixbuf = gdk_pixbuf_new_from_file (real_filename, NULL);
		}
		g_object_unref (limage->private->pixbuf);
		if ( pixbuf != NULL ) {
			limage->private->pixbuf = pixbuf;
		} else {
			limage->private->pixbuf =
				gdk_pixbuf_new_from_xpm_data ((const char **)
							      checkerboard_xpm);
		}

	}

	return limage->private->pixbuf;
}

