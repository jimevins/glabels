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

#define DEFAULT_FONT_FAMILY      "Helvetica"
#define DEFAULT_FONT_SIZE        14.0
#define DEFAULT_FONT_WEIGHT      GNOME_FONT_BOOK
#define DEFAULT_FONT_ITALIC_FLAG FALSE
#define DEFAULT_JUST             GTK_JUSTIFY_LEFT
#define DEFAULT_COLOR            GNOME_CANVAS_COLOR (0,0,0)

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelTextPrivate {
	GList           *lines; /* list of glLabelTextNode lists */
	gchar           *font_family;
	gdouble          font_size;
	GnomeFontWeight  font_weight;
	gboolean         font_italic_flag;
	GtkJustification just;
	guint            color;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;

static guint instance = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_text_class_init    (glLabelTextClass *klass);
static void gl_label_text_instance_init (glLabelText      *ltext);
static void gl_label_text_finalize      (GObject          *object);

static void update_size                 (glLabelText      *ltext);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_text_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glLabelTextClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_text_class_init,
			NULL,
			NULL,
			sizeof (glLabelText),
			0,
			(GInstanceInitFunc) gl_label_text_instance_init,
		};

		type = g_type_register_static (GL_TYPE_LABEL_OBJECT,
					       "glLabelText", &info, 0);
	}

	return type;
}

static void
gl_label_text_class_init (glLabelTextClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_label_text_finalize;
}

static void
gl_label_text_instance_init (glLabelText *ltext)
{
	ltext->private = g_new0 (glLabelTextPrivate, 1);

	ltext->private->font_family      = g_strdup(DEFAULT_FONT_FAMILY);
	ltext->private->font_size        = DEFAULT_FONT_SIZE;
	ltext->private->font_weight      = DEFAULT_FONT_WEIGHT;
	ltext->private->font_italic_flag = DEFAULT_FONT_ITALIC_FLAG;
	ltext->private->just             = DEFAULT_JUST;
	ltext->private->color            = DEFAULT_COLOR;
}

static void
gl_label_text_finalize (GObject *object)
{
	glLabelText *ltext;

	g_return_if_fail (object && GL_IS_LABEL_TEXT (object));

	ltext = GL_LABEL_TEXT (object);

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
/* Duplicate object.                                                         */
/*****************************************************************************/
glLabelText *
gl_label_text_dup (glLabelText *ltext,
		   glLabel     *label)
{
	glLabelText      *new_ltext;
	GList            *lines;
	gchar            *font_family;
	gdouble           font_size;
	GnomeFontWeight   font_weight;
	gboolean          font_italic_flag;
	guint             color;
	GtkJustification  just;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));
	g_return_if_fail (label && GL_IS_LABEL (label));

	new_ltext = GL_LABEL_TEXT(gl_label_text_new (label));

	gl_label_object_copy_props (GL_LABEL_OBJECT(new_ltext), GL_LABEL_OBJECT(ltext));

	lines = gl_label_text_get_lines (ltext);
	gl_label_text_get_props (ltext,
				 &font_family, &font_size, &font_weight,
				 &font_italic_flag,
				 &color, &just);

	gl_label_text_set_lines (new_ltext, lines);
	gl_label_text_set_props (new_ltext,
				 font_family, font_size, font_weight,
				 font_italic_flag,
				 color, just);

	gl_text_node_lines_free (&lines);
	g_free (font_family);

	gl_debug (DEBUG_LABEL, "END");

	return new_ltext;
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_text_set_lines (glLabelText *ltext,
			 GList       *lines)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	gl_text_node_lines_free (&ltext->private->lines);
	ltext->private->lines = gl_text_node_lines_dup (lines);

	update_size (ltext);

	gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	gl_debug (DEBUG_LABEL, "END");
}

void
gl_label_text_set_props (glLabelText     *ltext,
			 gchar           *font_family,
			 gdouble          font_size,
			 GnomeFontWeight  font_weight,
			 gboolean         font_italic_flag,
			 guint            color,
			 GtkJustification just)
{
	GdkPixbuf *pixbuf;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	g_free (ltext->private->font_family);

	ltext->private->font_family      = g_strdup (font_family);
	ltext->private->font_size        = font_size;
	ltext->private->font_weight      = font_weight;
	ltext->private->font_italic_flag = font_italic_flag;
	ltext->private->color            = color;
	ltext->private->just             = just;

	gl_debug (DEBUG_LABEL, "just = %d", ltext->private->just);

	update_size (ltext);

	gl_label_object_emit_changed (GL_LABEL_OBJECT(ltext));

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
GList *
gl_label_text_get_lines (glLabelText *ltext)
{
	g_return_val_if_fail (ltext && GL_IS_LABEL_TEXT (ltext), NULL);

	return gl_text_node_lines_dup (ltext->private->lines);
}

void
gl_label_text_get_props (glLabelText      *ltext,
			 gchar           **font_family,
			 gdouble          *font_size,
			 GnomeFontWeight  *font_weight,
			 gboolean         *font_italic_flag,
			 guint            *color,
			 GtkJustification *just)
{
	g_return_if_fail (ltext && GL_IS_LABEL_TEXT (ltext));

	*font_family      = g_strdup (ltext->private->font_family);
	*font_size        = ltext->private->font_size;
	*font_weight      = ltext->private->font_weight;
	*font_italic_flag = ltext->private->font_italic_flag;
	*color            = ltext->private->color;
	*just             = ltext->private->just;

	gl_debug (DEBUG_LABEL, "just = %d", *just);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update object size.                                             */
/*---------------------------------------------------------------------------*/
static void
update_size (glLabelText *ltext)
{
	gdouble         w, h;
	GnomeFont      *font;
	gchar          *text;
	gchar         **line;
	gint            i;
	GnomeGlyphList *glyphlist;
	ArtDRect        bbox;
	gdouble         affine[6];

	gl_debug (DEBUG_LABEL, "START");

	font = gnome_font_find_closest_from_weight_slant (
		ltext->private->font_family,
		ltext->private->font_weight,
		ltext->private->font_italic_flag,
		ltext->private->font_size);

	text = gl_text_node_lines_expand (ltext->private->lines, NULL);
	line = g_strsplit (text, "\n", -1);
	g_free (text);

	art_affine_identity (affine);

	w = 0.0;
	h = 0.0;
	for (i = 0; line[i] != NULL; i++) {

		glyphlist = gnome_glyphlist_from_text_dumb (font, 0,
							    0.0, 0.0,
							    line[i]);

		gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);

		if ( bbox.x1 > w ) w = bbox.x1;

		h += ltext->private->font_size;

	}

	g_strfreev (line);

	gl_label_object_set_size (GL_LABEL_OBJECT(ltext), w, h);

	gl_debug (DEBUG_LABEL, "END");
}

