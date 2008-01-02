/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  util.c:  various small utility functions
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include "util.h"

#include <string.h>
#include <glib.h>
#include <math.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcelllayout.h>

#define FRAC_EPSILON 0.00005


/****************************************************************************/
/* Append ".glabels" extension to filename if needed.                       */
/****************************************************************************/
gchar *
gl_util_add_extension (const gchar *orig_filename)
{
	gchar *new_filename, *extension;

	extension = strrchr (orig_filename, '.');
	if (extension == NULL) {
		new_filename = g_strconcat (orig_filename, ".glabels", NULL);
	} else {
		if (g_strcasecmp (extension, ".glabels") != 0) {
			new_filename =
			    g_strconcat (orig_filename, ".glabels", NULL);
		} else {
			new_filename = g_strdup (orig_filename);
		}
	}

	return new_filename;
}

/****************************************************************************/
/* Remove ".glabels" extension from filename if needed.                     */
/****************************************************************************/
gchar *
gl_util_remove_extension (const gchar *orig_filename)
{
	gchar *new_filename, *extension;

	new_filename = g_strdup (orig_filename);

	extension = strrchr (new_filename, '.');
	if (extension != NULL) {
		if (g_strcasecmp (extension, ".glabels") == 0) {
			*extension = 0; /* truncate string, rm extension */
		}
	}

	return new_filename;
}

/****************************************************************************/
/* Make sure we have an absolute path to filename.                          */
/****************************************************************************/
gchar *
gl_util_make_absolute (const gchar *filename)
{
	gchar *pwd, *absolute_filename;

	if (g_path_is_absolute (filename)) {
		absolute_filename = g_strdup (filename);
	} else {
		pwd = g_get_current_dir ();
		absolute_filename = g_build_filename (pwd, filename, NULL);
		g_free (pwd);
	}

	return absolute_filename;
}

/****************************************************************************/
/* Create fractional representation of number, if possible.                 */
/****************************************************************************/
gchar *
gl_util_fraction (gdouble x)
{
	static gdouble denom[] = { 1., 2., 3., 4., 8., 16., 32., 0. };
	gint i;
	gdouble product, remainder;
	gint n, d;

	for ( i=0; denom[i] != 0.0; i++ ) {
		product = x * denom[i];
		remainder = fabs(product - ((gint)(product+0.5)));
		if ( remainder < FRAC_EPSILON ) break;
	}

	if ( denom[i] == 0.0 ) {
		/* None of our denominators work. */
		return g_strdup_printf ("%.5g", x);
	}
	if ( denom[i] == 1.0 ) {
		/* Simple integer. */
		return g_strdup_printf ("%d", (gint)x);
	}
	n = (gint)( x * denom[i] + 0.5 );
	d = (gint)denom[i];
	if ( n > d ) {
		return g_strdup_printf ("%d_%d/%d", (n/d), (n%d), d);
	} else {
		return g_strdup_printf ("%d/%d", (n%d), d);
	}
}

/****************************************************************************/
/* Utilities to deal with PangoAlignment types.                             */
/****************************************************************************/
const gchar *
gl_util_align_to_string (PangoAlignment align)
{
	switch (align) {
	case PANGO_ALIGN_LEFT:
		return "Left";
	case PANGO_ALIGN_CENTER:
		return "Center";
	case PANGO_ALIGN_RIGHT:
		return "Right";
	default:
		return "?";
	}
}

PangoAlignment
gl_util_string_to_align (const gchar *string)
{

	if (g_strcasecmp (string, "Left") == 0) {
		return PANGO_ALIGN_LEFT;
	} else if (g_strcasecmp (string, "Center") == 0) {
		return PANGO_ALIGN_CENTER;
	} else if (g_strcasecmp (string, "Right") == 0) {
		return PANGO_ALIGN_RIGHT;
	} else {
		return PANGO_ALIGN_LEFT;
	}

}

/****************************************************************************/
/* Utilities to deal with PangoWeight types                                 */
/****************************************************************************/
const gchar *
gl_util_weight_to_string (PangoWeight weight)
{
	switch (weight) {
	case PANGO_WEIGHT_NORMAL:
		return "Regular";
	case PANGO_WEIGHT_BOLD:
		return "Bold";
	default:
		return "?";
	}
}

