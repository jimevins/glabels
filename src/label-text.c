/*
 *  label-text.c
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

#include "label-text.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <pango/pango.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "font-util.h"
#include "font-history.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define FONT_SCALE (72.0/96.0)

#define HANDLE_OUTLINE_RGBA_ARGS   0.5,   0.5,   0.5,   0.75
#define HANDLE_OUTLINE_WIDTH_PIXELS   2.0

#define SELECTION_SLOP_PIXELS 4.0


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
	glValignment     valign;
	glColorNode     *color_node;
	gdouble          line_spacing;
	gboolean         auto_shrink;

        gboolean         size_changed;
        gdouble          w;
        gdouble          h;

        gboolean         checkpoint_flag;
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

static void buffer_begin_user_action_cb (GtkTextBuffer    *textbuffer,
                                         glLabelText      *ltext);

static void buffer_changed_cb           (GtkTextBuffer    *textbuffer,
					 glLabelText      *ltext);

static void get_size                    (glLabelObject    *object,
					 gdouble          *w,
					 gdouble          *h);

static void set_font_family             (glLabelObject    *object,
					 const gchar      *font_family,
                                         gboolean          checkpoint);

static void set_font_size               (glLabelObject    *object,
					 gdouble           font_size,
                                         gboolean          checkpoint);

static void set_font_weight             (glLabelObject    *object,
					 PangoWeight       font_weight,
                                         gboolean          checkpoint);

static void set_font_italic_flag        (glLabelObject    *object,
					 gboolean          font_italic_flag,
                                         gboolean          checkpoint);

static void set_text_alignment          (glLabelObject    *object,
					 PangoAlignment    text_alignment,
                                         gboolean          checkpoint);

static void set_text_valignment         (glLabelObject    *object,
					 glValignment      text_valignment,
                                         gboolean          checkpoint);

static void set_text_line_spacing       (glLabelObject    *object,
					 gdouble           text_line_spacing,
                                         gboolean          checkpoint);

static void set_text_color              (glLabelObject    *object,
					 glColorNode      *text_color_node,
                                         gboolean          checkpoint);

static gchar          *get_font_family             (glLabelObject    *object);

static gdouble         get_font_size               (glLabelObject    *object);

static PangoWeight     get_font_weight             (glLabelObject    *object);

static gboolean        get_font_italic_flag        (glLabelObject    *object);

static PangoAlignment  get_text_alignment          (glLabelObject    *object);

static glValignment    get_text_valignment         (glLabelObject    *object);

static gdouble         get_text_line_spacing       (glLabelObject    *object);

static glColorNode*    get_text_color              (glLabelObject    *object);

static void            layout_text                 (glLabelText      *this,
                                                    cairo_t          *cr,
                                                    gboolean          screen_flag,
                                                    glMergeRecord    *record,
                                                    gboolean          path_only_flag);

static void            draw_object                 (glLabelObject    *object,
                                                    cairo_t          *cr,
                                                    gboolean          screen_flag,
                                                    glMergeRecord    *record);

static void            draw_shadow                 (glLabelObject    *object,
                                                    cairo_t          *cr,
                                                    gboolean          screen_flag,
                                                    glMergeRecord    *record);

static void            draw_text_real              (glLabelObject    *object,
                                                    cairo_t          *cr,
                                                    gboolean          screen_flag,
                                                    glMergeRecord    *record,
                                                    guint             color);

static gdouble         auto_shrink_font_size       (cairo_t          *cr,
                                                    gchar            *family,
                                                    gdouble           size,
                                                    PangoWeight       weight,
                                                    PangoStyle        style,
                                                    gdouble           line_spacing,
                                                    gchar            *text,
                                                    gdouble           width,
                                                    gdouble           height);

static gboolean        object_at                   (glLabelObject    *object,
                                                    cairo_t          *cr,
                                                    gdouble           x_pixels,
                                                    gdouble           y_pixels);

static void            draw_handles                (glLabelObject    *object,
                                                    cairo_t          *cr);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelText, gl_label_text, GL_TYPE_LABEL_OBJECT)


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
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
	label_object_class->set_text_valignment   = set_text_valignment;
	label_object_class->set_text_line_spacing = set_text_line_spacing;
	label_object_class->set_text_color        = set_text_color;
	label_object_class->get_font_family       = get_font_family;
	label_object_class->get_font_size         = get_font_size;
	label_object_class->get_font_weight       = get_font_weight;
	label_object_class->get_font_italic_flag  = get_font_italic_flag;
	label_object_class->get_text_alignment    = get_text_alignment;
	label_object_class->get_text_valignment   = get_text_valignment;
	label_object_class->get_text_line_spacing = get_text_line_spacing;
	label_object_class->get_text_color        = get_text_color;
        label_object_class->draw_object           = draw_object;
        label_object_class->draw_shadow           = draw_shadow;
        label_object_class->object_at             = object_at;
        label_object_class->draw_handles          = draw_handles;

	object_class->finalize = gl_label_text_finalize;
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_label_text_init (glLabelText *ltext)
{
	ltext->priv = g_new0 (glLabelTextPrivate, 1);

	ltext->priv->tag_table         = gtk_text_tag_table_new ();
	ltext->priv->buffer            = gtk_text_buffer_new (ltext->priv->tag_table);

        ltext->priv->size_changed      = TRUE;

        ltext->priv->checkpoint_flag   = TRUE;

	g_signal_connect (G_OBJECT(ltext->priv->buffer), "begin-user-action",
			  G_CALLBACK(buffer_begin_user_action_cb), ltext);
	g_signal_connect (G_OBJECT(ltext->priv->buffer), "changed",
			  G_CALLBACK(buffer_changed_cb), ltext);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
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
/** New Object Generator.                                                    */
/*****************************************************************************/
GObject *
gl_label_text_new (glLabel *label,
                   gboolean checkpoint)
{
	glLabelText   *ltext;
        glColorNode   *color_node;

	ltext = g_object_new (gl_label_text_get_type(), NULL);

        if (label != NULL)
        {
                if ( checkpoint )
                {
                        gl_label_checkpoint (label, _("Create text object"));
                }

                color_node = gl_color_node_new_default ();

                color_node->color = gl_label_get_default_text_color (label);

                ltext->priv->font_family      = gl_label_get_default_font_family (label);
                ltext->priv->font_size        = gl_label_get_default_font_size (label);
                ltext->priv->font_weight      = gl_label_get_default_font_weight (label);
                ltext->priv->font_italic_flag = gl_label_get_default_font_italic_flag (label);
                ltext->priv->align            = gl_label_get_default_text_alignment (label);
                ltext->priv->valign           = gl_label_get_default_text_valignment (label);
		ltext->priv->color_node       = color_node;	  
                ltext->priv->line_spacing     = gl_label_get_default_text_line_spacing (label);

                gl_label_add_object (label, GL_LABEL_OBJECT (ltext));
                gl_label_object_set_parent (GL_LABEL_OBJECT (ltext), label);
        }

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
	gl_label_text_set_lines (new_ltext, lines, FALSE);

	new_ltext->priv->font_family      = g_strdup (ltext->priv->font_family);
	new_ltext->priv->font_size        = ltext->priv->font_size;
	new_ltext->priv->font_weight      = ltext->priv->font_weight;
	new_ltext->priv->font_italic_flag = ltext->priv->font_italic_flag;
	set_text_color (dst_object, text_color_node, FALSE);
	new_ltext->priv->align            = ltext->priv->align;
	new_ltext->priv->valign           = ltext->priv->valign;
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
			 GList       *lines,
                         gboolean     checkpoint)
{
	gchar *text;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

        ltext->priv->checkpoint_flag = checkpoint;

	text = gl_text_node_lines_expand (lines, NULL);
	gtk_text_buffer_set_text (ltext->priv->buffer, text, -1);
	g_free (text);

        ltext->priv->size_changed = TRUE;

        ltext->priv->checkpoint_flag = TRUE;

	gl_debug (DEBUG_LABEL, "END");
}


