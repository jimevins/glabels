/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_text.c:  GLabels label text object
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
#include <libgnomeprint/gnome-glyphlist.h>

#include "label-text.h"

#include "pixmaps/checkerboard.xpm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define DEFAULT_FONT_FAMILY       "Sans"
#define DEFAULT_FONT_SIZE         14.0
#define DEFAULT_FONT_WEIGHT       GNOME_FONT_BOOK
#define DEFAULT_FONT_ITALIC_FLAG  FALSE
#define DEFAULT_JUST              GTK_JUSTIFY_LEFT
#define DEFAULT_COLOR             GNOME_CANVAS_COLOR (0,0,0)
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
	GnomeFontWeight  font_weight;
	gboolean         font_italic_flag;
	GtkJustification just;
	glColorNode     *color_node;
	gdouble          line_spacing;
	gboolean         auto_shrink;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glLabelObjectClass *parent_class = NULL;

static guint instance = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_text_class_init    (glLabelTextClass *klass);
static void gl_label_text_instance_init (glLabelText      *ltext);
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
					 GnomeFontWeight   font_weight);

static void set_font_italic_flag        (glLabelObject    *object,
					 gboolean          font_italic_flag);

static void set_text_alignment          (glLabelObject    *object,
					 GtkJustification  text_alignment);

static void set_text_line_spacing       (glLabelObject    *object,
					 gdouble           text_line_spacing);

static void set_text_color              (glLabelObject    *object,
					 glColorNode      *text_color_node);

static gchar          *get_font_family             (glLabelObject    *object);

static gdouble         get_font_size               (glLabelObject    *object);

static GnomeFontWeight get_font_weight             (glLabelObject    *object);

static gboolean        get_font_italic_flag        (glLabelObject    *object);

static GtkJustification get_text_alignment         (glLabelObject    *object);

static gdouble         get_text_line_spacing       (glLabelObject    *object);

static glColorNode*    get_text_color              (glLabelObject    *object);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_text_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glLabelTextClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_text_class_init,
			NULL,
			NULL,
			sizeof (glLabelText),
			0,
			(GInstanceInitFunc) gl_label_text_instance_init,
			NULL
		};

		type = g_type_register_static (GL_TYPE_LABEL_OBJECT,
					       "glLabelText", &info, 0);
	}

	return type;
}

static void
gl_label_text_class_init (glLabelTextClass *klass)
{
	GObjectClass       *object_class       = (GObjectClass *) klass;
	glLabelObjectClass *label_object_class = (glLabelObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	label_object_class->copy                 = copy;

	label_object_class->get_size             = get_size;

	label_object_class->set_font_family      = set_font_family;
	label_object_class->set_font_size        = set_font_size;
	label_object_class->set_font_weight      = set_font_weight;
	label_object_class->set_font_italic_flag = set_font_italic_flag;
	label_object_class->set_text_alignment   = set_text_alignment;
	label_object_class->set_text_line_spacing = set_text_line_spacing;
	label_object_class->set_text_color       = set_text_color;
	label_object_class->get_font_family      = get_font_family;
	label_object_class->get_font_size        = get_font_size;
	label_object_class->get_font_weight      = get_font_weight;
	label_object_class->get_font_italic_flag = get_font_italic_flag;
	label_object_class->get_text_alignment   = get_text_alignment;
	label_object_class->get_text_line_spacing = get_text_line_spacing;
	label_object_class->get_text_color       = get_text_color;

	object_class->finalize = gl_label_text_finalize;
}

static void
gl_label_text_instance_init (glLabelText *ltext)
{
	ltext->private = g_new0 (glLabelTextPrivate, 1);

	ltext->private->tag_table         = gtk_text_tag_table_new ();
	ltext->private->buffer            = gtk_text_buffer_new (ltext->private->tag_table);

	ltext->private->font_family       = g_strdup(DEFAULT_FONT_FAMILY);
	ltext->private->font_size         = DEFAULT_FONT_SIZE;
	ltext->private->font_weight       = DEFAULT_FONT_WEIGHT;
	ltext->private->font_italic_flag  = DEFAULT_FONT_ITALIC_FLAG;
	ltext->private->just              = DEFAULT_JUST;
	ltext->private->color_node        = gl_color_node_new_default ();
	ltext->private->color_node->color = DEFAULT_COLOR;
	ltext->private->line_spacing      = DEFAULT_TEXT_LINE_SPACING;
	ltext->private->auto_shrink       = DEFAULT_AUTO_SHRINK;

	g_signal_connect (G_OBJECT(ltext->private->buffer), "changed",
			  G_CALLBACK(buffer_changed_cb), ltext);
}

static void
gl_label_text_finalize (GObject *object)
{
	glLabelText *ltext;

	g_return_if_fail (object && GL_IS_LABEL_TEXT (object));

	ltext = GL_LABEL_TEXT (object);

	gl_color_node_free (&(ltext->private->color_node));
	g_object_unref (ltext->private->tag_table);
	g_object_unref (ltext->private->buffer);
	g_free (ltext->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);
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

	new_ltext->private->font_family      = g_strdup (ltext->private->font_family);
	new_ltext->private->font_size        = ltext->private->font_size;
	new_ltext->private->font_weight      = ltext->private->font_weight;
	new_ltext->private->font_italic_flag = ltext->private->font_italic_flag;
	set_text_color (dst_object, text_color_node);
	new_ltext->private->just             = ltext->private->just;
	new_ltext->private->line_spacing     = ltext->private->line_spacing;
	new_ltext->private->auto_shrink      = ltext->private->auto_shrink;

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
	gtk_text_buffer_set_text (ltext->private->buffer, text, -1);
	g_free (text);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
GtkTextBuffer *
gl_label_text_get_buffer (glLabelText *ltext)
{
	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), NULL);

	return ltext->private->buffer;
}

