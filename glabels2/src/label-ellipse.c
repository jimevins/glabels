/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_ellipse.c:  GLabels label ellipse object
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

#include "label-ellipse.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelEllipsePrivate {
	gdouble          line_width;
	guint            line_color;
	guint            fill_color;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;

static guint instance = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_ellipse_class_init    (glLabelEllipseClass *klass);
static void gl_label_ellipse_instance_init (glLabelEllipse      *lellipse);
static void gl_label_ellipse_finalize      (GObject             *object);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_ellipse_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glLabelEllipseClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_ellipse_class_init,
			NULL,
			NULL,
			sizeof (glLabelEllipse),
			0,
			(GInstanceInitFunc) gl_label_ellipse_instance_init,
		};

		type = g_type_register_static (GL_TYPE_LABEL_OBJECT,
					       "glLabelEllipse", &info, 0);
	}

	return type;
}

static void
gl_label_ellipse_class_init (glLabelEllipseClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_label_ellipse_finalize;
}

static void
gl_label_ellipse_instance_init (glLabelEllipse *lellipse)
{
	lellipse->private = g_new0 (glLabelEllipsePrivate, 1);
}

static void
gl_label_ellipse_finalize (GObject *object)
{
	glLabelEllipse *lellipse;

	g_return_if_fail (object && GL_IS_LABEL_ELLIPSE (object));

	lellipse = GL_LABEL_ELLIPSE (object);

	g_free (lellipse->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*****************************************************************************/
/* NEW label "ellipse" object.                                               */
/*****************************************************************************/
GObject *
gl_label_ellipse_new (glLabel *label)
{
	glLabelEllipse *lellipse;

	lellipse = g_object_new (gl_label_ellipse_get_type(), NULL);

	gl_label_object_set_parent (GL_LABEL_OBJECT(lellipse), label);

	return G_OBJECT (lellipse);
}

/*****************************************************************************/
/* Duplicate object.                                                         */
/*****************************************************************************/
glLabelEllipse *
gl_label_ellipse_dup (glLabelEllipse *lellipse,
		      glLabel        *label)
{
	glLabelEllipse *new_lellipse;
	gdouble         line_width;
	guint           line_color, fill_color;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse));
	g_return_if_fail (label && GL_IS_LABEL (label));

	new_lellipse = GL_LABEL_ELLIPSE(gl_label_ellipse_new (label));

	gl_label_object_copy_props (GL_LABEL_OBJECT(new_lellipse),
				    GL_LABEL_OBJECT(lellipse));

	line_width = gl_label_ellipse_get_line_width (lellipse);
	line_color = gl_label_ellipse_get_line_color (lellipse);
	fill_color = gl_label_ellipse_get_fill_color (lellipse);

	gl_label_ellipse_set_line_width (new_lellipse, line_width);
	gl_label_ellipse_set_line_color (new_lellipse, line_color);
	gl_label_ellipse_set_fill_color (new_lellipse, fill_color);

	gl_debug (DEBUG_LABEL, "END");

	return new_lellipse;
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_ellipse_set_line_width (glLabelEllipse *lellipse,
				 gdouble         line_width)
{
	g_return_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse));

	if ( lellipse->private->line_width != line_width ) {
		lellipse->private->line_width = line_width;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lellipse));
	}
}

void
gl_label_ellipse_set_line_color (glLabelEllipse *lellipse,
				 guint           line_color)
{
	g_return_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse));

	if ( lellipse->private->line_color != line_color ) {
		lellipse->private->line_color = line_color;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lellipse));
	}
}

void
gl_label_ellipse_set_fill_color (glLabelEllipse *lellipse,
				 guint           fill_color)
{
	g_return_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse));

	if ( lellipse->private->fill_color != fill_color ) {
		lellipse->private->fill_color = fill_color;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lellipse));
	}
}

/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
gdouble
gl_label_ellipse_get_line_width (glLabelEllipse *lellipse)
{
	g_return_val_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse), 0.0);

	return lellipse->private->line_width;
}

guint
gl_label_ellipse_get_line_color (glLabelEllipse *lellipse)
{
	g_return_val_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse), 0);

	return lellipse->private->line_color;
}

guint
gl_label_ellipse_get_fill_color (glLabelEllipse *lellipse)
{
	g_return_val_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse), 0);

	return lellipse->private->fill_color;
}