void
gl_label_text_set_text (glLabelText *ltext,
                        const gchar *text,
                        gboolean     checkpoint)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

        ltext->priv->checkpoint_flag = checkpoint;

	gtk_text_buffer_set_text (ltext->priv->buffer, text, -1);

        ltext->priv->size_changed = TRUE;

        ltext->priv->checkpoint_flag = TRUE;

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


gchar *
gl_label_text_get_text (glLabelText      *ltext)
{
	GtkTextIter  start, end;
	gchar       *text;

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), NULL);

	gtk_text_buffer_get_bounds (ltext->priv->buffer, &start, &end);
	text = gtk_text_buffer_get_text (ltext->priv->buffer,
					 &start, &end, FALSE);

	return text;
}


/*****************************************************************************/
/* Text buffer "changed" callback.                                           */
/*****************************************************************************/
static void
buffer_begin_user_action_cb (GtkTextBuffer *textbuffer,
                             glLabelText   *ltext)
{
        glLabel *label;

        if ( ltext->priv->checkpoint_flag )
        {
                label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                gl_label_checkpoint (label, _("Typing"));
        }
}


/*****************************************************************************/
/* Text buffer "changed" callback.                                           */
/*****************************************************************************/
static void
buffer_changed_cb (GtkTextBuffer *textbuffer,
                   glLabelText   *ltext)
{
        ltext->priv->size_changed = TRUE;

	gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
}


