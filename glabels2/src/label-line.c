/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_line.c:  GLabels label line object
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

#include "label-line.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelLinePrivate {
	gdouble          line_width;
	guint            line_color;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;

static guint instance = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_line_class_init (glLabelLineClass *klass);
static void gl_label_line_instance_init (glLabelLine *lline);
static void gl_label_line_finalize (GObject *object);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_line_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glLabelLineClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_line_class_init,
			NULL,
			NULL,
			sizeof (glLabelLine),
			0,
			(GInstanceInitFunc) gl_label_line_instance_init,
		};

		type = g_type_register_static (GL_TYPE_LABEL_OBJECT,
					       "glLabelLine", &info, 0);
	}

	return type;
}

static void
gl_label_line_class_init (glLabelLineClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_label_line_finalize;
}

static void
gl_label_line_instance_init (glLabelLine *lline)
{
	lline->private = g_new0 (glLabelLinePrivate, 1);
}

static void
gl_label_line_finalize (GObject *object)
{
	glLabelLine *lline;

	g_return_if_fail (object && GL_IS_LABEL_LINE (object));

	lline = GL_LABEL_LINE (object);

	g_free (lline->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*****************************************************************************/
/* NEW label "line" object.                                               */
/*****************************************************************************/
GObject *
gl_label_line_new (glLabel *label)
{
	glLabelLine *lline;

	lline = g_object_new (gl_label_line_get_type(), NULL);

	gl_label_object_set_parent (GL_LABEL_OBJECT(lline), label);

	return G_OBJECT (lline);
}

/*****************************************************************************/
/* Duplicate object.                                                         */
/*****************************************************************************/
glLabelLine *
gl_label_line_dup (glLabelLine *lline,
		      glLabel *label)
{
	glLabelLine *new_lline;
	gdouble        x, y, w, h, line_width;
	guint          line_color;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lline && GL_IS_LABEL_LINE (lline));
	g_return_if_fail (label && GL_IS_LABEL (label));

	new_lline = GL_LABEL_LINE(gl_label_line_new (label));

	gl_label_object_get_position (GL_LABEL_OBJECT(lline), &x, &y);
	gl_label_object_get_size     (GL_LABEL_OBJECT(lline), &w, &h);

	gl_label_object_set_position (GL_LABEL_OBJECT(new_lline),  x,  y);
	gl_label_object_set_size     (GL_LABEL_OBJECT(new_lline),  w,  h);

	line_width = gl_label_line_get_line_width (lline);
	line_color = gl_label_line_get_line_color (lline);

	gl_label_line_set_line_width (new_lline, line_width);
	gl_label_line_set_line_color (new_lline, line_color);

	gl_debug (DEBUG_LABEL, "END");

	return new_lline;
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_line_set_line_width (glLabelLine *lline,
				 gdouble        line_width)
{
	g_return_if_fail (lline && GL_IS_LABEL_LINE (lline));

	if ( lline->private->line_width != line_width ) {
		lline->private->line_width = line_width;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lline));
	}
}

void
gl_label_line_set_line_color (glLabelLine *lline,
				 guint          line_color)
{
	g_return_if_fail (lline && GL_IS_LABEL_LINE (lline));

	if ( lline->private->line_color != line_color ) {
		lline->private->line_color = line_color;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lline));
	}
}


/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
gdouble
gl_label_line_get_line_width (glLabelLine *lline)
{
	g_return_val_if_fail (lline && GL_IS_LABEL_LINE (lline), 0.0);

	return lline->private->line_width;
}

guint
gl_label_line_get_line_color (glLabelLine *lline)
{
	g_return_val_if_fail (lline && GL_IS_LABEL_LINE (lline), 0);

	return lline->private->line_color;
}

