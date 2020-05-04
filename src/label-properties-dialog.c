/*
 *  label-properties-dialog.c
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

#include "label-properties-dialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "builder-util.h"
#include "prefs.h"
#include "mini-preview.h"
#include "window.h"
#include "debug.h"


#define MINI_PREVIEW_MIN_WIDTH  270
#define MINI_PREVIEW_MIN_HEIGHT 360


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelPropertiesDialogPrivate
{
	glLabel        *label;
	GtkBox         *box;
	GtkWidget      *preview;
	GtkLabel       *vendor;
	GtkLabel       *partnum;
	GtkLabel       *description;
	GtkLabel       *dimensions;
	GtkLabel       *medium;
	GtkLabel       *layout;
	GtkLabel       *margins;
	GtkLabel       *similar_label;
	GtkLabel       *similar;
	GtkLabel       *orientation_label;
	GtkBox         *orientation_box;
	GtkRadioButton *normal_orientation;
	GtkRadioButton *rotated_orientation;
};



/*========================================================*/
/* Private function prototypes                            */
/*========================================================*/

static void gl_label_properties_dialog_finalize    (GObject *object);

static void gl_label_properties_dialog_rotation_cb (glLabelPropertiesDialog *dialog);

static void gl_label_properties_dialog_response_cb (glLabelPropertiesDialog *dialog,
                                                    gint                     response);

static void gl_label_properties_dialog_construct   (glLabelPropertiesDialog *dialog,
                                                    glLabel                 *label);


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelPropertiesDialog, gl_label_properties_dialog, GTK_TYPE_DIALOG)



static void
gl_label_properties_dialog_class_init (glLabelPropertiesDialogClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_UI, "");

  	object_class->finalize = gl_label_properties_dialog_finalize;
}


static void
gl_label_properties_dialog_rotation_cb (glLabelPropertiesDialog *dialog)
{
	gboolean  rotate_flag;
	glWindow *window;
	glView   *view;

	gl_debug (DEBUG_UI, "START");

	rotate_flag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->rotated_orientation));
	gl_mini_preview_set_rotate (GL_MINI_PREVIEW (dialog->priv->preview), rotate_flag);
	gl_label_set_rotate_flag (dialog->priv->label, rotate_flag, TRUE);

	window = GL_WINDOW (gtk_window_get_transient_for (GTK_WINDOW (dialog)));
	view = GL_VIEW (window->view);
	if (view->zoom_to_fit_flag)
		gl_view_zoom_to_fit (view);

	gl_debug (DEBUG_UI, "END");
}


static void
gl_label_properties_dialog_response_cb (glLabelPropertiesDialog *dialog,
                                        gint                     response)
{
	gl_debug (DEBUG_UI, "START");

	switch (response)
	{
	case GL_RESPONSE_SELECT_OTHER:
	case GTK_RESPONSE_CLOSE:
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_widget_hide (GTK_WIDGET (dialog));
		break;
	default:
		g_error ("response = %d", response);
		g_assert_not_reached ();
	}

	gl_debug (DEBUG_UI, "END");
}


static void
gl_label_properties_dialog_init (glLabelPropertiesDialog *dialog)
{
        gchar             *builder_filename;
	GtkBuilder        *builder;
        static gchar      *object_ids[] = {"label_properties_box", NULL};
        GError            *error = NULL;
	GtkWidget         *vbox;

  	gl_debug (DEBUG_UI, "START");

	dialog->priv = g_new0 (glLabelPropertiesDialogPrivate, 1);

        vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

	/*Translators: Button label to choose different template*/
	gtk_dialog_add_button (GTK_DIALOG (dialog), _("Choose _other…"), GL_RESPONSE_SELECT_OTHER);
	gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Close"), GTK_RESPONSE_CLOSE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);

	builder = gtk_builder_new ();
	builder_filename = g_build_filename (GLABELS_DATA_DIR, "ui", "label-properties-dialog.ui", NULL);
	gtk_builder_add_objects_from_file (builder, builder_filename, object_ids, &error);
	g_free (builder_filename);
	if (error) {
		g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
		g_error_free (error);
		return;
	}

        gl_builder_util_get_widgets (builder,
	                             "label_properties_box", &dialog->priv->box,
	                             "vendor_value", &dialog->priv->vendor,
	                             "partnum_value", &dialog->priv->partnum,
	                             "description_value", &dialog->priv->description,
	                             "dimensions_value", &dialog->priv->dimensions,
	                             "medium_value", &dialog->priv->medium,
	                             "layout_value", &dialog->priv->layout,
	                             "margins_value", &dialog->priv->margins,
	                             "similar_label", &dialog->priv->similar_label,
	                             "similar_value", &dialog->priv->similar,
	                             "orientation_label", &dialog->priv->orientation_label,
	                             "orientation_box", &dialog->priv->orientation_box,
	                             "orientation_normal_radio", &dialog->priv->normal_orientation,
	                             "orientation_rotated_radio", &dialog->priv->rotated_orientation,
	                             NULL);

	gtk_container_add (GTK_CONTAINER (vbox), GTK_WIDGET (dialog->priv->box));

	dialog->priv->preview = gl_mini_preview_new (MINI_PREVIEW_MIN_HEIGHT, MINI_PREVIEW_MIN_WIDTH);
        gtk_box_pack_start (dialog->priv->box, GTK_WIDGET (dialog->priv->preview), FALSE, FALSE, 0);
	gtk_widget_show_all (dialog->priv->preview);

	g_object_unref (G_OBJECT (builder));

	gl_debug (DEBUG_UI, "END");
}