/*****************************************************************************/
/* Get object size method.                                                   */
/*****************************************************************************/
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
        gdouble               font_size;
        gdouble               line_spacing;
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

        font_size = GL_LABEL_TEXT (object)->priv->font_size * FONT_SCALE;
        line_spacing = GL_LABEL_TEXT (object)->priv->line_spacing;

	gtk_text_buffer_get_bounds (ltext->priv->buffer, &start, &end);
	text = gtk_text_buffer_get_text (ltext->priv->buffer,
					 &start, &end, FALSE);

	
	fontmap = pango_cairo_font_map_new ();
	context = pango_font_map_create_context (PANGO_FONT_MAP (fontmap));
	options = cairo_font_options_create ();
        cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_NONE);
        cairo_font_options_set_hint_metrics (options, CAIRO_HINT_METRICS_OFF);
	pango_cairo_context_set_font_options (context, options);
	cairo_font_options_destroy (options);

	layout = pango_layout_new (context);

        style = GL_LABEL_TEXT (object)->priv->font_italic_flag ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;

	desc = pango_font_description_new ();
	pango_font_description_set_family (desc, GL_LABEL_TEXT (object)->priv->font_family);
	pango_font_description_set_weight (desc, GL_LABEL_TEXT (object)->priv->font_weight);
	pango_font_description_set_style  (desc, style);
	pango_font_description_set_size   (desc, font_size * PANGO_SCALE);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free       (desc);

        pango_layout_set_spacing (layout, font_size * (line_spacing-1) * PANGO_SCALE);
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


/*****************************************************************************/
/* Set font family method.                                                   */
/*****************************************************************************/
static void
set_font_family (glLabelObject *object,
		 const gchar   *font_family,
                 gboolean       checkpoint)
{
	glLabelText    *ltext = (glLabelText *)object;
	gchar          *good_font_family;
        glLabel        *label;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));
	g_return_if_fail (font_family);

        good_font_family = gl_font_util_validate_family (font_family);

	if (ltext->priv->font_family) {
		if (strcmp (ltext->priv->font_family, good_font_family) == 0) {
			g_free (good_font_family);
			gl_debug (DEBUG_LABEL, "END (no change)");
			return;
		}
		g_free (ltext->priv->font_family);
	}

        if ( checkpoint )
        {
                label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                gl_label_checkpoint (label, _("Font family"));
        }

	ltext->priv->font_family = g_strdup (good_font_family);
	g_free (good_font_family);

	gl_debug (DEBUG_LABEL, "new font family = %s", ltext->priv->font_family);

        ltext->priv->size_changed = TRUE;

        gl_font_history_model_add_family (gl_font_history, ltext->priv->font_family);

	gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set font size method.                                                     */
