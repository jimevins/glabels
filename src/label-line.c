/*
 *  label-line.c
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

#include "label-line.h"

#include <glib.h>

#include "debug.h"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelLinePrivate {
	gdouble          line_width;
	glColorNode     *line_color_node;
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void    gl_label_line_finalize     (GObject           *object);

static void    copy                       (glLabelObject     *dst_object,
					   glLabelObject     *src_object);

static void    set_line_color             (glLabelObject     *object,
					   glColorNode       *line_color_node);

static void    set_line_width             (glLabelObject     *object,
					   gdouble            line_width);

static glColorNode   *get_line_color      (glLabelObject     *object);

static gdouble get_line_width             (glLabelObject     *object);

static void    draw_object                (glLabelObject     *object,
                                           cairo_t           *cr,
                                           gboolean           screen_flag,
                                           glMergeRecord     *record);

static void    draw_shadow                (glLabelObject     *object,
                                           cairo_t           *cr,
                                           gboolean           screen_flag,
                                           glMergeRecord     *record);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelLine, gl_label_line, GL_TYPE_LABEL_OBJECT);


static void
gl_label_line_class_init (glLabelLineClass *class)
{
	GObjectClass       *object_class       = G_OBJECT_CLASS (class);
	glLabelObjectClass *label_object_class = GL_LABEL_OBJECT_CLASS (class);

	gl_label_line_parent_class = g_type_class_peek_parent (class);

	label_object_class->copy           = copy;
	label_object_class->set_line_color = set_line_color;
	label_object_class->set_line_width = set_line_width;
	label_object_class->get_line_color = get_line_color;
	label_object_class->get_line_width = get_line_width;
        label_object_class->draw_object    = draw_object;
        label_object_class->draw_shadow    = draw_shadow;

	object_class->finalize = gl_label_line_finalize;
}


static void
gl_label_line_init (glLabelLine *lline)
{
	lline->priv = g_new0 (glLabelLinePrivate, 1);
	lline->priv->line_color_node = gl_color_node_new_default ();
}


static void
gl_label_line_finalize (GObject *object)
{
	glLabelLine *lline = GL_LABEL_LINE (object);

	g_return_if_fail (object && GL_IS_LABEL_LINE (object));

	gl_color_node_free (&(lline->priv->line_color_node));
	g_free (lline->priv);

	G_OBJECT_CLASS (gl_label_line_parent_class)->finalize (object);
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
	glColorNode *line_color_node;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lline && GL_IS_LABEL_LINE (lline));
	g_return_if_fail (new_lline && GL_IS_LABEL_LINE (new_lline));

	line_width = get_line_width (src_object);
	line_color_node = get_line_color (src_object);

	set_line_width (dst_object, line_width);
	set_line_color (dst_object, line_color_node);

	gl_color_node_free (&line_color_node);
	
	gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_color (glLabelObject *object,
		glColorNode   *line_color_node)
{
	glLabelLine *lline = (glLabelLine *)object;

	g_return_if_fail (lline && GL_IS_LABEL_LINE (lline));

	if ( !gl_color_node_equal (lline->priv->line_color_node, line_color_node)) {
		
		gl_color_node_free (&(lline->priv->line_color_node ));
		lline->priv->line_color_node = gl_color_node_dup (line_color_node);
		
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

	if ( lline->priv->line_width != line_width ) {
		lline->priv->line_width = line_width;
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

	return lline->priv->line_width;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line width method.                                          */
/*---------------------------------------------------------------------------*/
static glColorNode*
get_line_color (glLabelObject *object)
{
	glLabelLine *lline = (glLabelLine *)object;

	g_return_val_if_fail (lline && GL_IS_LABEL_LINE (lline), 0);

	return gl_color_node_dup (lline->priv->line_color_node);
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
        gdouble        w, h;
	gdouble        line_width;
	glColorNode   *line_color_node;
        guint          line_color;

	gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

	line_width = gl_label_object_get_line_width (object);
	
	line_color_node = gl_label_object_get_line_color (object);
        line_color = gl_color_node_expand (line_color_node, record);
        if (line_color_node->field_flag && screen_flag)
        {
                line_color = GL_COLOR_MERGE_DEFAULT;
        }


        cairo_move_to (cr, 0.0, 0.0);
        cairo_line_to (cr, w, h);


	/* Draw line */
        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (line_color));
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);

	gl_color_node_free (&line_color_node);

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
        gdouble        w, h;
	gdouble        line_width;
	glColorNode   *line_color_node;
	glColorNode   *shadow_color_node;
	gdouble        shadow_opacity;
	guint          shadow_line_color;

	gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

	line_width = gl_label_object_get_line_width (object);
	
	line_color_node = gl_label_object_get_line_color (object);
        if (line_color_node->field_flag)
        {
                line_color_node->color = GL_COLOR_MERGE_DEFAULT;
        }

	shadow_color_node = gl_label_object_get_shadow_color (object);
	if (shadow_color_node->field_flag)
	{
		shadow_color_node->color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (object);
	shadow_line_color = gl_color_shadow (shadow_color_node->color, shadow_opacity, line_color_node->color);


        cairo_move_to (cr, 0.0, 0.0);
        cairo_line_to (cr, w, h);


        /* Draw outline shadow */
        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (shadow_line_color));
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);


	gl_color_node_free (&line_color_node);
	gl_color_node_free (&shadow_color_node);

	gl_debug (DEBUG_LABEL, "END");
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
