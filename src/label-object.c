/*
 *  label-object.c
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

#include "label-object.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <math.h>

#include "marshal.h"

#include "debug.h"


/*========================================================*/
/* Private defines.                                       */
/*========================================================*/

#define DEFAULT_SHADOW_X_OFFSET (3.6)
#define DEFAULT_SHADOW_Y_OFFSET (3.6)
#define DEFAULT_SHADOW_COLOR    GL_COLOR_BLACK
#define DEFAULT_SHADOW_OPACITY  (0.5)

#define HANDLE_FILL_RGBA_ARGS      0.0,   0.75,  0.0,   0.4
#define HANDLE_OUTLINE_RGBA_ARGS   0.0,   0.0,   0.0,   0.8

#define HANDLE_OUTLINE_WIDTH_PIXELS   1.0
#define HANDLE_PIXELS 7


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelObjectPrivate {

	gchar             *name;

        gboolean           selected_flag;
        glLabel           *parent;

	gdouble            x, y;
	gdouble            w, h;
        cairo_matrix_t     matrix;

	gdouble            aspect_ratio;

	gboolean           shadow_state;
	gdouble            shadow_x;
	gdouble            shadow_y;
	glColorNode       *shadow_color_node;
	gdouble            shadow_opacity;
};

enum {
        CHANGED,
        MOVED,
        LAST_SIGNAL
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static guint instance = 0;

static guint signals[LAST_SIGNAL] = {0};


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void     gl_label_object_finalize  (GObject            *object);

static void     set_size                  (glLabelObject      *object,
					   gdouble             w,
					   gdouble             h,
                                           gboolean            checkpoint);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelObject, gl_label_object, G_TYPE_OBJECT);


static void
gl_label_object_class_init (glLabelObjectClass *class)
{
	GObjectClass       *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_LABEL, "START");

	gl_label_object_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_label_object_finalize;

	class->set_size     = set_size;
	class->draw_handles = gl_label_object_draw_handles_box; /* Default style */
	class->handle_at    = gl_label_object_box_handle_at;    /* Default style */

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
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	gl_debug (DEBUG_LABEL, "END");
}


static void
gl_label_object_init (glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	object->priv = g_new0 (glLabelObjectPrivate, 1);

	object->priv->name = g_strdup_printf ("object%d", instance++);

	cairo_matrix_init_identity (&object->priv->matrix);

	object->priv->shadow_state = FALSE;
	object->priv->shadow_x = DEFAULT_SHADOW_X_OFFSET;
	object->priv->shadow_y = DEFAULT_SHADOW_Y_OFFSET;
	object->priv->shadow_color_node = gl_color_node_new_default ();
        object->priv->shadow_color_node->color = DEFAULT_SHADOW_COLOR;
	object->priv->shadow_opacity = DEFAULT_SHADOW_OPACITY;

	gl_debug (DEBUG_LABEL, "END");
}


static void
gl_label_object_finalize (GObject *object)
{
        glLabelObject *label_object = GL_LABEL_OBJECT (object);

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	g_free (label_object->priv->name);
	g_free (label_object->priv);

	G_OBJECT_CLASS (gl_label_object_parent_class)->finalize (object);

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
        cairo_matrix_t    matrix;
	gboolean          shadow_state;
	gdouble           shadow_x, shadow_y;
	glColorNode      *shadow_color_node;
	gdouble           shadow_opacity;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (src_object && GL_IS_LABEL_OBJECT (src_object), NULL);

	dst_object = g_object_new (G_OBJECT_TYPE(src_object), NULL);

        gl_label_object_set_parent (dst_object, label);
        dst_object->priv->selected_flag = src_object->priv->selected_flag;

	gl_label_object_get_position      (src_object, &x, &y);
	gl_label_object_get_raw_size      (src_object, &w, &h);
	gl_label_object_get_matrix        (src_object, &matrix);
	gl_label_object_get_shadow_offset (src_object, &shadow_x, &shadow_y);
	shadow_color_node = gl_label_object_get_shadow_color   (src_object);
	shadow_opacity    = gl_label_object_get_shadow_opacity (src_object);
	shadow_state      = gl_label_object_get_shadow_state   (src_object);

	gl_label_object_set_position (dst_object, x, y, FALSE);
	gl_label_object_set_size     (dst_object, w, h, FALSE);
	gl_label_object_set_matrix   (dst_object, &matrix);
	gl_label_object_set_shadow_offset  (dst_object, shadow_x, shadow_y, FALSE);
	gl_label_object_set_shadow_color   (dst_object, shadow_color_node, FALSE);
	gl_label_object_set_shadow_opacity (dst_object, shadow_opacity, FALSE);
	gl_label_object_set_shadow_state   (dst_object, shadow_state, FALSE);

	gl_color_node_free (&shadow_color_node);

	if ( GL_LABEL_OBJECT_GET_CLASS(src_object)->copy != NULL )
        {
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
        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));
        g_return_if_fail (label && GL_IS_LABEL (label));

        object->priv->parent = label;
}


