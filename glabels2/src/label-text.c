/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_text.c:  GLabels label text object
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

#include "label-text.h"

#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include <glib/gmessages.h>
#include <pango/pango.h>
#include <math.h>

#include "util.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define DEFAULT_FONT_FAMILY       "Sans"
#define DEFAULT_FONT_SIZE         14.0
#define DEFAULT_FONT_WEIGHT       PANGO_WEIGHT_NORMAL
#define DEFAULT_FONT_ITALIC_FLAG  FALSE
#define DEFAULT_ALIGN             PANGO_ALIGN_LEFT
#define DEFAULT_COLOR             GL_COLOR (0,0,0)
#define DEFAULT_TEXT_LINE_SPACING 1.0
#define DEFAULT_AUTO_SHRINK       FALSE

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelTextPrivate {
	GtkTextTagTable *tag_table;
	GtkTextBuffer   *buffer;

	gchar           *font_family;
	gdouble          font_size;
	PangoWeight      font_weight;
	gboolean         font_italic_flag;
	PangoAlignment   align;
	glColorNode     *color_node;
	gdouble          line_spacing;
	gboolean         auto_shrink;

        gboolean         size_changed;
        gdouble          w;
        gdouble          h;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_text_finalize      (GObject          *object);

static void copy                        (glLabelObject    *dst_object,
					 glLabelObject    *src_object);

static void buffer_changed_cb           (GtkTextBuffer    *textbuffer,
					 glLabelText      *ltext);

static void get_size                    (glLabelObject    *object,
					 gdouble          *w,
					 gdouble          *h);

static void set_font_family             (glLabelObject    *object,
					 const gchar      *font_family);

static void set_font_size               (glLabelObject    *object,
					 gdouble           font_size);

static void set_font_weight             (glLabelObject    *object,
					 PangoWeight       font_weight);

static void set_font_italic_flag        (glLabelObject    *object,
					 gboolean          font_italic_flag);

static void set_text_alignment          (glLabelObject    *object,
					 PangoAlignment    text_alignment);

static void set_text_line_spacing       (glLabelObject    *object,
					 gdouble           text_line_spacing);

static void set_text_color              (glLabelObject    *object,
					 glColorNode      *text_color_node);

static gchar          *get_font_family             (glLabelObject    *object);

static gdouble         get_font_size               (glLabelObject    *object);

static PangoWeight     get_font_weight             (glLabelObject    *object);

static gboolean        get_font_italic_flag        (glLabelObject    *object);

static PangoAlignment  get_text_alignment          (glLabelObject    *object);

static gdouble         get_text_line_spacing       (glLabelObject    *object);

static glColorNode*    get_text_color              (glLabelObject    *object);

static void            draw_object                 (glLabelObject    *object,
                                                    cairo_t          *cr,
                                                    gboolean          screen_flag,
                                                    glMergeRecord    *record);

static void            draw_shadow                 (glLabelObject    *object,
                                                    cairo_t          *cr,
                                                    gboolean          screen_flag,
                                                    glMergeRecord    *record);

static gdouble         auto_shrink_font_size       (cairo_t          *cr,
                                                    gchar            *family,
                                                    gdouble           size,
                                                    PangoWeight       weight,
                                                    PangoStyle        style,
                                                    gchar            *text,
                                                    gdouble           width);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelText, gl_label_text, GL_TYPE_LABEL_OBJECT);