/*****************************************************************************/
static void
set_font_size (glLabelObject *object,
	       gdouble        font_size,
               gboolean       checkpoint)
{
	glLabelText    *ltext = (glLabelText *)object;
        glLabel        *label;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->font_size != font_size)
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                        gl_label_checkpoint (label, _("Font size"));
                }

                ltext->priv->size_changed = TRUE;

		ltext->priv->font_size = font_size;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set font weight method.                                                   */
/*****************************************************************************/
static void
set_font_weight (glLabelObject   *object,
		 PangoWeight      font_weight,
                 gboolean         checkpoint)
{
	glLabelText    *ltext = (glLabelText *)object;
        glLabel        *label;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->font_weight != font_weight)
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                        gl_label_checkpoint (label, _("Font weight"));
                }

                ltext->priv->size_changed = TRUE;

		ltext->priv->font_weight = font_weight;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set font italic flag method.                                              */
/*****************************************************************************/
static void
set_font_italic_flag (glLabelObject *object,
		      gboolean       font_italic_flag,
                      gboolean       checkpoint)
{
	glLabelText    *ltext = (glLabelText *)object;
        glLabel        *label;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->font_italic_flag != font_italic_flag)
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                        gl_label_checkpoint (label, _("Italic"));
                }

                ltext->priv->size_changed = TRUE;

		ltext->priv->font_italic_flag = font_italic_flag;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set text alignment method.                                                */
/*****************************************************************************/
static void
set_text_alignment (glLabelObject    *object,
		    PangoAlignment    text_alignment,
                    gboolean          checkpoint)
{
	glLabelText    *ltext = (glLabelText *)object;
        glLabel        *label;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->align != text_alignment)
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                        gl_label_checkpoint (label, _("Align text"));
                }

                ltext->priv->size_changed = TRUE;

		ltext->priv->align = text_alignment;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set vertical text alignment method.                                       */
/*****************************************************************************/
static void
set_text_valignment (glLabelObject    *object,
		     glValignment      text_valignment,
                     gboolean          checkpoint)
{
	glLabelText    *ltext = (glLabelText *)object;
        glLabel        *label;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->valign != text_valignment)
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                        gl_label_checkpoint (label, _("Vertically align text"));
                }

                ltext->priv->size_changed = TRUE;

		ltext->priv->valign = text_valignment;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set text line spacing method.                                             */
/*****************************************************************************/
static void
set_text_line_spacing (glLabelObject *object,
	               gdouble        line_spacing,
                       gboolean       checkpoint)
{
	glLabelText    *ltext = (glLabelText *)object;
        glLabel        *label;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->line_spacing != line_spacing)
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                        gl_label_checkpoint (label, _("Line spacing"));
                }

                ltext->priv->size_changed = TRUE;

		ltext->priv->line_spacing = line_spacing;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set text color method.                                                    */
/*****************************************************************************/
static void
set_text_color (glLabelObject *object,
		glColorNode   *text_color_node,
                gboolean       checkpoint)
{
	glLabelText    *ltext = (glLabelText *)object;
        glLabel        *label;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (!gl_color_node_equal (ltext->priv->color_node, text_color_node))
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                        gl_label_checkpoint (label, _("Text color"));
                }

		gl_color_node_free (&(ltext->priv->color_node));
		ltext->priv->color_node = gl_color_node_dup (text_color_node);
		
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));
	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get font family method.                                                   */
/*****************************************************************************/
static gchar *
get_font_family (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), NULL);

	return g_strdup (ltext->priv->font_family);
}


/*****************************************************************************/
/* Get font size method.                                                     */
/*****************************************************************************/
static gdouble
get_font_size (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), 0.0);

	return ltext->priv->font_size;
}


/*****************************************************************************/
/* Get font weight method.                                                   */
/*****************************************************************************/
static PangoWeight
get_font_weight (glLabelObject   *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), PANGO_WEIGHT_NORMAL);

	return ltext->priv->font_weight;
}