PangoWeight
gl_util_string_to_weight (const gchar *string)
{

	if (g_strcasecmp (string, "Regular") == 0) {
		return PANGO_WEIGHT_NORMAL;
	} else if (g_strcasecmp (string, "Bold") == 0) {
		return PANGO_WEIGHT_BOLD;
	} else {
		return PANGO_WEIGHT_NORMAL;
	}

}

/****************************************************************************/
/* Convienience function to set strings in a text combo_box from a GList    */
/****************************************************************************/
void
gl_util_combo_box_set_strings (GtkComboBox       *combo,
			       GList             *list)
{
	GtkTreeModel *model;
	GList        *p;

	g_return_if_fail (list);

	model = gtk_combo_box_get_model(combo);
	gtk_list_store_clear (GTK_LIST_STORE (model));

	for (p=list; p!=NULL; p=p->next) {
		if (p->data) {
			gtk_combo_box_append_text (combo, p->data);
		}
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  gl_util_combo_box_set_active_text support.                      */
/*---------------------------------------------------------------------------*/

typedef struct {
  const gchar *text;
  GtkTreeIter  iter;
  gboolean     found;
} TextSearchData;

static gboolean
search_text_func (GtkTreeModel *model,
		  GtkTreePath  *path,
		  GtkTreeIter  *iter,
		  gpointer      data)
{
  TextSearchData *search_data = (TextSearchData *)data;
  gchar          *text = NULL;

  gtk_tree_model_get (model, iter, 0, &text, -1);

  if (strcmp (text,search_data->text) == 0) {
    search_data->found = TRUE;
    search_data->iter  = *iter;
  }

  g_free (text);
  
  return FALSE;
}

/****************************************************************************/
/* Convienience function to set active text in a text combo_box from text   */
/****************************************************************************/
void
gl_util_combo_box_set_active_text (GtkComboBox       *combo,
				   const gchar       *text)
{
	GtkTreeModel   *model = gtk_combo_box_get_model(combo);

	g_return_if_fail (GTK_IS_LIST_STORE (model));

	if (!text) {

		gtk_combo_box_set_active (combo, -1);

	} else {
		TextSearchData  search_data;

		search_data.text        = text;
		search_data.found       = FALSE;

		gtk_tree_model_foreach (model, search_text_func, &search_data);
		if (search_data.found) {
			gtk_combo_box_set_active_iter (combo,
						       &search_data.iter);
		} else {
			gtk_combo_box_set_active (combo, -1);
		}    

	}

}

/****************************************************************************/
/* Convienience function to add a simple text model to an existing          */
/* combo_box.  This is needed since combo_boxes created with glade do not   */
/* use the gtk_combo_box_new_text() constructor.                            */
/****************************************************************************/
void
gl_util_combo_box_add_text_model (GtkComboBox       *combo)
{
	GtkCellRenderer *cell;
	GtkListStore *store;

	store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store));
	g_object_unref (store);

	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
					"text", 0,
					NULL);
}

/****************************************************************************/
/* Get list of available font families.                                     */
/****************************************************************************/
GList  *
gl_util_get_font_family_list (void)
{
	GList                *list = NULL;
	PangoFontMap         *fontmap;
	PangoContext         *context;
	PangoFontFamily     **families;
	gint                  n;
	gint                  i;
	gchar                *name;

	fontmap = pango_cairo_font_map_new ();
	context = pango_cairo_font_map_create_context (PANGO_CAIRO_FONT_MAP (fontmap));

	pango_context_list_families (context, &families, &n);

	for ( i=0; i<n; i++ )
	{
		name = g_strdup (pango_font_family_get_name (families[i]));
		list = g_list_append (list, name);
	}

	g_free (families);

	g_object_unref (context);
	g_object_unref (fontmap);

	return list;
}

/****************************************************************************/
/* Free previosly allocated list of font families.                          */
/****************************************************************************/
void    gl_util_font_family_list_free (GList *list)
{
        GList *p;

        for (p = list; p != NULL; p = p->next) {
                g_free (p->data);
                p->data = NULL;
        }

        g_list_free (list);
}