static void
gl_label_text_class_init (glLabelTextClass *class)
{
	GObjectClass       *object_class       = G_OBJECT_CLASS (class);
	glLabelObjectClass *label_object_class = GL_LABEL_OBJECT_CLASS (class);

	gl_label_text_parent_class = g_type_class_peek_parent (class);

	label_object_class->copy                  = copy;

	label_object_class->get_size              = get_size;

	label_object_class->set_font_family       = set_font_family;
	label_object_class->set_font_size         = set_font_size;
	label_object_class->set_font_weight       = set_font_weight;
	label_object_class->set_font_italic_flag  = set_font_italic_flag;
	label_object_class->set_text_alignment    = set_text_alignment;
	label_object_class->set_text_line_spacing = set_text_line_spacing;
	label_object_class->set_text_color        = set_text_color;
	label_object_class->get_font_family       = get_font_family;
	label_object_class->get_font_size         = get_font_size;
	label_object_class->get_font_weight       = get_font_weight;
	label_object_class->get_font_italic_flag  = get_font_italic_flag;
	label_object_class->get_text_alignment    = get_text_alignment;
	label_object_class->get_text_line_spacing = get_text_line_spacing;
	label_object_class->get_text_color        = get_text_color;
        label_object_class->draw_object           = draw_object;
        label_object_class->draw_shadow           = draw_shadow;

	object_class->finalize = gl_label_text_finalize;
}

static void
gl_label_text_init (glLabelText *ltext)
{
	ltext->priv = g_new0 (glLabelTextPrivate, 1);

	ltext->priv->tag_table         = gtk_text_tag_table_new ();
	ltext->priv->buffer            = gtk_text_buffer_new (ltext->priv->tag_table);

	ltext->priv->font_family       = g_strdup(DEFAULT_FONT_FAMILY);
	ltext->priv->font_size         = DEFAULT_FONT_SIZE;
	ltext->priv->font_weight       = DEFAULT_FONT_WEIGHT;
	ltext->priv->font_italic_flag  = DEFAULT_FONT_ITALIC_FLAG;
	ltext->priv->align             = DEFAULT_ALIGN;
	ltext->priv->color_node        = gl_color_node_new_default ();
	ltext->priv->color_node->color = DEFAULT_COLOR;
	ltext->priv->line_spacing      = DEFAULT_TEXT_LINE_SPACING;
	ltext->priv->auto_shrink       = DEFAULT_AUTO_SHRINK;

        ltext->priv->size_changed      = TRUE;

	g_signal_connect (G_OBJECT(ltext->priv->buffer), "changed",
			  G_CALLBACK(buffer_changed_cb), ltext);
}

static void
gl_label_text_finalize (GObject *object)
{
	glLabelText *ltext = GL_LABEL_TEXT (object);

	g_return_if_fail (object && GL_IS_LABEL_TEXT (object));

	g_object_unref (ltext->priv->tag_table);
	g_object_unref (ltext->priv->buffer);
	g_free (ltext->priv->font_family);
	gl_color_node_free (&(ltext->priv->color_node));
	g_free (ltext->priv);

	G_OBJECT_CLASS (gl_label_text_parent_class)->finalize (object);
}

/*****************************************************************************/
/* NEW label "text" object.                                               */
/*****************************************************************************/
GObject *
gl_label_text_new (glLabel *label)
{
	glLabelText *ltext;

	ltext = g_object_new (gl_label_text_get_type(), NULL);

	gl_label_object_set_parent (GL_LABEL_OBJECT(ltext), label);

	return G_OBJECT (ltext);
}