/*****************************************************************************/
/* Get font italic flag method.                                              */
/*****************************************************************************/
static gboolean
get_font_italic_flag (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), FALSE);

	return ltext->priv->font_italic_flag;
}


/*****************************************************************************/
/* Get text alignment method.                                                */
/*****************************************************************************/
static PangoAlignment
get_text_alignment (glLabelObject    *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), GTK_JUSTIFY_LEFT);

	return ltext->priv->align;
}


/*****************************************************************************/
/* Get vertical text alignment method.                                       */
/*****************************************************************************/
static glValignment
get_text_valignment (glLabelObject    *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), GTK_JUSTIFY_LEFT);

	return ltext->priv->valign;
}


/*****************************************************************************/
/* Get text line spacing method.                                             */
/*****************************************************************************/
static gdouble
get_text_line_spacing (glLabelObject *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), 0.0);

	return ltext->priv->line_spacing;
}


/*****************************************************************************/
/* Get text color method.                                                    */
/*****************************************************************************/
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
			       gboolean          auto_shrink,
                               gboolean          checkpoint)
{
        glLabel *label;

	gl_debug (DEBUG_LABEL, "BEGIN");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->priv->auto_shrink != auto_shrink)
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (ltext));
                        gl_label_checkpoint (label, _("Auto shrink"));
                }

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
/* Automatically shrink text size to fit within bounding box.                */
/*****************************************************************************/
static gdouble
auto_shrink_font_size (cairo_t     *cr,
                       gchar       *family,
                       gdouble      size,
                       PangoWeight  weight,
                       PangoStyle   style,
                       gdouble      line_spacing,
                       gchar       *text,
                       gdouble      width,
                       gdouble      height)
{
        PangoLayout          *layout;
        PangoFontDescription *desc;
        gint                  iw, ih;
        gdouble               layout_width, layout_height;
        gdouble               new_wsize, new_hsize;

        layout = pango_cairo_create_layout (cr);

        desc = pango_font_description_new ();
        pango_font_description_set_family (desc, family);
        pango_font_description_set_weight (desc, weight);
        pango_font_description_set_style  (desc, style);
        pango_font_description_set_size   (desc, size * PANGO_SCALE);

        pango_layout_set_font_description (layout, desc);
        pango_font_description_free       (desc);

        pango_layout_set_spacing (layout, size * (line_spacing-1) * PANGO_SCALE);
        pango_layout_set_width (layout, -1);
        pango_layout_set_text (layout, text, -1);
        pango_layout_get_size (layout, &iw, &ih);
        layout_width = (gdouble)iw / (gdouble)PANGO_SCALE;
        layout_height = (gdouble)ih / (gdouble)PANGO_SCALE;

        g_object_unref (layout);

        fprintf (stderr, "Object w = %g, layout w = %g\n", width, layout_width);
        fprintf (stderr, "Object h = %g, layout h = %g\n", height, layout_height);

        new_wsize = new_hsize = size;
        if ( layout_width > width )
        {
                /* Scale down. */
                new_wsize = size * (width-2*GL_LABEL_TEXT_MARGIN) / layout_width;

                /* Round down to nearest 1/2 point */
                new_wsize = (int)(new_wsize*2.0) / 2.0;

                /* don't get ridiculously small. */
                if (new_wsize < 1.0)
                {
                        new_wsize = 1.0;
                }
        }

        if ( layout_height > height )
        {
                /* Scale down. */
                new_hsize = size * height / layout_height;

                /* Round down to nearest 1/2 point */
                new_hsize = (int)(new_hsize*2.0) / 2.0;

                /* don't get ridiculously small. */
                if (new_hsize < 1.0)
                {
                        new_hsize = 1.0;
                }
        }

        return (new_wsize < new_hsize ? new_wsize : new_hsize);
}