GList *
gl_label_text_get_lines (glLabelText *ltext)
{
	GtkTextIter  start, end;
	gchar       *text;
	GList       *lines;

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), NULL);

	gtk_text_buffer_get_bounds (ltext->private->buffer, &start, &end);
	text = gtk_text_buffer_get_text (ltext->private->buffer,
					 &start, &end, FALSE);
	lines = gl_text_node_lines_new_from_text (text);
	g_free (text);

	return lines;
}

void
gl_label_text_get_box (glLabelText *ltext,
		       gdouble     *w,
		       gdouble     *h)
{
	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	/* peek at the parent size. */
	(* parent_class->get_size) (GL_LABEL_OBJECT(ltext), w, h);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  text buffer "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
void buffer_changed_cb (GtkTextBuffer *textbuffer,
			glLabelText   *ltext)
{
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
	glLabelText    *ltext = (glLabelText *)object;
	GnomeFont      *font;
	GtkTextIter     start, end;
	gchar          *text;
	gchar         **line;
	gint            i;
	GnomeGlyphList *glyphlist;
	ArtDRect        bbox;
	gdouble         affine[6];
	gdouble         w_parent, h_parent;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	(* parent_class->get_size) (object, &w_parent, &h_parent);

	if ( (w_parent != 0.0) || (h_parent != 0.0) ) {
		*w = w_parent;
		*h = h_parent;
		return;
	}

	font = gnome_font_find_closest_from_weight_slant (
		ltext->private->font_family,
		ltext->private->font_weight,
		ltext->private->font_italic_flag,
		ltext->private->font_size);

	gtk_text_buffer_get_bounds (ltext->private->buffer, &start, &end);
	text = gtk_text_buffer_get_text (ltext->private->buffer,
					 &start, &end, FALSE);
	line = g_strsplit (text, "\n", -1);
	g_free (text);

	art_affine_identity (affine);

	*w = 0.0;
	*h = 0.0;
	for (i = 0; line[i] != NULL; i++) {

		glyphlist = gnome_glyphlist_from_text_dumb (font, 0,
							    0.0, 0.0,
							    line[i]);

		gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);

		gnome_glyphlist_unref (glyphlist);

		if ( bbox.x1 > *w ) *w = bbox.x1;

		if (i) {
			*h += ltext->private->line_spacing * ltext->private->font_size;
		} else {
			*h += ltext->private->font_size;
		}

	}

	if ( *h == 0.0 ) *h = ltext->private->font_size;

	*w += 2*GL_LABEL_TEXT_MARGIN;
	*h += 2*GL_LABEL_TEXT_MARGIN;


	g_strfreev (line);

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

	family_names = gnome_font_family_list ();
	if (g_list_find_custom (family_names, font_family, (GCompareFunc)g_utf8_collate)) {
		good_font_family = g_strdup (font_family);
	} else {
		if (family_names != NULL) {
			good_font_family = g_strdup (family_names->data); /* 1st entry */
		} else {
			good_font_family = g_strdup (font_family);
		}
	}
	gnome_font_family_list_free (family_names);

	if (ltext->private->font_family) {
		if (g_strcasecmp (ltext->private->font_family, good_font_family) == 0) {
			gl_debug (DEBUG_LABEL, "END (no change)");
			return;
		}
		g_free (ltext->private->font_family);
	}
	ltext->private->font_family = g_strdup (good_font_family);
	g_free (good_font_family);

	gl_debug (DEBUG_LABEL, "new font family = %s", ltext->private->font_family);

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

	if (ltext->private->font_size != font_size) {

		ltext->private->font_size = font_size;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set font weight method.                                         */
/*---------------------------------------------------------------------------*/
static void
set_font_weight (glLabelObject   *object,
		 GnomeFontWeight  font_weight)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->private->font_weight != font_weight) {

		ltext->private->font_weight = font_weight;
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

	if (ltext->private->font_italic_flag != font_italic_flag) {

		ltext->private->font_italic_flag = font_italic_flag;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  set text alignment method.                                      */
/*---------------------------------------------------------------------------*/
static void
set_text_alignment (glLabelObject    *object,
		    GtkJustification  text_alignment)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	if (ltext->private->just != text_alignment) {

		ltext->private->just = text_alignment;
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

	if (ltext->private->line_spacing != line_spacing) {

		ltext->private->line_spacing = line_spacing;
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

	if (!gl_color_node_equal (ltext->private->color_node, text_color_node)) {

		gl_color_node_free (&(ltext->private->color_node));
		ltext->private->color_node = gl_color_node_dup (text_color_node);
		
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

	return g_strdup (ltext->private->font_family);
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

	return ltext->private->font_size;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get font weight method.                                         */
/*---------------------------------------------------------------------------*/
static GnomeFontWeight
get_font_weight (glLabelObject   *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), GNOME_FONT_BOOK);

	return ltext->private->font_weight;
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

	return ltext->private->font_italic_flag;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  get text alignment method.                                      */
/*---------------------------------------------------------------------------*/
static GtkJustification
get_text_alignment (glLabelObject    *object)
{
	glLabelText    *ltext = (glLabelText *)object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), GTK_JUSTIFY_LEFT);

	return ltext->private->just;
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

	return ltext->private->line_spacing;
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

	return gl_color_node_dup (ltext->private->color_node);
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

	if (ltext->private->auto_shrink != auto_shrink) {

		ltext->private->auto_shrink = auto_shrink;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Query auto shrink flag.
/*****************************************************************************/
gboolean
gl_label_text_get_auto_shrink (glLabelText      *ltext)
{
	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), 0);

	return ltext->private->auto_shrink;
}
