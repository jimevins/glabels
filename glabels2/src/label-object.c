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
#include <config.h>

#include <glib.h>
#include <libart_lgpl/libart.h>

#include "label-object.h"
#include "marshal.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelObjectPrivate {
	gchar             *name;
	gdouble            x, y;
	gdouble            w, h;
	gdouble            affine[6];
};

enum {
	CHANGED,
	MOVED,
	FLIP_ROTATE,
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
	signals[FLIP_ROTATE] =
		g_signal_new ("flip_rotate",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelObjectClass, flip_rotate),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
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

	art_affine_identity (object->private->affine);

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

	g_free (GL_LABEL_OBJECT(object)->private->name);
	g_free (GL_LABEL_OBJECT(object)->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* New label object.                                                         */
/*****************************************************************************/
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
/* Duplicate object.                                                         */
/*****************************************************************************/
glLabelObject *
gl_label_object_dup (glLabelObject *src_object,
		     glLabel       *label)
{
	glLabelObject    *dst_object;
	gdouble           x, y, w, h;
	gdouble           affine[6];

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (src_object && GL_IS_LABEL_OBJECT (src_object));

	dst_object = g_object_new (G_OBJECT_TYPE(src_object), NULL);

	gl_label_object_set_parent (dst_object, label);

	gl_label_object_get_position (src_object, &x, &y);
	gl_label_object_get_size     (src_object, &w, &h);
	gl_label_object_get_affine   (src_object, affine);

	gl_label_object_set_position (dst_object, x, y);
	gl_label_object_set_size     (dst_object, w, h);
	gl_label_object_set_affine   (dst_object, affine);

	if ( GL_LABEL_OBJECT_GET_CLASS(src_object)->copy != NULL ) {

		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(src_object)->copy (dst_object, src_object);

	}

	gl_debug (DEBUG_LABEL, "END");

	return dst_object;
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

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_size != NULL ) {

		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_size (object, w, h);

	} else {

		object->private->w = w;
		object->private->h = h;
	}

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

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_size != NULL ) {

		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->get_size (object, w, h);

	} else {

		*w = object->private->w;
		*h = object->private->h;
	}

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Get extent of object.                                                     */
/*****************************************************************************/
void
gl_label_object_get_extent (glLabelObject *object,
			    gdouble       *x1,
			    gdouble       *y1,
			    gdouble       *x2,
			    gdouble       *y2)
{
	gdouble  w, h;
	ArtPoint a1, a2, a3, a4, b1, b2, b3, b4;
	gdouble  affine[6];

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_size (object, &w, &h);

	/* setup untransformed corners of bounding box */
	a1.x = 0.0;
	a1.y = 0.0;
	a2.x = w;
	a2.y = 0.0;
	a3.x = w;
	a3.y = h;
	a4.x = 0.0;
	a4.y = h;

	/* transform these points */
	gl_label_object_get_applied_affine (object, affine);
	art_affine_point (&b1, &a1, affine);
	art_affine_point (&b2, &a2, affine);
	art_affine_point (&b3, &a3, affine);
	art_affine_point (&b4, &a4, affine);

	/* now find the maximum extent of these points in x and y */
	*x1 = MIN (b1.x, MIN (b2.x, MIN (b3.x, b4.x))) + object->private->x;
	*y1 = MIN (b1.y, MIN (b2.y, MIN (b3.y, b4.y))) + object->private->y;
	*x2 = MAX (b1.x, MAX (b2.x, MAX (b3.x, b4.x))) + object->private->x;
	*y2 = MAX (b1.y, MAX (b2.y, MAX (b3.y, b4.y))) + object->private->y;
	
	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Flip object horizontally.                                                */
/****************************************************************************/
void
gl_label_object_flip_horiz (glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	art_affine_flip (object->private->affine, object->private->affine, TRUE, FALSE);

	g_signal_emit (G_OBJECT(object), signals[FLIP_ROTATE], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Flip object vertically.                                                  */
/****************************************************************************/
void
gl_label_object_flip_vert (glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	art_affine_flip (object->private->affine, object->private->affine, FALSE, TRUE);

	g_signal_emit (G_OBJECT(object), signals[FLIP_ROTATE], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Rotate object.                                                           */
/****************************************************************************/
void
gl_label_object_rotate (glLabelObject *object,
			gdouble        theta_degs)
{
	gdouble rotate_affine[6];

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	art_affine_rotate (rotate_affine, theta_degs);
	art_affine_multiply (object->private->affine, object->private->affine, rotate_affine);

	g_signal_emit (G_OBJECT(object), signals[FLIP_ROTATE], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Set raw affine                                                           */
/****************************************************************************/
void
gl_label_object_set_affine (glLabelObject *object,
			    gdouble        affine[6])
{
	gl_debug (DEBUG_LABEL, "");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	object->private->affine[0] = affine[0];
	object->private->affine[1] = affine[1];
	object->private->affine[2] = affine[2];
	object->private->affine[3] = affine[3];
	object->private->affine[4] = affine[4];
	object->private->affine[5] = affine[5];
}

/****************************************************************************/
/* Get raw affine                                                           */
/****************************************************************************/
void
gl_label_object_get_affine (glLabelObject *object,
			    gdouble        affine[6])
{
	gl_debug (DEBUG_LABEL, "");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	affine[0] = object->private->affine[0];
	affine[1] = object->private->affine[1];
	affine[2] = object->private->affine[2];
	affine[3] = object->private->affine[3];
	affine[4] = object->private->affine[4];
	affine[5] = object->private->affine[5];
}

/****************************************************************************/
/* Get applied affine, i.e. translated to center of object and back         */
/****************************************************************************/
void
gl_label_object_get_applied_affine (glLabelObject *object,
				    gdouble        affine[6])
{
	gdouble w, h;
	gdouble to_center[6], to_origin[6];

	gl_debug (DEBUG_LABEL, "");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_size (object, &w, &h);

	/* setup transformation affine */
	art_affine_translate (to_center, -w/2.0, -h/2.0);
	art_affine_multiply (affine, to_center, object->private->affine);
	art_affine_translate (to_origin, w/2.0, h/2.0);
	art_affine_multiply (affine, affine, to_origin);
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