/*****************************************************************************/
/* Update pango layout.                                                      */
/*****************************************************************************/
static void
layout_text (glLabelText      *this,
             cairo_t          *cr,
             gboolean          screen_flag,
             glMergeRecord    *record,
             gboolean          path_only_flag)
{
        gint                  iw, ih, y;
        gdouble               object_w, object_h;
        gdouble               raw_w, raw_h;
        gchar                *text;
        GList                *lines;
        gdouble               font_size;
        gboolean              auto_shrink;
        PangoLayout          *layout;
        PangoStyle            style;
        PangoFontDescription *desc;
        gdouble               scale_x, scale_y;
        cairo_font_options_t *font_options;
        PangoContext         *context;


        gl_debug (DEBUG_LABEL, "START");

        /*
         * Workaround for pango Bug#700592, which is a regression of Bug#341481.
         * Render font at device scale and scale font size accordingly.
         */
        scale_x = 1.0;
        scale_y = 1.0;
        cairo_device_to_user_distance (cr, &scale_x, &scale_y);
        scale_x = fabs (scale_x);
        scale_y = fabs (scale_y);
        cairo_save (cr);
        cairo_scale (cr, scale_x, scale_y);

        gl_label_object_get_size (GL_LABEL_OBJECT (this), &object_w, &object_h);
        gl_label_object_get_raw_size (GL_LABEL_OBJECT (this), &raw_w, &raw_h);

        lines = gl_label_text_get_lines (this);
        text = gl_text_node_lines_expand (lines, record);

        style = this->priv->font_italic_flag ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;

        font_size   = this->priv->font_size * FONT_SCALE;
        auto_shrink = gl_label_text_get_auto_shrink (this);
        if (!screen_flag && record && auto_shrink && (raw_w != 0.0))
        {
                font_size = auto_shrink_font_size (cr,
                                                   this->priv->font_family,
                                                   font_size,
                                                   this->priv->font_weight,
                                                   style,
                                                   this->priv->line_spacing,
                                                   text,
                                                   object_w,
                                                   object_h);
        }


        layout = pango_cairo_create_layout (cr);

        font_options = cairo_font_options_create ();
        cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_NONE);
        cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_OFF);
        context = pango_layout_get_context (layout);
        pango_cairo_context_set_font_options (context, font_options);
        cairo_font_options_destroy (font_options);

        desc = pango_font_description_new ();
        pango_font_description_set_family (desc, this->priv->font_family);
        pango_font_description_set_weight (desc, this->priv->font_weight);
        pango_font_description_set_size   (desc, font_size * PANGO_SCALE / scale_x);
        pango_font_description_set_style  (desc, style);
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free       (desc);

        pango_layout_set_text (layout, text, -1);
        pango_layout_set_spacing (layout, font_size * (this->priv->line_spacing-1) * PANGO_SCALE / scale_x);
        if ( (raw_w == 0.0) || auto_shrink )
        {
                pango_layout_set_width (layout, -1);
        }
        else
        {
                pango_layout_set_width (layout, (object_w - 2*GL_LABEL_TEXT_MARGIN) * PANGO_SCALE / scale_x);
        }
        pango_layout_set_wrap (layout, PANGO_WRAP_WORD);
        pango_layout_set_alignment (layout, this->priv->align);
        pango_layout_get_pixel_size (layout, &iw, &ih);

        switch (this->priv->valign)
        {
        case GL_VALIGN_VCENTER:
                y = (object_h/scale_x - ih) / 2;
                break;
        case GL_VALIGN_BOTTOM:
                y = object_h/scale_x - ih;
                break;
        default:
                y = 0;
                break;
        }

        cairo_move_to (cr, GL_LABEL_TEXT_MARGIN/scale_x, y);
        if ( path_only_flag )
        {
                pango_cairo_layout_path (cr, layout);
        }
        else
        {
                pango_cairo_show_layout (cr, layout);
        }

        g_object_unref (layout);
        gl_text_node_lines_free (&lines);

        cairo_restore (cr);

        gl_debug (DEBUG_LABEL, "END");
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
        glColorNode     *color_node;
        guint            color;

        gl_debug (DEBUG_LABEL, "START");

        color_node = gl_label_object_get_text_color (object);
        color = gl_color_node_expand (color_node, record);
        if (color_node->field_flag && screen_flag)
        {
                color = GL_COLOR_MERGE_DEFAULT;
        }
        gl_color_node_free (&color_node);

        draw_text_real (object, cr, screen_flag, record, color);

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
        glColorNode     *color_node;
        guint            color;
        glColorNode     *shadow_color_node;
        gdouble          shadow_opacity;
        guint            shadow_color;

        gl_debug (DEBUG_LABEL, "START");

        color_node = gl_label_object_get_text_color (object);
        color = gl_color_node_expand (color_node, record);
        if (color_node->field_flag && screen_flag)
        {
                color = GL_COLOR_MERGE_DEFAULT;
        }
        gl_color_node_free (&color_node);

        shadow_color_node = gl_label_object_get_shadow_color (object);
	shadow_color = gl_color_node_expand (shadow_color_node, record);
        if (shadow_color_node->field_flag)
        {
                shadow_color_node->color = GL_COLOR_SHADOW_MERGE_DEFAULT;
        }
        shadow_opacity = gl_label_object_get_shadow_opacity (object);
	shadow_color = gl_color_set_opacity (shadow_color, shadow_opacity);
        gl_color_node_free (&shadow_color_node);

        draw_text_real (object, cr, screen_flag, record, shadow_color);

        gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Draw text.                                                                */
