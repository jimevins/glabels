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

static void    gl_label_line_class_init    (glLabelLineClass *klass);
static void    gl_label_line_instance_init (glLabelLine      *lline);
static void    gl_label_line_finalize      (GObject          *object);

static void    copy                        (glLabelObject    *dst_object,
					    glLabelObject    *src_object);

static void    set_line_color              (glLabelObject    *object,
					    guint             line_color);

static void    set_line_width              (glLabelObject    *object,
					    gdouble           line_width);

static guint   get_line_color              (glLabelObject    *object);

static gdouble get_line_width              (glLabelObject    *object);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_line_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glLabelLineClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_line_class_init,
			NULL,
			NULL,
			sizeof (glLabelLine),
			0,
			(GInstanceInitFunc) gl_label_line_instance_init,
			NULL
		};

		type = g_type_register_static (GL_TYPE_LABEL_OBJECT,
					       "glLabelLine", &info, 0);
	}

	return type;
}

static void
gl_label_line_class_init (glLabelLineClass *klass)
{
	GObjectClass       *object_class       = (GObjectClass *) klass;
	glLabelObjectClass *label_object_class = (glLabelObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	label_object_class->copy           = copy;
	label_object_class->set_line_color = set_line_color;
	label_object_class->set_line_width = set_line_width;
	label_object_class->get_line_color = get_line_color;
	label_object_class->get_line_width = get_line_width;

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
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
	glLabelLine *lline     = (glLabelLine *)src_object;
	glLabelLine *new_lline = (glLabelLine *)dst_object;
	gdouble      line_width;
	guint        line_color;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lline && GL_IS_LABEL_LINE (lline));
	g_return_if_fail (new_lline && GL_IS_LABEL_LINE (new_lline));

	line_width = get_line_width (src_object);
	line_color = get_line_color (src_object);

	set_line_width (dst_object, line_width);
	set_line_color (dst_object, line_color);

	gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_color (glLabelObject *object,
		guint          line_color)
{
	glLabelLine *lline = (glLabelLine *)object;

	g_return_if_fail (lline && GL_IS_LABEL_LINE (lline));

	if ( lline->private->line_color != line_color ) {
		lline->private->line_color = line_color;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lline));
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line width method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_width (glLabelObject *object,
		gdouble        line_width)
{
	glLabelLine *lline = (glLabelLine *)object;

	g_return_if_fail (lline && GL_IS_LABEL_LINE (lline));

	if ( lline->private->line_width != line_width ) {
		lline->private->line_width = line_width;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lline));
	}
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line color method.                                          */
/*---------------------------------------------------------------------------*/
static gdouble
get_line_width (glLabelObject *object)
{
	glLabelLine *lline = (glLabelLine *)object;

	g_return_val_if_fail (lline && GL_IS_LABEL_LINE (lline), 0.0);

	return lline->private->line_width;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line width method.                                          */
/*---------------------------------------------------------------------------*/
static guint
get_line_color (glLabelObject *object)
{
	glLabelLine *lline = (glLabelLine *)object;

	g_return_val_if_fail (lline && GL_IS_LABEL_LINE (lline), 0);

	return lline->private->line_color;
}