/*****************************************************************************/
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
	glLabelText      *ltext     = (glLabelText *)src_object;
	glLabelText      *new_ltext = (glLabelText *)dst_object;
	GList            *lines;
	glColorNode      *text_color_node;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));
	g_return_if_fail (new_ltext && GL_IS_LABEL_TEXT (new_ltext));

	lines = gl_label_text_get_lines (ltext);
	text_color_node = get_text_color (src_object);
	gl_label_text_set_lines (new_ltext, lines);

	new_ltext->priv->font_family      = g_strdup (ltext->priv->font_family);
	new_ltext->priv->font_size        = ltext->priv->font_size;
	new_ltext->priv->font_weight      = ltext->priv->font_weight;
	new_ltext->priv->font_italic_flag = ltext->priv->font_italic_flag;
	set_text_color (dst_object, text_color_node);
	new_ltext->priv->align            = ltext->priv->align;
	new_ltext->priv->line_spacing     = ltext->priv->line_spacing;
	new_ltext->priv->auto_shrink      = ltext->priv->auto_shrink;

        new_ltext->priv->size_changed     = ltext->priv->size_changed;
        new_ltext->priv->w                = ltext->priv->w;
        new_ltext->priv->h                = ltext->priv->h;

	gl_color_node_free (&text_color_node);
	gl_text_node_lines_free (&lines);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_text_set_lines (glLabelText *ltext,
			 GList       *lines)
{
	gchar *text;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	text = gl_text_node_lines_expand (lines, NULL);
	gtk_text_buffer_set_text (ltext->priv->buffer, text, -1);
	g_free (text);

        ltext->priv->size_changed = TRUE;

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
GtkTextBuffer *
gl_label_text_get_buffer (glLabelText *ltext)
{
	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), NULL);

	return ltext->priv->buffer;
}

GList *
gl_label_text_get_lines (glLabelText *ltext)
{
	GtkTextIter  start, end;
	gchar       *text;
	GList       *lines;

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), NULL);

	gtk_text_buffer_get_bounds (ltext->priv->buffer, &start, &end);
	text = gtk_text_buffer_get_text (ltext->priv->buffer,
					 &start, &end, FALSE);
	lines = gl_text_node_lines_new_from_text (text);
	g_free (text);

	return lines;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  text buffer "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
void buffer_changed_cb (GtkTextBuffer *textbuffer,
			glLabelText   *ltext)
{
        ltext->priv->size_changed = TRUE;

	gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get object size method.                                         */
/*---------------------------------------------------------------------------*/
static void
get_size (glLabelObject *object,
	  gdouble       *w,
	  gdouble       *h)
{
	glLabelText          *ltext = (glLabelText *)object;
	PangoFontMap         *fontmap;
	PangoContext         *context;
	cairo_font_options_t *options;
        PangoStyle            style;
        PangoLayout          *layout;
        PangoFontDescription *desc;
	GtkTextIter           start, end;
	gchar                *text;
	gdouble               w_parent, h_parent;
	gint                  iw, ih;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	gl_label_object_get_raw_size (object, &w_parent, &h_parent);

	if ( (w_parent != 0.0) || (h_parent != 0.0) ) {
		*w = w_parent;
		*h = h_parent;
		return;
	}

        if (!ltext->priv->size_changed)
        {
                *w = ltext->priv->w;
                *h = ltext->priv->h;
		return;
        }

	gtk_text_buffer_get_bounds (ltext->priv->buffer, &start, &end);
	text = gtk_text_buffer_get_text (ltext->priv->buffer,
					 &start, &end, FALSE);

	
	fontmap = pango_cairo_font_map_new ();
	context = pango_cairo_font_map_create_context (PANGO_CAIRO_FONT_MAP (fontmap));
	options = cairo_font_options_create ();
	cairo_font_options_set_hint_metrics (options, CAIRO_HINT_METRICS_OFF);
        cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_NONE);
	pango_cairo_context_set_font_options (context, options);
	cairo_font_options_destroy (options);

	layout = pango_layout_new (context);

        style = GL_LABEL_TEXT (object)->priv->font_italic_flag ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;

	desc = pango_font_description_new ();
	pango_font_description_set_family (desc, GL_LABEL_TEXT (object)->priv->font_family);
	pango_font_description_set_weight (desc, GL_LABEL_TEXT (object)->priv->font_weight);
	pango_font_description_set_style  (desc, style);
	pango_font_description_set_size   (desc, GL_LABEL_TEXT (object)->priv->font_size * PANGO_SCALE);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free       (desc);

	pango_layout_set_text (layout, text, -1);
	pango_layout_get_size (layout, &iw, &ih);
	*w = ltext->priv->w = iw / PANGO_SCALE + 2*GL_LABEL_TEXT_MARGIN;
	*h = ltext->priv->h = ih / PANGO_SCALE;
        ltext->priv->size_changed = FALSE;

	g_object_unref (layout);
	g_object_unref (context);
	g_object_unref (fontmap);
	g_free (text);

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set font family method.                                         */
/*---------------------------------------------------------------------------*/
static void
set_font_family (glLabelObject *object,
		 const gchar   *font_family)
{
	glLabelText    *ltext = (glLabelText *)object;
	GList          *family_names;
	gchar          *good_font_family;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));
	g_return_if_fail (font_family);

	family_names = gl_util_get_font_family_list ();
	if (g_list_find_custom (family_names, font_family, (GCompareFunc)g_utf8_collate)) {
		good_font_family = g_strdup (font_family);
	} else {
		if (family_names != NULL) {
			good_font_family = g_strdup (family_names->data); /* 1st entry */
		} else {
			good_font_family = g_strdup (font_family);
		}
	}
	gl_util_font_family_list_free (family_names);

	if (ltext->priv->font_family) {
		if (g_strcasecmp (ltext->priv->font_family, good_font_family) == 0) {
			g_free (good_font_family);
			gl_debug (DEBUG_LABEL, "END (no change)");
			return;
		}
		g_free (ltext->priv->font_family);
	}
	ltext->priv->font_family = g_strdup (good_font_family);
	g_free (good_font_family);

	gl_debug (DEBUG_LABEL, "new font family = %s", ltext->priv->font_family);

        ltext->priv->size_changed = TRUE;

	gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set font size method.                                           */