/*****************************************************************************/
static void
draw_text_real (glLabelObject *object,
                cairo_t       *cr,
                gboolean       screen_flag,
                glMergeRecord *record,
                guint          color)
{
        gl_debug (DEBUG_LABEL, "START");

        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (color));
        layout_text (GL_LABEL_TEXT (object), cr, screen_flag, record, FALSE);

        gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Is object at coordinates?                                                 */
/*****************************************************************************/
static gboolean
object_at (glLabelObject *object,
           cairo_t       *cr,
           gdouble        x,
           gdouble        y)
{
        gdouble           w, h;
        gdouble           scale_x, scale_y;

        gl_label_object_get_size (object, &w, &h);

        if ( (x >= 0) && (x <= w) && (y >= 0) && (y <= h) )
        {
                cairo_new_path (cr);
                layout_text (GL_LABEL_TEXT (object), cr, TRUE, NULL, TRUE);
                if (cairo_in_fill (cr, x, y))
                {
                        return TRUE;
                }


                scale_x = 1.0;
                scale_y = 1.0;
                cairo_device_to_user_distance (cr, &scale_x, &scale_y);

                cairo_set_line_width (cr, 2*SELECTION_SLOP_PIXELS*scale_x);

                if (cairo_in_stroke (cr, x, y))
                {
                        return TRUE;
                }


                if (gl_label_object_is_selected (object))
                {
                        cairo_new_path (cr);
                        cairo_rectangle (cr, 0, 0, w, h);

                        scale_x = 1.0;
                        scale_y = 1.0;
                        cairo_device_to_user_distance (cr, &scale_x, &scale_y);

                        cairo_set_line_width (cr, 2*SELECTION_SLOP_PIXELS*scale_x);

                        if (cairo_in_stroke (cr, x, y))
                        {
                                return TRUE;
                        }
                }

        }

        return FALSE;
}


/*****************************************************************************/
/* Draw text style handles.                                                  */
/*****************************************************************************/
static void
draw_handles (glLabelObject     *object,
              cairo_t           *cr)
{
        gdouble w, h;
        gdouble scale_x, scale_y;
        gdouble dashes[2] = { 2, 2 };

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

        cairo_save (cr);

        cairo_rectangle (cr, 0, 0, w, h);

        scale_x = 1.0;
        scale_y = 1.0;
        cairo_device_to_user_distance (cr, &scale_x, &scale_y);
        cairo_scale (cr, scale_x, scale_y);

        cairo_set_dash (cr, dashes, 2, 0);
        cairo_set_line_width (cr, HANDLE_OUTLINE_WIDTH_PIXELS);
        cairo_set_source_rgba (cr, HANDLE_OUTLINE_RGBA_ARGS);
        cairo_stroke (cr);

        cairo_restore (cr);

        gl_label_object_draw_handles_box (object, cr);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