/*****************************************************************************/
/* Get parent label of object.                                               */
/*****************************************************************************/
glLabel *
gl_label_object_get_parent (glLabelObject *object)
{
        gl_debug (DEBUG_LABEL, "START");

        g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), NULL);

        gl_debug (DEBUG_LABEL, "END");

        return object->priv->parent;
}


/*****************************************************************************/
/* Select object.                                                            */
/*****************************************************************************/
void
gl_label_object_select (glLabelObject     *object)
{
        object->priv->selected_flag = TRUE;
}


/*****************************************************************************/
/* Unselect object.                                                          */
/*****************************************************************************/
void
gl_label_object_unselect (glLabelObject     *object)
{
        object->priv->selected_flag = FALSE;
}


/*****************************************************************************/
/* Is object selected?                                                       */
/*****************************************************************************/
gboolean
gl_label_object_is_selected (glLabelObject     *object)
{
        return object->priv->selected_flag;
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

	g_free(object->priv->name);
	object->priv->name = name;

        gl_label_object_emit_changed (object);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get name of object.                                                       */
/*****************************************************************************/
gchar *
gl_label_object_get_name (glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), NULL);

	gl_debug (DEBUG_LABEL, "END");

	return g_strdup(object->priv->name);
}


/*****************************************************************************/
/* Set position of object.                                                   */
/*****************************************************************************/
void    
gl_label_object_set_position (glLabelObject *object,
			      gdouble        x,
			      gdouble        y,
                              gboolean       checkpoint)
{
	gdouble  dx, dy;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( (x != object->priv->x) || (y != object->priv->y) )
        {
                if ( checkpoint )
                {
                        gl_label_checkpoint (object->priv->parent, _("Move"));
                }

		dx = x - object->priv->x;
		dy = y - object->priv->y;

		object->priv->x = x;
		object->priv->y = y;

                g_signal_emit (G_OBJECT(object), signals[MOVED], 0);

	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set position of object relative to old position.                          */
/*****************************************************************************/
void    
gl_label_object_set_position_relative (glLabelObject *object,
				       gdouble        dx,
				       gdouble        dy,
                                       gboolean       checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( (dx != 0.0) || (dy != 0.0) )
        {
                if ( checkpoint )
                {
                        gl_label_checkpoint (object->priv->parent, _("Move"));
                }

		object->priv->x += dx;
		object->priv->y += dy;

		gl_debug (DEBUG_LABEL, "       x = %f, y= %f",
			  object->priv->x,
			  object->priv->y);

                g_signal_emit (G_OBJECT(object), signals[MOVED], 0);
	}

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

	*x = object->priv->x;
	*y = object->priv->y;

	gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Default set size method.                                        */
/*---------------------------------------------------------------------------*/
static void
set_size (glLabelObject *object,
	  gdouble        w,
	  gdouble        h,
          gboolean       checkpoint)
{
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( (object->priv->w != w) || (object->priv->h != h) )
        {
                if ( checkpoint )
                {
                        gl_label_checkpoint (object->priv->parent, _("Resize"));
                }

		object->priv->w = w;
		object->priv->h = h;

                gl_label_object_emit_changed (object);
	}
}


/*****************************************************************************/
/* Set size of object.                                                       */
/*****************************************************************************/
void    
gl_label_object_set_size (glLabelObject *object,
			  gdouble        w,
			  gdouble        h,
                          gboolean       checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_size != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_size (object, w, h, checkpoint);

		object->priv->aspect_ratio = h / w;
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set size of object honoring current aspect ratio.                         */
/*****************************************************************************/
void    
gl_label_object_set_size_honor_aspect (glLabelObject *object,
				       gdouble        w,
				       gdouble        h,
                                       gboolean       checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( h > w*object->priv->aspect_ratio )
        {
		h = w * object->priv->aspect_ratio;
	}
        else
        {
		w = h / object->priv->aspect_ratio;
	}

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_size != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_size (object, w, h, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get raw size method (don't let object content adjust size).               */
/*****************************************************************************/
void
gl_label_object_get_raw_size (glLabelObject *object,
                              gdouble       *w,
                              gdouble       *h)
{
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	*w = object->priv->w;
	*h = object->priv->h;
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

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_size != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->get_size (object, w, h);
	}
        else
        {
		gl_label_object_get_raw_size (object, w, h);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get extent of object.                                                     */
/*****************************************************************************/
void
gl_label_object_get_extent (glLabelObject *object,
                            glLabelRegion *region)
{
	gdouble        w, h;
        gdouble        line_w;
	gdouble        xa1, ya1, xa2, ya2, xa3, ya3, xa4, ya4;
        cairo_matrix_t matrix;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_size (object, &w, &h);
        line_w = gl_label_object_get_line_width (object);

	/* setup untransformed corners of bounding box, account for line width */
	xa1 =   - line_w/2;
	ya1 =   - line_w/2;
	xa2 = w + line_w/2;
	ya2 =   - line_w/2;
	xa3 = w + line_w/2;
	ya3 = h + line_w/2;
	xa4 =   - line_w/2;
	ya4 = h + line_w/2;

	/* transform these points */
	gl_label_object_get_matrix (object, &matrix);
        cairo_matrix_transform_point (&matrix, &xa1, &ya1);
        cairo_matrix_transform_point (&matrix, &xa2, &ya2);
        cairo_matrix_transform_point (&matrix, &xa3, &ya3);
        cairo_matrix_transform_point (&matrix, &xa4, &ya4);

	/* now find the maximum extent of these points in x and y */
	region->x1 = MIN (xa1, MIN (xa2, MIN (xa3, xa4))) + object->priv->x;
	region->y1 = MIN (ya1, MIN (ya2, MIN (ya3, ya4))) + object->priv->y;
	region->x2 = MAX (xa1, MAX (xa2, MAX (xa3, xa4))) + object->priv->x;
	region->y2 = MAX (ya1, MAX (ya2, MAX (ya3, ya4))) + object->priv->y;

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Can text properties be set for this object?                               */
/*****************************************************************************/
gboolean
gl_label_object_can_text (glLabelObject     *object)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_font_family != NULL )
        {
		return TRUE;
	}
        else
        {
		return FALSE;
	}
}


/*****************************************************************************/
/* Set font family for all text contained in object.                         */
/*****************************************************************************/
void    
gl_label_object_set_font_family (glLabelObject     *object,
				 const gchar       *font_family,
                                 gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_font_family != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_font_family (object, font_family, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set font size for all text contained in object.                          */
/****************************************************************************/
void    
gl_label_object_set_font_size (glLabelObject     *object,
			       gdouble            font_size,
                               gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_font_size != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_font_size (object, font_size, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set font weight for all text contained in object.                        */
/****************************************************************************/
void    
gl_label_object_set_font_weight (glLabelObject     *object,
				 PangoWeight        font_weight,
                                 gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_font_weight != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_font_weight (object, font_weight, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set font italic flag for all text contained in object.                   */
/****************************************************************************/
void    
gl_label_object_set_font_italic_flag (glLabelObject     *object,
				      gboolean           font_italic_flag,
                                      gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_font_italic_flag != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_font_italic_flag (object, font_italic_flag, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set text alignment for all text contained in object.                     */
/****************************************************************************/
void    
gl_label_object_set_text_alignment (glLabelObject     *object,
				    PangoAlignment     text_alignment,
                                    gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_text_alignment != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_text_alignment (object, text_alignment, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set text line spacing for all text contained in object.                  */
/****************************************************************************/
void    
gl_label_object_set_text_line_spacing (glLabelObject     *object,
			               gdouble            text_line_spacing,
                                       gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_text_line_spacing != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_text_line_spacing (object, text_line_spacing, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set text color for all text contained in object.                         */
/****************************************************************************/
void    
gl_label_object_set_text_color (glLabelObject     *object,
				glColorNode       *text_color_node,
                                gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_text_color != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_text_color (object, text_color_node, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get font family for all text contained in object.                         */
/*****************************************************************************/
gchar *
gl_label_object_get_font_family (glLabelObject     *object)
{
	gchar *ret = NULL;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), NULL);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_font_family != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_font_family (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/****************************************************************************/
/* Get font size for all text contained in object.                          */
/****************************************************************************/
gdouble
gl_label_object_get_font_size (glLabelObject     *object)
{
	gdouble ret = 0.0;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), 0.0);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_font_size != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_font_size (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/****************************************************************************/
/* Get font weight for all text contained in object.                        */
/****************************************************************************/
PangoWeight    
gl_label_object_get_font_weight (glLabelObject     *object)
{
	PangoWeight     ret = PANGO_WEIGHT_NORMAL;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), PANGO_WEIGHT_NORMAL);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_font_weight != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_font_weight (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/****************************************************************************/
/* Get font italic flag for all text contained in object.                   */
/****************************************************************************/
gboolean
gl_label_object_get_font_italic_flag (glLabelObject     *object)
{
	gboolean ret = FALSE;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_font_italic_flag != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_font_italic_flag (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/****************************************************************************/
/* Get text alignment for all text contained in object.                     */
/****************************************************************************/
PangoAlignment
gl_label_object_get_text_alignment (glLabelObject     *object)
{
	PangoAlignment ret = PANGO_ALIGN_LEFT;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), PANGO_ALIGN_LEFT);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_text_alignment != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_text_alignment (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/****************************************************************************/
/* Get text line spacing for all text contained in object.                  */
/****************************************************************************/
gdouble
gl_label_object_get_text_line_spacing (glLabelObject     *object)
{
	gdouble ret = 0.0;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), 0.0);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_text_line_spacing != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_text_line_spacing (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/****************************************************************************/
/* Get text color for all text contained in object.                         */
/****************************************************************************/
glColorNode*
gl_label_object_get_text_color (glLabelObject     *object)
{
	glColorNode *ret = NULL;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), 0);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_text_color != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_text_color (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/*****************************************************************************/
/* Can fill properties be set for this object?                               */
/*****************************************************************************/
gboolean
gl_label_object_can_fill (glLabelObject     *object)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_fill_color != NULL )
        {
		return TRUE;
	}
        else
        {
		return FALSE;
	}

}


/****************************************************************************/
/* Set fill color for object.                                               */
/****************************************************************************/
void    
gl_label_object_set_fill_color (glLabelObject     *object,
				glColorNode       *fill_color_node,
                                gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_fill_color != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_fill_color (object, fill_color_node, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get fill color for object.                                               */
/****************************************************************************/
glColorNode*
gl_label_object_get_fill_color (glLabelObject     *object)
{
	glColorNode *ret = NULL;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), 0);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_fill_color != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_fill_color (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/*****************************************************************************/
/* Can line color property be set for this object?                           */
/*****************************************************************************/
gboolean
gl_label_object_can_line_color (glLabelObject     *object)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_line_color != NULL )
        {
		return TRUE;
	}
        else
        {
		return FALSE;
	}

}


/****************************************************************************/
/* Set line color for object.                                               */
/****************************************************************************/
void    
gl_label_object_set_line_color (glLabelObject     *object,
				glColorNode       *line_color_node,
                                gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_line_color != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_line_color (object, line_color_node, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get line color for object.                                               */
/****************************************************************************/
glColorNode*
gl_label_object_get_line_color (glLabelObject     *object)
{
	glColorNode *ret = NULL;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), 0);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_line_color != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_line_color (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/*****************************************************************************/
/* Can line width property be set for this object?                           */
/*****************************************************************************/
gboolean
gl_label_object_can_line_width (glLabelObject     *object)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_line_width != NULL )
        {
		return TRUE;
	}
        else
        {
		return FALSE;
	}

}


/****************************************************************************/
/* Set line width for object.                                               */
/****************************************************************************/
void    
gl_label_object_set_line_width (glLabelObject     *object,
				gdouble            line_width,
                                gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->set_line_width != NULL )
        {
		/* We have an object specific method, use it */
		GL_LABEL_OBJECT_GET_CLASS(object)->set_line_width (object, line_width, checkpoint);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get line width for object.                                               */
/****************************************************************************/
gdouble
gl_label_object_get_line_width (glLabelObject     *object)
{
	gdouble ret = 0.0;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), 0.0);

	if ( GL_LABEL_OBJECT_GET_CLASS(object)->get_line_width != NULL )
        {
		/* We have an object specific method, use it */
		ret = GL_LABEL_OBJECT_GET_CLASS(object)->get_line_width (object);
	}

	gl_debug (DEBUG_LABEL, "END");

	return ret;
}


/****************************************************************************/
/* Set shadow state of object.                                              */
/****************************************************************************/
void    
gl_label_object_set_shadow_state (glLabelObject     *object,
				  gboolean           state,
                                  gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if (object->priv->shadow_state != state)
	{
                if ( checkpoint )
                {
                        gl_label_checkpoint (object->priv->parent, _("Shadow state"));
                }

		object->priv->shadow_state = state;

		gl_label_object_emit_changed (object);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get shadow state of object.                                              */
/****************************************************************************/
gboolean
gl_label_object_get_shadow_state (glLabelObject     *object)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

	return object->priv->shadow_state;
}


/****************************************************************************/
/* Set offset of object's shadow.                                           */
/****************************************************************************/
void    
gl_label_object_set_shadow_offset (glLabelObject     *object,
				   gdouble            x,
				   gdouble            y,
                                   gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( (x != object->priv->shadow_x) || (y != object->priv->shadow_y) )
	{
                if ( checkpoint )
                {
                        gl_label_checkpoint (object->priv->parent, _("Shadow offset"));
                }

		object->priv->shadow_x = x;
		object->priv->shadow_y = y;

		gl_label_object_emit_changed (object);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get offset of object's shadow.                                           */
/****************************************************************************/
void
gl_label_object_get_shadow_offset (glLabelObject     *object,
				   gdouble           *x,
				   gdouble           *y)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	*x = object->priv->shadow_x;
	*y = object->priv->shadow_y;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set color of object's shadow.                                            */
/****************************************************************************/
void    
gl_label_object_set_shadow_color (glLabelObject     *object,
				  glColorNode       *color_node,
                                  gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if ( !gl_color_node_equal (object->priv->shadow_color_node, color_node ))
	{
                if ( checkpoint )
                {
                        gl_label_checkpoint (object->priv->parent, _("Shadow color"));
                }

		gl_color_node_free (&(object->priv->shadow_color_node));
		object->priv->shadow_color_node = gl_color_node_dup (color_node);

		gl_label_object_emit_changed (object);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get color of object's shadow.                                            */
/****************************************************************************/
glColorNode*
gl_label_object_get_shadow_color (glLabelObject     *object)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), NULL);

	return gl_color_node_dup (object->priv->shadow_color_node);
}


/****************************************************************************/
/* Set opacity of object's shadow.                                          */
/****************************************************************************/
void    
gl_label_object_set_shadow_opacity (glLabelObject     *object,
				    gdouble            alpha,
                                    gboolean           checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	if (object->priv->shadow_opacity != alpha)
	{
                if ( checkpoint )
                {
                        gl_label_checkpoint (object->priv->parent, _("Shadow opacity"));
                }

                object->priv->shadow_opacity = alpha;

		gl_label_object_emit_changed (object);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get opacity of object's shadow.                                          */
/****************************************************************************/
gdouble
gl_label_object_get_shadow_opacity (glLabelObject     *object)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

	return object->priv->shadow_opacity;
}


/****************************************************************************/
/* Flip object horizontally.                                                */
/****************************************************************************/
void
gl_label_object_flip_horiz (glLabelObject *object)
{
        cairo_matrix_t flip_matrix;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        cairo_matrix_init_scale (&flip_matrix, -1.0, 1.0);
        cairo_matrix_multiply (&object->priv->matrix, &object->priv->matrix, &flip_matrix);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Flip object vertically.                                                  */
/****************************************************************************/
void
gl_label_object_flip_vert (glLabelObject *object)
{
        cairo_matrix_t flip_matrix;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        cairo_matrix_init_scale (&flip_matrix, 1.0, -1.0);
        cairo_matrix_multiply (&object->priv->matrix, &object->priv->matrix, &flip_matrix);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Rotate object.                                                           */
/****************************************************************************/
void
gl_label_object_rotate (glLabelObject *object,
			gdouble        theta_degs)
{
        cairo_matrix_t rotate_matrix;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        cairo_matrix_init_rotate (&rotate_matrix, theta_degs*(G_PI/180.));
        cairo_matrix_multiply (&object->priv->matrix, &object->priv->matrix, &rotate_matrix);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set raw affine                                                           */
/****************************************************************************/
void
gl_label_object_set_matrix (glLabelObject  *object,
                            cairo_matrix_t *matrix)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        object->priv->matrix = *matrix;
}


/****************************************************************************/
/* Get raw affine                                                           */
/****************************************************************************/
void
gl_label_object_get_matrix (glLabelObject  *object,
                            cairo_matrix_t *matrix)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        *matrix = object->priv->matrix;
}


/*****************************************************************************/
/* Draw object                                                               */
/*****************************************************************************/
void
gl_label_object_draw (glLabelObject *object,
                      cairo_t       *cr,
                      gboolean       screen_flag,
                      glMergeRecord *record)

{
        gdouble        x0, y0;
        cairo_matrix_t matrix;
	gboolean       shadow_state;
	gdouble        shadow_x, shadow_y;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        gl_label_object_get_position (object, &x0, &y0);
        gl_label_object_get_matrix (object, &matrix);

        cairo_save (cr);
        cairo_translate (cr, x0, y0);

        if ( GL_LABEL_OBJECT_GET_CLASS(object)->draw_shadow != NULL )
        {
                shadow_state = gl_label_object_get_shadow_state (object);

                if ( shadow_state )
                {
                        gl_label_object_get_shadow_offset (object, &shadow_x, &shadow_y);

                        cairo_save (cr);
                        cairo_translate (cr, shadow_x, shadow_y);
                        cairo_transform (cr, &matrix);

                        GL_LABEL_OBJECT_GET_CLASS(object)->draw_shadow (object,
                                                                        cr,
                                                                        screen_flag,
                                                                        record);

                        cairo_restore (cr);
                }
        }

        if ( GL_LABEL_OBJECT_GET_CLASS(object)->draw_object != NULL )
        {
                cairo_save (cr);
                cairo_transform (cr, &matrix);

                GL_LABEL_OBJECT_GET_CLASS(object)->draw_object (object,
                                                                cr,
                                                                screen_flag,
                                                                record);

                cairo_restore (cr);
        }

        cairo_restore (cr);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Is object located at coordinates.                                         */
/*****************************************************************************/
gboolean
gl_label_object_is_located_at (glLabelObject     *object,
                               cairo_t           *cr,
                               gdouble            x_pixels,
                               gdouble            y_pixels)
{
        gboolean            ret_val = FALSE;
        gdouble             x0, y0;
        cairo_matrix_t      matrix;
        gdouble             x, y;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

        if ( GL_LABEL_OBJECT_GET_CLASS(object)->draw_object != NULL )
        {

                gl_label_object_get_position (object, &x0, &y0);
                gl_label_object_get_matrix (object, &matrix);

                cairo_save (cr);

                cairo_translate (cr, x0, y0);
                cairo_transform (cr, &matrix);

                x = x_pixels;
                y = y_pixels;
                cairo_device_to_user (cr, &x, &y);

                ret_val = GL_LABEL_OBJECT_GET_CLASS(object)->object_at (object, cr, x, y);

                cairo_restore (cr);
        }

	gl_debug (DEBUG_LABEL, "END");

        return ret_val;
}


/*****************************************************************************/
/* Draw object handles                                                       */
/*****************************************************************************/
void
gl_label_object_draw_handles (glLabelObject     *object,
                              cairo_t           *cr)
{
        gdouble        x0, y0;
        cairo_matrix_t matrix;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        if ( GL_LABEL_OBJECT_GET_CLASS(object)->draw_object != NULL )
        {

                gl_label_object_get_position (object, &x0, &y0);
                gl_label_object_get_matrix (object, &matrix);

                cairo_save (cr);

                cairo_translate (cr, x0, y0);
                cairo_transform (cr, &matrix);

                GL_LABEL_OBJECT_GET_CLASS(object)->draw_handles (object, cr);

                cairo_restore (cr);
        }

	gl_debug (DEBUG_LABEL, "END");
}


/*--------------------------------------------------------------------------*/
/* Draw individual handle                                                   */
/*--------------------------------------------------------------------------*/
static void
draw_handle (cairo_t                *cr,
             glLabelObject          *object,
             gdouble                 x_handle,
             gdouble                 y_handle)
{
        gdouble scale_x, scale_y;


        gl_debug (DEBUG_VIEW, "START");

        cairo_save (cr);

        cairo_translate (cr, x_handle, y_handle);

        scale_x = 1.0;
        scale_y = 1.0;
        cairo_device_to_user_distance (cr, &scale_x, &scale_y);
        cairo_scale (cr, scale_x, scale_y);

        cairo_rectangle (cr,
                         -HANDLE_PIXELS/2.0, -HANDLE_PIXELS/2.0,
                         HANDLE_PIXELS, HANDLE_PIXELS);

        cairo_set_source_rgba (cr, HANDLE_FILL_RGBA_ARGS);
        cairo_fill_preserve (cr);
                               
        cairo_set_line_width (cr, HANDLE_OUTLINE_WIDTH_PIXELS);
        cairo_set_source_rgba (cr, HANDLE_OUTLINE_RGBA_ARGS);
        cairo_stroke (cr);

        cairo_restore (cr);

        gl_debug (DEBUG_VIEW, "END");
}


/*--------------------------------------------------------------------------*/
/* Create handle path                                                       */
/*--------------------------------------------------------------------------*/
static void
create_handle_path (cairo_t                *cr,
                    glLabelObject          *object,
                    gdouble                 x_handle,
                    gdouble                 y_handle)
{
        gdouble scale_x, scale_y;


        gl_debug (DEBUG_VIEW, "START");

        cairo_save (cr);

        cairo_translate (cr, x_handle, y_handle);

        scale_x = 1.0;
        scale_y = 1.0;
        cairo_device_to_user_distance (cr, &scale_x, &scale_y);
        cairo_scale (cr, scale_x, scale_y);

        cairo_rectangle (cr,
                         -HANDLE_PIXELS/2.0, -HANDLE_PIXELS/2.0,
                         HANDLE_PIXELS, HANDLE_PIXELS);

        cairo_restore (cr);

        gl_debug (DEBUG_VIEW, "END");
}


/*****************************************************************************/
/* Draw box style handles.                                                   */
/*****************************************************************************/
void
gl_label_object_draw_handles_box (glLabelObject     *object,
                                  cairo_t           *cr)
{
        gdouble w, h;

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

        /* North */
        draw_handle (cr, object, w/2, 0);

        /* North East */
        draw_handle (cr, object, w, 0);

        /* East */
        draw_handle (cr, object, w, h/2);

        /* South East */
        draw_handle (cr, object, w, h);

        /* South */
        draw_handle (cr, object, w/2, h);

        /* South West */
        draw_handle (cr, object, 0, h);

        /* West */
        draw_handle (cr, object, 0, h/2);

        /* North West */
        draw_handle (cr, object, 0, 0);
}


/*****************************************************************************/
/* Draw line style handles.                                                  */
/*****************************************************************************/
void
gl_label_object_draw_handles_line (glLabelObject     *object,
                                   cairo_t           *cr)
{
        gdouble w, h;

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

        /* P2 */
        draw_handle (cr, object, w, h);

        /* P1 */
        draw_handle (cr, object, 0, 0);
}


/*****************************************************************************/
/* Get handle at given coordinates, if any.                                  */
/*****************************************************************************/
glLabelObjectHandle
gl_label_object_handle_at (glLabelObject     *object,
                           cairo_t           *cr,
                           gdouble            x_pixels,
                           gdouble            y_pixels)
{
        glLabelObjectHandle handle = GL_LABEL_OBJECT_HANDLE_NONE;
        gdouble             x0, y0;
        cairo_matrix_t      matrix;
        gdouble             x, y;

	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), handle);

        if ( GL_LABEL_OBJECT_GET_CLASS(object)->draw_object != NULL )
        {

                gl_label_object_get_position (object, &x0, &y0);
                gl_label_object_get_matrix (object, &matrix);

                cairo_save (cr);

                cairo_translate (cr, x0, y0);
                cairo_transform (cr, &matrix);

                x = x_pixels;
                y = y_pixels;
                cairo_device_to_user (cr, &x, &y);

                handle = GL_LABEL_OBJECT_GET_CLASS(object)->handle_at (object, cr, x, y);

                cairo_restore (cr);
        }

	gl_debug (DEBUG_LABEL, "END");

        return handle;
}


/*****************************************************************************/
/* Get handle at given coordinates for box style handles, if any.            */
/*****************************************************************************/
glLabelObjectHandle
gl_label_object_box_handle_at (glLabelObject     *object,
                               cairo_t           *cr,
                               gdouble            x,
                               gdouble            y)
{
        gdouble w, h;

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

        /* South East */
        create_handle_path (cr, object, w, h);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_SE;
        }
        
        /* South West */
        create_handle_path (cr, object, 0, h);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_SW;
        }

        /* North East */
        create_handle_path (cr, object, w, 0);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_NE;
        }

        /* North West */
        create_handle_path (cr, object, 0, 0);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_NW;
        }

        /* East */
        create_handle_path (cr, object, w, h/2);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_E;
        }

        /* South */
        create_handle_path (cr, object, w/2, h);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_S;
        }

        /* West */
        create_handle_path (cr, object, 0, h/2);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_W;
        }

        /* North */
        create_handle_path (cr, object, w/2, 0);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_N;
        }

        return GL_LABEL_OBJECT_HANDLE_NONE;
}


/*****************************************************************************/
/* Get handle at given coordinates for line style handles, if any.           */
/*****************************************************************************/
glLabelObjectHandle
gl_label_object_line_handle_at (glLabelObject     *object,
                                cairo_t           *cr,
                                gdouble            x,
                                gdouble            y)
{
        gdouble w, h;

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

        /* P2 */
        create_handle_path (cr, object, w, h);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_P2;
        }
        
        /* P1 */
        create_handle_path (cr, object, 0, h);
        if (cairo_in_fill (cr, x, y))
        {
                return GL_LABEL_OBJECT_HANDLE_P1;
        }

        return GL_LABEL_OBJECT_HANDLE_NONE;
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