/*---------------------------------------------------------------------------*/
static void
set_font_size (glLabelObject *object,
	       gdouble        font_size)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->font_size != font_size) {

                ltext->priv->size_changed = TRUE;

		ltext->priv->font_size = font_size;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set font weight method.                                         */
/*---------------------------------------------------------------------------*/
static void
set_font_weight (glLabelObject   *object,
		 PangoWeight      font_weight)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->font_weight != font_weight) {

                ltext->priv->size_changed = TRUE;

		ltext->priv->font_weight = font_weight;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set font italic flag method.                                    */
/*---------------------------------------------------------------------------*/
static void
set_font_italic_flag (glLabelObject *object,
		      gboolean       font_italic_flag)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->font_italic_flag != font_italic_flag) {

                ltext->priv->size_changed = TRUE;

		ltext->priv->font_italic_flag = font_italic_flag;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set text alignment method.                                      */
/*---------------------------------------------------------------------------*/
static void
set_text_alignment (glLabelObject    *object,
		    PangoAlignment    text_alignment)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->align != text_alignment) {

                ltext->priv->size_changed = TRUE;

		ltext->priv->align = text_alignment;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set text line spacing method.                                   */
/*---------------------------------------------------------------------------*/
static void
set_text_line_spacing (glLabelObject *object,
	               gdouble        line_spacing)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->line_spacing != line_spacing) {

                ltext->priv->size_changed = TRUE;

		ltext->priv->line_spacing = line_spacing;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set text color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_text_color (glLabelObject *object,
		glColorNode   *text_color_node)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (!gl_color_node_equal (ltext->priv->color_node, text_color_node)) {

		gl_color_node_free (&(ltext->priv->color_node));
		ltext->priv->color_node = gl_color_node_dup (text_color_node);
		
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get font family method.                                         */
/*---------------------------------------------------------------------------*/
static gchar *
get_font_family (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), NULL);

	return g_strdup (ltext->priv->font_family);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get font size method.                                           */
/*---------------------------------------------------------------------------*/
static gdouble
get_font_size (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), 0.0);

	return ltext->priv->font_size;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get font weight method.                                         */
/*---------------------------------------------------------------------------*/
static PangoWeight
get_font_weight (glLabelObject   *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), PANGO_WEIGHT_NORMAL);

	return ltext->priv->font_weight;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get font italic flag method.                                    */
