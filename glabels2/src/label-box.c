/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_box.c:  GLabels label box object
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

#include "label-box.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelBoxPrivate {
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

static void gl_label_box_class_init    (glLabelBoxClass *klass);
static void gl_label_box_instance_init (glLabelBox      *lbox);
static void gl_label_box_finalize      (GObject         *object);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_box_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glLabelBoxClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_box_class_init,
			NULL,
			NULL,
			sizeof (glLabelBox),
			0,
			(GInstanceInitFunc) gl_label_box_instance_init,
		};

		type = g_type_register_static (GL_TYPE_LABEL_OBJECT,
					       "glLabelBox", &info, 0);
	}

	return type;
}

static void
gl_label_box_class_init (glLabelBoxClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_label_box_finalize;
}

static void
gl_label_box_instance_init (glLabelBox *lbox)
{
	lbox->private = g_new0 (glLabelBoxPrivate, 1);
}

static void
gl_label_box_finalize (GObject *object)
{
	glLabelBox *lbox;

	g_return_if_fail (object && GL_IS_LABEL_BOX (object));

	lbox = GL_LABEL_BOX (object);

	g_free (lbox->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*****************************************************************************/
/* NEW label "box" object.                                                   */
/*****************************************************************************/
GObject *
gl_label_box_new (glLabel *label)
{
	glLabelBox *lbox;

	lbox = g_object_new (gl_label_box_get_type(), NULL);

	gl_label_object_set_parent (GL_LABEL_OBJECT(lbox), label);

	return G_OBJECT (lbox);
}

/*****************************************************************************/
/* Duplicate object.                                                         */
/*****************************************************************************/
glLabelBox *
gl_label_box_dup (glLabelBox *lbox,
		  glLabel    *label)
{
	glLabelBox *new_lbox;
	gdouble     x, y, w, h, line_width;
	guint       line_color, fill_color;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbox && GL_IS_LABEL_BOX (lbox));
	g_return_if_fail (label && GL_IS_LABEL (label));

	new_lbox = GL_LABEL_BOX(gl_label_box_new (label));

	gl_label_object_get_position (GL_LABEL_OBJECT(lbox), &x, &y);
	gl_label_object_get_size     (GL_LABEL_OBJECT(lbox), &w, &h);

	gl_label_object_set_position (GL_LABEL_OBJECT(new_lbox),  x,  y);
	gl_label_object_set_size     (GL_LABEL_OBJECT(new_lbox),  w,  h);

	line_width = gl_label_box_get_line_width (lbox);
	line_color = gl_label_box_get_line_color (lbox);
	fill_color = gl_label_box_get_fill_color (lbox);

	gl_label_box_set_line_width (new_lbox, line_width);
	gl_label_box_set_line_color (new_lbox, line_color);
	gl_label_box_set_fill_color (new_lbox, fill_color);


	gl_debug (DEBUG_LABEL, "END");

	return new_lbox;
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_box_set_line_width (glLabelBox *lbox,
			     gdouble     line_width)
{
	g_return_if_fail (lbox && GL_IS_LABEL_BOX (lbox));

	if ( lbox->private->line_width != line_width ) {
		lbox->private->line_width = line_width;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbox));
	}
}

void
gl_label_box_set_line_color (glLabelBox *lbox,
			     guint       line_color)
{
	g_return_if_fail (lbox && GL_IS_LABEL_BOX (lbox));

	if ( lbox->private->line_color != line_color ) {
		lbox->private->line_color = line_color;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbox));
	}
}

void
gl_label_box_set_fill_color (glLabelBox *lbox,
			     guint       fill_color)
{
	g_return_if_fail (lbox && GL_IS_LABEL_BOX (lbox));

	if ( lbox->private->fill_color != fill_color ) {
		lbox->private->fill_color = fill_color;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbox));
	}
}

/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
gdouble
gl_label_box_get_line_width (glLabelBox *lbox)
{
	g_return_val_if_fail (lbox && GL_IS_LABEL_BOX (lbox), 0.0);

	return lbox->private->line_width;
}

guint
gl_label_box_get_line_color (glLabelBox *lbox)
{
	g_return_val_if_fail (lbox && GL_IS_LABEL_BOX (lbox), 0);

	return lbox->private->line_color;
}

guint
gl_label_box_get_fill_color (glLabelBox *lbox)
{
	g_return_val_if_fail (lbox && GL_IS_LABEL_BOX (lbox), 0);

	return lbox->private->fill_color;
}