static void
gl_label_properties_dialog_finalize (GObject *object)
{
	glLabelPropertiesDialog* dialog = GL_LABEL_PROPERTIES_DIALOG (object);

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_LABEL_PROPERTIES_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);


	G_OBJECT_CLASS (gl_label_properties_dialog_parent_class)->finalize (object);

	gl_debug (DEBUG_UI, "END");
}


/* FIXME: Move this function to library */
static void
lgl_template_get_margins (const lglTemplate *template,
                          gdouble           *hmargin,
                          gdouble           *vmargin)
{
	GList                   *p, *q;
	const lglTemplateFrame  *frame;
	const lglTemplateLayout *layout;

	/* Set margins to maximum */
	*hmargin = template->page_width;
	*vmargin = template->page_height;

	/* and search lowest margin in layouts */
	for (p = template->frames; p; p = p->next)
	{
		frame = (lglTemplateFrame *)p->data;
		for (q = frame->all.layouts; q; q = q->next)
		{
			layout = (lglTemplateLayout *)q->data;
			if (layout->x0 < *hmargin)
				*hmargin = layout->x0;
			if (layout->y0 < *vmargin)
				*vmargin = layout->y0;
		}
	}
}

/* FIXME: Move this function to library */
gchar *
lgl_template_get_margins_string (const lglTemplate *template)
{
	lglUnits  units;
	gdouble   units_per_point;
	gdouble   hmargin, vmargin;
	gchar    *hmargin_str, *vmargin_str;
	gchar    *margins_str;

	units = gl_prefs_model_get_units (gl_prefs);
	units_per_point = lgl_units_get_units_per_point (units);

	lgl_template_get_margins (template, &hmargin, &vmargin);

	if (units == LGL_UNITS_INCH)
	{
		hmargin_str = lgl_str_format_fraction (hmargin*units_per_point);
		vmargin_str = lgl_str_format_fraction (vmargin*units_per_point);
	}
	else
	{
		hmargin_str = g_strdup_printf ("%.5g", hmargin*units_per_point);
		vmargin_str = g_strdup_printf ("%.5g", vmargin*units_per_point);
	}

  	/*Translators: first param is numeric value of horizontal margine, second*/
	/*is numeric value of vertical margine and third is unit. Example:       */
	/*"10 / 12.5 mm (horizontal / vertical)"*/
	margins_str = g_strdup_printf (_("%s / %s %s (horizontal / vertical)"),
	                               hmargin_str, vmargin_str, lgl_units_get_name (units));

	g_free (hmargin_str);
	g_free (vmargin_str);

	return margins_str;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create properties widgets.                                          */
/*--------------------------------------------------------------------------*/
static void
gl_label_properties_dialog_construct (glLabelPropertiesDialog *dialog,
				      glLabel                 *label)
{
	const lglTemplate *template;
	gchar             *template_name;
	lglUnits           units;
	gdouble            units_per_point;
	lglVendor         *vendor;
	gchar             *text;
	gchar             *link;
	gchar             *escaped_url;
	GList             *list, *p;
	GString           *list_text;
	gboolean           rotate_flag;
	gdouble            w, h;

	gl_debug (DEBUG_UI, "START");

	dialog->priv->label = label;

	template = gl_label_get_template (label);
	template_name = lgl_template_get_name (template);
	units = gl_prefs_model_get_units (gl_prefs);
	units_per_point = lgl_units_get_units_per_point (units);

	/* Vendor */
	vendor = lgl_db_lookup_vendor_from_name (template->brand);
	if (vendor && vendor->url)
	{
		escaped_url = g_markup_escape_text (vendor->url, -1);
		link = g_strconcat ("<a href=\"", escaped_url , "\">", template->brand, "</a>", NULL);
  		gtk_label_set_markup (dialog->priv->vendor, link);
		g_free (escaped_url);
		g_free (link);
	}
	else
	{
		gtk_label_set_label (dialog->priv->vendor, template->brand);
	}

	/* Part number */
	if (template->product_url)
	{
		escaped_url = g_markup_escape_text (template->product_url, -1);
		link = g_strconcat ("<a href=\"", escaped_url, "\">", template->part, "</a>", NULL);
		gtk_label_set_markup (dialog->priv->partnum, link);
		g_free (escaped_url);
		g_free (link);
	}
	else
	{
		gtk_label_set_label (dialog->priv->partnum, template->part);
	}

	/* Description */
	gtk_label_set_label (dialog->priv->description, template->description);

	/* Dimensions */
	text = gl_label_get_dimensions_string (label);
	gtk_label_set_label (dialog->priv->dimensions, text);
	g_free (text);

	/* Medium */
	text = lgl_db_lookup_paper_name_from_id (template->paper_id);
	gtk_label_set_label (dialog->priv->medium, text);
	g_free (text);

	/* Layout */
	text = lgl_template_frame_get_layout_description (template->frames->data);
	gtk_label_set_label (dialog->priv->layout, text);
	g_free (text);

	/* Margins */
	text = lgl_template_get_margins_string (template);
	gtk_label_set_label (dialog->priv->margins, text);
	g_free (text);

	/* Similar products */
	list = lgl_db_get_similar_template_name_list (template_name);
	if (list)
	{
		list_text = g_string_new ("");
		for (p = list; p; p = p->next)
		{
			g_string_append (list_text, (char *)p->data);
			if (p->next)
				g_string_append_c (list_text, '\n');
		}
	        gtk_label_set_text (dialog->priv->similar, list_text->str);
		g_string_free (list_text, TRUE);
	}
	else
	{
		gtk_widget_hide (GTK_WIDGET (dialog->priv->similar_label));
		gtk_widget_hide (GTK_WIDGET (dialog->priv->similar));
	}

	/* Orientation */
	gl_label_get_size (label, &w, &h);
	if (w == h)
	{
		gtk_widget_hide (GTK_WIDGET (dialog->priv->orientation_label));
		gtk_widget_hide (GTK_WIDGET (dialog->priv->orientation_box));
		gl_mini_preview_set_draw_arrow (GL_MINI_PREVIEW (dialog->priv->preview), FALSE);
	}
	else
	{
		rotate_flag = gl_label_get_rotate_flag (label);
		if (rotate_flag)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->rotated_orientation), TRUE);
		else
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->normal_orientation), TRUE);
		gl_mini_preview_set_draw_arrow (GL_MINI_PREVIEW (dialog->priv->preview), TRUE);
        	gl_mini_preview_set_rotate (GL_MINI_PREVIEW (dialog->priv->preview), rotate_flag);
	}

	/* Preview */
	gl_mini_preview_set_template (GL_MINI_PREVIEW (dialog->priv->preview), template);

	g_free (template_name);
	g_free (vendor);

	g_signal_connect_swapped (G_OBJECT (dialog->priv->normal_orientation),
	                          "toggled",
	                          G_CALLBACK (gl_label_properties_dialog_rotation_cb),
	                          dialog);
	g_signal_connect_swapped (G_OBJECT (dialog->priv->rotated_orientation),
	                          "toggled",
	                          G_CALLBACK (gl_label_properties_dialog_rotation_cb),
	                          dialog);
	g_signal_connect (G_OBJECT (dialog),
	                  "response",
	                  G_CALLBACK (gl_label_properties_dialog_response_cb),
	                  NULL);

	gl_debug (DEBUG_UI, "END");
}


/*****************************************************************************/
/* NEW document properties dialog.                                                   */
/*****************************************************************************/
GtkWidget*
gl_label_properties_dialog_new (glLabel *label,
				GtkWindow *parent)
{
	glLabelPropertiesDialog *dialog;

	g_assert (GL_IS_LABEL (label));

	gl_debug (DEBUG_UI, "START");

	dialog = GL_LABEL_PROPERTIES_DIALOG (g_object_new (GL_TYPE_LABEL_PROPERTIES_DIALOG, NULL));

	if (parent)
		gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

	gl_label_properties_dialog_construct (GL_LABEL_PROPERTIES_DIALOG (dialog),
	                                      label);

	gl_debug (DEBUG_UI, "END");

	return GTK_WIDGET (dialog);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