/*---------------------------------------------------------------------------*/
static gboolean
get_font_italic_flag (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), FALSE);

	return ltext->priv->font_italic_flag;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get text alignment method.                                      */
/*---------------------------------------------------------------------------*/
static PangoAlignment
get_text_alignment (glLabelObject    *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), GTK_JUSTIFY_LEFT);

	return ltext->priv->align;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get text line spacing method.                                   */
/*---------------------------------------------------------------------------*/
static gdouble
get_text_line_spacing (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), 0.0);

	return ltext->priv->line_spacing;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get text color method.                                          */
/*---------------------------------------------------------------------------*/
static glColorNode*
get_text_color (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), 0);

	return gl_color_node_dup (ltext->priv->color_node);
}

/*****************************************************************************/
/* Set auto shrink flag.                                                     */
/*****************************************************************************/
void
gl_label_text_set_auto_shrink (glLabelText      *ltext,
			       gboolean          auto_shrink)
{
	gl_debug (DEBUG_LABEL, "BEGIN");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->auto_shrink != auto_shrink) {

		ltext->priv->auto_shrink = auto_shrink;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Query auto shrink flag.                                                   */
/*****************************************************************************/
gboolean
gl_label_text_get_auto_shrink (glLabelText      *ltext)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), 0);

	return ltext->priv->auto_shrink;
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
        gdouble          x0, y0;
        cairo_matrix_t   matrix;
	gdouble          object_w, object_h;
	gdouble          raw_w, raw_h;
	gchar           *text;
	GList           *lines;
	gchar           *font_family;
	gdouble          font_size;
	PangoWeight      font_weight;
	gboolean         font_italic_flag;
	glColorNode     *color_node;
        gboolean         auto_shrink;
	guint            color;
	gdouble          text_line_spacing;
        PangoAlignment   alignment;
        PangoStyle       style;
        PangoLayout     *layout;
        PangoFontDescription *desc;
        gdouble          scale_x, scale_y;
        cairo_font_options_t *font_options;
        PangoContext         *context;


	gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_position (object, &x0, &y0);
        gl_label_object_get_matrix (object, &matrix);

	gl_label_object_get_size (object, &object_w, &object_h);
	gl_label_object_get_raw_size (object, &raw_w, &raw_h);
	lines = gl_label_text_get_lines (GL_LABEL_TEXT (object));
	font_family = gl_label_object_get_font_family (object);
	font_size = gl_label_object_get_font_size (object);
	font_weight = gl_label_object_get_font_weight (object);
	font_italic_flag = gl_label_object_get_font_italic_flag (object);

	color_node = gl_label_object_get_text_color (object);
	color = gl_color_node_expand (color_node, record);
        if (color_node->field_flag && screen_flag)
        {
                color = GL_COLOR_MERGE_DEFAULT;
        }
	gl_color_node_free (&color_node);
	
	alignment = gl_label_object_get_text_alignment (object);
	text_line_spacing =
		gl_label_object_get_text_line_spacing (object);
        auto_shrink = gl_label_text_get_auto_shrink (GL_LABEL_TEXT (object));

	text = gl_text_node_lines_expand (lines, record);

        style = font_italic_flag ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;


        if (!screen_flag && record && auto_shrink && (raw_w != 0.0))
        {
                font_size = auto_shrink_font_size (cr,
                                                   font_family,
                                                   font_size,
                                                   font_weight,
                                                   style,
                                                   text,
                                                   object_w);
                g_print ("Autosize new size = %g\n", font_size);
        }


        /*
         * Workaround for pango Bug#341481.
         * Render font at device scale and scale font size accordingly.
         */
        scale_x = 1.0;
        scale_y = 1.0;
        cairo_device_to_user_distance (cr, &scale_x, &scale_y);
        scale_x = fabs (scale_x);
        scale_y = fabs (scale_y);
        cairo_save (cr);
        cairo_scale (cr, scale_x, scale_y);

        layout = pango_cairo_create_layout (cr);

        font_options = cairo_font_options_create ();
        cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_OFF);
        cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_NONE);
        context = pango_layout_get_context (layout);
        pango_cairo_context_set_font_options (context, font_options);
        cairo_font_options_destroy (font_options);

        desc = pango_font_description_new ();
        pango_font_description_set_family (desc, font_family);
        pango_font_description_set_weight (desc, font_weight);
        pango_font_description_set_style  (desc, style);
        pango_font_description_set_size   (desc, font_size * PANGO_SCALE / scale_x);
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free       (desc);

        pango_layout_set_text (layout, text, -1);
        pango_layout_set_spacing (layout, font_size * (text_line_spacing-1) * PANGO_SCALE / scale_x);
        if (raw_w == 0.0)
        {
                pango_layout_set_width (layout, -1);
        }
        else
        {
                pango_layout_set_width (layout, object_w * PANGO_SCALE / scale_x);
        }
        pango_layout_set_wrap (layout, PANGO_WRAP_CHAR);
        pango_layout_set_alignment (layout, alignment);


        cairo_set_source_rgba (cr,
                               GL_COLOR_F_RED (color),
                               GL_COLOR_F_GREEN (color),
                               GL_COLOR_F_BLUE (color),
                               GL_COLOR_F_ALPHA (color));

        cairo_move_to (cr, GL_LABEL_TEXT_MARGIN, 0);
        pango_cairo_show_layout (cr, layout);

        cairo_restore (cr);

        g_object_unref (layout);


	gl_text_node_lines_free (&lines);
	g_free (font_family);

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
        gdouble          x0, y0;
        cairo_matrix_t   matrix;
	gdouble          object_w, object_h;
	gdouble          raw_w, raw_h;
	gchar           *text;
	GList           *lines;
	gchar           *font_family;
	gdouble          font_size;
	PangoWeight      font_weight;
	gboolean         font_italic_flag;
        gboolean         auto_shrink;
	guint            color;
	glColorNode     *color_node;
	gdouble          text_line_spacing;
	glColorNode     *shadow_color_node;
	gdouble          shadow_opacity;
	guint            shadow_color;
        PangoAlignment   alignment;
        PangoStyle       style;
        PangoLayout     *layout;
        PangoFontDescription *desc;
        gdouble          scale_x, scale_y;
        cairo_font_options_t *font_options;
        PangoContext         *context;


	gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_position (object, &x0, &y0);
        gl_label_object_get_matrix (object, &matrix);

	gl_label_object_get_size (object, &object_w, &object_h);
	gl_label_object_get_raw_size (object, &raw_w, &raw_h);
	lines = gl_label_text_get_lines (GL_LABEL_TEXT (object));
	font_family = gl_label_object_get_font_family (object);
	font_size = gl_label_object_get_font_size (object);
	font_weight = gl_label_object_get_font_weight (object);
	font_italic_flag = gl_label_object_get_font_italic_flag (object);

	color_node = gl_label_object_get_text_color (object);
	color = gl_color_node_expand (color_node, record);
	gl_color_node_free (&color_node);
	
	alignment = gl_label_object_get_text_alignment (object);
	text_line_spacing =
		gl_label_object_get_text_line_spacing (object);
        auto_shrink = gl_label_text_get_auto_shrink (GL_LABEL_TEXT (object));

	shadow_color_node = gl_label_object_get_shadow_color (object);
	if (shadow_color_node->field_flag)
	{
		shadow_color_node->color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (object);
	shadow_color = gl_color_shadow (shadow_color_node->color, shadow_opacity, color);
	gl_color_node_free (&shadow_color_node);

	text = gl_text_node_lines_expand (lines, record);

        style = font_italic_flag ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;


        if (!screen_flag && record && auto_shrink && (raw_w != 0.0))
        {
                font_size = auto_shrink_font_size (cr,
                                                   font_family,
                                                   font_size,
                                                   font_weight,
                                                   style,
                                                   text,
                                                   object_w);
        }


        /*
         * Workaround for pango Bug#341481.
         * Render font at device scale and scale font size accordingly.
         */
        scale_x = 1.0;
        scale_y = 1.0;
        cairo_device_to_user_distance (cr, &scale_x, &scale_y);
        scale_x = fabs (scale_x);
        scale_y = fabs (scale_y);
        cairo_save (cr);
        cairo_scale (cr, scale_x, scale_y);

        layout = pango_cairo_create_layout (cr);

        font_options = cairo_font_options_create ();
        cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_OFF);
        cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_NONE);
        context = pango_layout_get_context (layout);
        pango_cairo_context_set_font_options (context, font_options);
        cairo_font_options_destroy (font_options);

        desc = pango_font_description_new ();
        pango_font_description_set_family (desc, font_family);
        pango_font_description_set_weight (desc, font_weight);
        pango_font_description_set_style  (desc, style);
        pango_font_description_set_size   (desc, font_size * PANGO_SCALE / scale_x);
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free       (desc);

        pango_layout_set_text (layout, text, -1);
        pango_layout_set_spacing (layout, font_size * (text_line_spacing-1) * PANGO_SCALE / scale_x);
        if (raw_w == 0.0)
        {
                pango_layout_set_width (layout, -1);
        }
        else
        {
                pango_layout_set_width (layout, object_w * PANGO_SCALE / scale_x);
        }
        pango_layout_set_wrap (layout, PANGO_WRAP_CHAR);
        pango_layout_set_alignment (layout, alignment);

        cairo_set_source_rgba (cr,
                               GL_COLOR_F_RED (shadow_color),
                               GL_COLOR_F_GREEN (shadow_color),
                               GL_COLOR_F_BLUE (shadow_color),
                               GL_COLOR_F_ALPHA (shadow_color));

        cairo_move_to (cr, GL_LABEL_TEXT_MARGIN, 0);
        pango_cairo_show_layout (cr, layout);


        cairo_restore (cr);

        g_object_unref (layout);

	gl_text_node_lines_free (&lines);
	g_free (font_family);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Automatically shrink text size to fit within horizontal width.            */
