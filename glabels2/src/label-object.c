/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_object.c:  GLabels label object base class
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

#include "label-object.h"
#include "marshal.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelObjectPrivate {
	gchar  *name;
	gdouble x, y;
	gdouble w, h;
};

enum {
	CHANGED,
	MOVED,
	TOP,
	BOTTOM,
	LAST_SIGNAL
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;

static guint signals[LAST_SIGNAL] = {0};

static guint instance = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_object_class_init    (glLabelObjectClass *klass);
static void gl_label_object_instance_init (glLabelObject      *object);
static void gl_label_object_finalize      (GObject            *object);

static void merge_changed_cb              (glLabel            *label,
					   glLabelObject      *object);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_object_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glLabelObjectClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_object_class_init,
			NULL,
			NULL,
			sizeof (glLabelObject),
			0,
			(GInstanceInitFunc) gl_label_object_instance_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "glLabelObject", &info, 0);
	}

	return type;
}

static void
gl_label_object_class_init (glLabelObjectClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gl_debug (DEBUG_LABEL, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_label_object_finalize;

	signals[CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelObjectClass, changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	signals[MOVED] =
		g_signal_new ("moved",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelObjectClass, moved),
			      NULL, NULL,
			      gl_marshal_VOID__DOUBLE_DOUBLE,
			      G_TYPE_NONE,
			      2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	signals[TOP] =
		g_signal_new ("top",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelObjectClass, top),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	signals[BOTTOM] =
		g_signal_new ("bottom",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelObjectClass, bottom),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	gl_debug (DEBUG_LABEL, "END");
}

static void
gl_label_object_instance_init (glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	object->private = g_new0 (glLabelObjectPrivate, 1);

	object->private->name = g_strdup_printf ("object%d", instance++);

	gl_debug (DEBUG_LABEL, "END");
}

static void
gl_label_object_finalize (GObject *object)
{
	glLabel       *parent;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	parent = GL_LABEL_OBJECT(object)->parent;
	gl_label_remove_object (parent, GL_LABEL_OBJECT(object));

	g_free (GL_LABEL_OBJECT(object)->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_LABEL, "END");
}

GObject *
gl_label_object_new (glLabel *label)
{
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	object = g_object_new (gl_label_object_get_type(), NULL);

	gl_label_object_set_parent (object, label);

	gl_debug (DEBUG_LABEL, "END");

	return G_OBJECT (object);
}

/*****************************************************************************/
/* Emit "changed" signal (for derived objects).                              */
/*****************************************************************************/
void
gl_label_object_emit_changed (glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	g_signal_emit (G_OBJECT(object), signals[CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Set parent label of object.                                               */
/*****************************************************************************/
void
gl_label_object_set_parent (glLabelObject *object,
			    glLabel       *label)
{
	glLabel *old_parent;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));
	g_return_if_fail (label && GL_IS_LABEL (label));

	old_parent = object->parent;
	if ( old_parent != NULL ) {
		g_signal_handlers_disconnect_by_func (old_parent,
						      G_CALLBACK(merge_changed_cb),
						      object);
		gl_label_remove_object( old_parent, object );
	}
	gl_label_add_object( label, object );

	g_signal_connect (G_OBJECT(label), "merge_changed",
			  G_CALLBACK(merge_changed_cb), object);

	g_signal_emit (G_OBJECT(object), signals[CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Get parent label of object.                                               */
/*****************************************************************************/
glLabel *
gl_label_object_get_parent (glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_debug (DEBUG_LABEL, "END");

	return object->parent;
}

/*****************************************************************************/
/* Set name of object.                                                       */
/*****************************************************************************/
void
gl_label_object_set_name (glLabelObject *object,
			  gchar         *name)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	g_free(object->private->name);
	object->private->name = name;

	g_signal_emit (G_OBJECT(object), signals[CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Get name of object.                                                       */
/*****************************************************************************/
gchar *
gl_label_object_get_name (glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_debug (DEBUG_LABEL, "END");

	return g_strdup(object->private->name);
}

/*****************************************************************************/
/* Set position of object.                                                   */
/*****************************************************************************/
void
gl_label_object_set_position (glLabelObject *object,
			      gdouble        x,
			      gdouble        y)
{
	gdouble dx, dy;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	dx = x - object->private->x;
	dy = y - object->private->y;

	object->private->x = x;
	object->private->y = y;

	g_signal_emit (G_OBJECT(object), signals[MOVED], 0, dx, dy);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Set position of object relative to old position.                          */
/*****************************************************************************/
void
gl_label_object_set_position_relative (glLabelObject *object,
				       gdouble        dx,
				       gdouble        dy)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	object->private->x += dx;
	object->private->y += dy;

	gl_debug (DEBUG_LABEL, "       x = %f, y= %f",
		  object->private->x,
		  object->private->y);

	g_signal_emit (G_OBJECT(object), signals[MOVED], 0, dx, dy);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Get position of object.                                                   */
/*****************************************************************************/
void
gl_label_object_get_position (glLabelObject *object,
			      gdouble       *x,
			      gdouble       *y)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	*x = object->private->x;
	*y = object->private->y;

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Set size of object.                                                       */
/*****************************************************************************/
void
gl_label_object_set_size (glLabelObject *object,
			  gdouble        w,
			  gdouble        h)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	object->private->w = w;
	object->private->h = h;

	g_signal_emit (G_OBJECT(object), signals[CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Get size of object.                                                       */
/*****************************************************************************/
void
gl_label_object_get_size (glLabelObject *object,
			  gdouble       *w,
			  gdouble       *h)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	*w = object->private->w;
	*h = object->private->h;

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Bring label object to front/top.                                         */
/****************************************************************************/
void
gl_label_object_raise_to_top (glLabelObject *object)
{
	glLabel *label;

	gl_debug (DEBUG_LABEL, "START");

	label = object->parent;

	gl_label_raise_object_to_top (label, object);

	g_signal_emit (G_OBJECT(object), signals[TOP], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Send label object to rear/bottom.                                        */
/****************************************************************************/
void
gl_label_object_lower_to_bottom (glLabelObject *object)
{
	glLabel *label;

	gl_debug (DEBUG_LABEL, "START");

	label = object->parent;

	gl_label_lower_object_to_bottom (label, object);

	g_signal_emit (G_OBJECT(object), signals[BOTTOM], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Label's merge data changed callback.                           */
/*--------------------------------------------------------------------------*/
static void
merge_changed_cb (glLabel       *label,
		  glLabelObject *object)
{
	gl_label_object_emit_changed (object);
}

