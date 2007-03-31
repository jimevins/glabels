/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_box.c:  GLabels label box object
 *
 *  Copyright (C) 2001-2007  Jim Evins <evins@snaught.com>.
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

#include "label-box.h"

#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include <glib/gmessages.h>

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelBoxPrivate {
	gdouble          line_width;
	glColorNode      *line_color_node;
	glColorNode      *fill_color_node;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void    gl_label_box_finalize      (GObject         *object);

static void    copy                       (glLabelObject   *dst_object,
					   glLabelObject   *src_object);

static void    set_fill_color             (glLabelObject   *object,
					   glColorNode     *fill_color_node);

static void    set_line_color             (glLabelObject   *object,
					   glColorNode     *line_color_node);

static void    set_line_width             (glLabelObject   *object,
					   gdouble          line_width);

static glColorNode*   get_fill_color      (glLabelObject   *object);

static glColorNode*   get_line_color      (glLabelObject   *object);

static gdouble get_line_width             (glLabelObject   *object);

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
G_DEFINE_TYPE (glLabelBox, gl_label_box, GL_TYPE_LABEL_OBJECT);

static void
gl_label_box_class_init (glLabelBoxClass *class)
{
	GObjectClass       *object_class       = G_OBJECT_CLASS (class);
	glLabelObjectClass *label_object_class = GL_LABEL_OBJECT_CLASS (class);

	gl_label_box_parent_class = g_type_class_peek_parent (class);

	label_object_class->copy           = copy;
	label_object_class->set_fill_color = set_fill_color;
	label_object_class->set_line_color = set_line_color;
	label_object_class->set_line_width = set_line_width;
	label_object_class->get_fill_color = get_fill_color;
	label_object_class->get_line_color = get_line_color;
	label_object_class->get_line_width = get_line_width;
        label_object_class->draw_object    = draw_object;
        label_object_class->draw_shadow    = draw_shadow;

	object_class->finalize = gl_label_box_finalize;
}

static void
gl_label_box_init (glLabelBox *lbox)
{
	lbox->priv = g_new0 (glLabelBoxPrivate, 1);
	lbox->priv->line_color_node = gl_color_node_new_default ();
	lbox->priv->fill_color_node = gl_color_node_new_default ();
}

static void
gl_label_box_finalize (GObject *object)
{
	glLabelBox *lbox = GL_LABEL_BOX (object);

	g_return_if_fail (object && GL_IS_LABEL_BOX (object));

	gl_color_node_free (&(lbox->priv->fill_color_node));
	gl_color_node_free (&(lbox->priv->line_color_node));
	g_free (lbox->priv);

	G_OBJECT_CLASS (gl_label_box_parent_class)->finalize (object);
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
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
	glLabelBox  *lbox     = (glLabelBox *)src_object;
	glLabelBox  *new_lbox = (glLabelBox *)dst_object;
	gdouble      line_width;
	glColorNode *line_color_node;
	glColorNode *fill_color_node;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbox && GL_IS_LABEL_BOX (lbox));
	g_return_if_fail (new_lbox && GL_IS_LABEL_BOX (new_lbox));

	line_width = get_line_width (src_object);
	line_color_node = get_line_color (src_object);
	fill_color_node = get_fill_color (src_object);

	set_line_width (dst_object, line_width);
	set_line_color (dst_object, line_color_node);
	set_fill_color (dst_object, fill_color_node);
	
	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);

	gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set fill color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_fill_color (glLabelObject *object,
		glColorNode   *fill_color_node)
{
	glLabelBox *lbox = (glLabelBox *)object;
	
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbox && GL_IS_LABEL_BOX (lbox));

	if (!gl_color_node_equal (lbox->priv->fill_color_node, fill_color_node)) {

		gl_color_node_free (&(lbox->priv->fill_color_node));
		lbox->priv->fill_color_node = gl_color_node_dup (fill_color_node);

		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbox));
	}
	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_color (glLabelObject *object,
		glColorNode   *line_color_node)
{
	glLabelBox *lbox = (glLabelBox *)object;

	g_return_if_fail (lbox && GL_IS_LABEL_BOX (lbox));

	if ( !gl_color_node_equal (lbox->priv->line_color_node, line_color_node )) {
		gl_color_node_free (&(lbox->priv->line_color_node));
		lbox->priv->line_color_node = gl_color_node_dup (line_color_node);
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbox));
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line width method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_width (glLabelObject *object,
		gdouble        line_width)
{
	glLabelBox *lbox = (glLabelBox *)object;

	g_return_if_fail (lbox && GL_IS_LABEL_BOX (lbox));

	if ( lbox->priv->line_width != line_width ) {
		lbox->priv->line_width = line_width;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbox));
	}
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get fill color method.                                          */
/*---------------------------------------------------------------------------*/
static gdouble
get_line_width (glLabelObject *object)
{
	glLabelBox *lbox = (glLabelBox *)object;

	g_return_val_if_fail (lbox && GL_IS_LABEL_BOX (lbox), 0.0);

	return lbox->priv->line_width;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line color method.                                          */
/*---------------------------------------------------------------------------*/
static glColorNode*
get_line_color (glLabelObject *object)
{
	glLabelBox *lbox = (glLabelBox *)object;

	g_return_val_if_fail (lbox && GL_IS_LABEL_BOX (lbox), 0);

	return gl_color_node_dup (lbox->priv->line_color_node);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line width method.                                          */
/*---------------------------------------------------------------------------*/
static glColorNode*
get_fill_color (glLabelObject *object)
{
	glLabelBox *lbox = (glLabelBox *)object;

	g_return_val_if_fail (lbox && GL_IS_LABEL_BOX (lbox), 0);

	return gl_color_node_dup (lbox->priv->fill_color_node);
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
	glColorNode   *fill_color_node;
        guint          line_color;
        guint          fill_color;

	gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_size (object, &w, &h);

	line_width = gl_label_object_get_line_width (object);
	
	line_color_node = gl_label_object_get_line_color (object);
	fill_color_node = gl_label_object_get_fill_color (object);
        line_color = gl_color_node_expand (line_color_node, record);
        fill_color = gl_color_node_expand (fill_color_node, record);
        if (line_color_node->field_flag && screen_flag)
        {
                line_color = GL_COLOR_MERGE_DEFAULT;
        }
        if (fill_color_node->field_flag && screen_flag)
        {
                fill_color = GL_COLOR_FILL_MERGE_DEFAULT;
        }


        cairo_rectangle (cr, 0.0, 0.0, w, h);

	/* Paint fill color */
        cairo_set_source_rgba (cr,
                               GL_COLOR_F_RED (fill_color),
                               GL_COLOR_F_GREEN (fill_color),
                               GL_COLOR_F_BLUE (fill_color),
                               GL_COLOR_F_ALPHA (fill_color));
	cairo_fill_preserve (cr);

	/* Draw outline */
        cairo_set_source_rgba (cr,
                               GL_COLOR_F_RED (line_color),
                               GL_COLOR_F_GREEN (line_color),
                               GL_COLOR_F_BLUE (line_color),
                               GL_COLOR_F_ALPHA (line_color));
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);

	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);

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
	glColorNode   *fill_color_node;
        guint          line_color;
        guint          fill_color;
	glColorNode   *shadow_color_node;
        guint          shadow_color;
	gdouble        shadow_opacity;
	guint          shadow_line_color;
	guint          shadow_fill_color;

	gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_size (object, &w, &h);

	line_width = gl_label_object_get_line_width (object);
	
	line_color_node = gl_label_object_get_line_color (object);
	fill_color_node = gl_label_object_get_fill_color (object);
        line_color = gl_color_node_expand (line_color_node, record);
        fill_color = gl_color_node_expand (fill_color_node, record);
        if (line_color_node->field_flag && screen_flag)
        {
                line_color = GL_COLOR_MERGE_DEFAULT;
        }
        if (fill_color_node->field_flag && screen_flag)
        {
                fill_color = GL_COLOR_FILL_MERGE_DEFAULT;
        }

	shadow_color_node = gl_label_object_get_shadow_color (object);
        shadow_color = gl_color_node_expand (shadow_color_node, record);
	if (shadow_color_node->field_flag && screen_flag)
	{
		shadow_color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (object);
	shadow_line_color = gl_color_shadow (shadow_color, shadow_opacity, line_color_node->color);
	shadow_fill_color = gl_color_shadow (shadow_color, shadow_opacity, fill_color_node->color);
	

        cairo_rectangle (cr, 0.0, 0.0, w, h);


        /* Draw fill shadow */
        cairo_set_source_rgba (cr,
                               GL_COLOR_F_RED (shadow_fill_color),
                               GL_COLOR_F_GREEN (shadow_fill_color),
                               GL_COLOR_F_BLUE (shadow_fill_color),
                               GL_COLOR_F_ALPHA (shadow_fill_color));
        cairo_fill_preserve (cr);

        /* Draw outline shadow */
        cairo_set_source_rgba (cr,
                               GL_COLOR_F_RED (shadow_line_color),
                               GL_COLOR_F_GREEN (shadow_line_color),
                               GL_COLOR_F_BLUE (shadow_line_color),
                               GL_COLOR_F_ALPHA (shadow_line_color));
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);


	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);
	gl_color_node_free (&shadow_color_node);

	gl_debug (DEBUG_LABEL, "END");
}