/*****************************************************************************/
static gdouble
auto_shrink_font_size (cairo_t     *cr,
                       gchar       *family,
                       gdouble      size,
                       PangoWeight  weight,
                       PangoStyle   style,
                       gchar       *text,
                       gdouble      width)
{
        PangoLayout          *layout;
        PangoFontDescription *desc;
        gint                  iw, ih;
        gdouble               layout_width;
        gdouble               new_size;

        layout = pango_cairo_create_layout (cr);

        desc = pango_font_description_new ();
        pango_font_description_set_family (desc, family);
        pango_font_description_set_weight (desc, weight);
        pango_font_description_set_style  (desc, style);
        pango_font_description_set_size   (desc, size * PANGO_SCALE);
        
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free       (desc);

        pango_layout_set_text (layout, text, -1);
        pango_layout_set_width (layout, -1);
        pango_layout_get_size (layout, &iw, &ih);
        layout_width = (gdouble)iw / (gdouble)PANGO_SCALE;

        g_object_unref (layout);

        g_print ("Object w = %g, layout w = %g\n", width, layout_width);

        if ( layout_width > width )
        {
                /* Scale down. */
                new_size = size * (width-2*GL_LABEL_TEXT_MARGIN)/layout_width;

                /* Round down to nearest 1/2 point */
                new_size = (int)(new_size*2.0) / 2.0;

                /* don't get ridiculously small. */
                if (new_size < 1.0)
                {
                        new_size = 1.0;
                }
        }
        else
        {
                new_size = size;
        }

        return new_size;
}

