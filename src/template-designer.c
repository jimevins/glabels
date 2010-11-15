/*
 *  template-designer.c
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#include "template-designer.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>

#include <libglabels.h>
#include "prefs.h"
#include "mini-preview.h"
#include "mini-preview-pixbuf-cache.h"
#include "print-op-dialog.h"
#include "combo-util.h"
#include "builder-util.h"
#include "units-util.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define DEFAULT_MARGIN 9.0

#define DEFAULT_RECT_W      252.0
#define DEFAULT_RECT_H      144.0
#define DEFAULT_RECT_R        0.0
#define DEFAULT_RECT_WASTE    0.0 /* Should never exceed 1/2 the distance between items. */

#define DEFAULT_ELLIPSE_W      252.0
#define DEFAULT_ELLIPSE_H      144.0
#define DEFAULT_ELLIPSE_WASTE    0.0 /* Should never exceed 1/2 the distance between items. */

#define DEFAULT_ROUND_R      72.0
#define DEFAULT_ROUND_WASTE   0.0 /* Should never exceed 1/2 the distance between items. */

#define DEFAULT_CD_RADIUS   166.5
#define DEFAULT_CD_HOLE      58.5
#define DEFAULT_CD_WASTE      9.0 /* Should never exceed 1/2 the distance between items. */

#define DELTA 0.01
#define MAX_PAGE_DIM_POINTS 5000.0


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glTemplateDesignerPrivate
{
        gboolean         edit_flag;
        gchar           *edit_brand;
        gchar           *edit_part;

        GtkBuilder      *builder;

	/* Assistant pages */
	GtkWidget       *start_page;
	GtkWidget       *name_page;
	GtkWidget       *pg_size_page;
	GtkWidget       *shape_page;
	GtkWidget       *rect_size_page;
	GtkWidget       *ellipse_size_page;
	GtkWidget       *round_size_page;
	GtkWidget       *cd_size_page;
	GtkWidget       *nlayouts_page;
	GtkWidget       *layout_page;
	GtkWidget       *finish_page;

	/* Name page controls */
	GtkWidget       *brand_entry;
	GtkWidget       *part_num_entry;
        GtkWidget       *name_warning_image;
        GtkWidget       *name_warning_label;
	GtkWidget       *description_entry;

	/* Page size page controls */
	GtkWidget       *pg_size_combo_hbox;
	GtkWidget       *pg_size_combo;
	GtkWidget       *pg_w_spin;
	GtkWidget       *pg_h_spin;
	GtkWidget       *pg_w_units_label;
	GtkWidget       *pg_h_units_label;

	/* Shape page controls */
	GtkWidget       *shape_rect_radio;
	GtkWidget       *shape_ellipse_radio;
	GtkWidget       *shape_round_radio;
	GtkWidget       *shape_cd_radio;

	/* Label size (rectangular) page controls */
	GtkWidget       *rect_image;
	GtkWidget       *rect_w_spin;
	GtkWidget       *rect_h_spin;
	GtkWidget       *rect_r_spin;
	GtkWidget       *rect_x_waste_spin;
	GtkWidget       *rect_y_waste_spin;
	GtkWidget       *rect_margin_spin;
	GtkWidget       *rect_w_units_label;
	GtkWidget       *rect_h_units_label;
	GtkWidget       *rect_r_units_label;
	GtkWidget       *rect_x_waste_units_label;
	GtkWidget       *rect_y_waste_units_label;
	GtkWidget       *rect_margin_units_label;

	/* Label size (elliptical) page controls */
	GtkWidget       *ellipse_image;
	GtkWidget       *ellipse_w_spin;
	GtkWidget       *ellipse_h_spin;
	GtkWidget       *ellipse_waste_spin;
	GtkWidget       *ellipse_margin_spin;
	GtkWidget       *ellipse_w_units_label;
	GtkWidget       *ellipse_h_units_label;
	GtkWidget       *ellipse_waste_units_label;
	GtkWidget       *ellipse_margin_units_label;

	/* Label size (round) page controls */
	GtkWidget       *round_image;
	GtkWidget       *round_r_spin;
	GtkWidget       *round_waste_spin;
	GtkWidget       *round_margin_spin;
	GtkWidget       *round_r_units_label;
	GtkWidget       *round_waste_units_label;
	GtkWidget       *round_margin_units_label;

	/* Label size (cd) page controls */
	GtkWidget       *cd_image;
	GtkWidget       *cd_radius_spin;
	GtkWidget       *cd_hole_spin;
	GtkWidget       *cd_w_spin;
	GtkWidget       *cd_h_spin;
	GtkWidget       *cd_waste_spin;
	GtkWidget       *cd_margin_spin;
	GtkWidget       *cd_radius_units_label;
	GtkWidget       *cd_hole_units_label;
	GtkWidget       *cd_w_units_label;
	GtkWidget       *cd_h_units_label;
	GtkWidget       *cd_waste_units_label;
	GtkWidget       *cd_margin_units_label;

	/* Number of layouts controls */
	GtkWidget       *nlayouts_image1;
	GtkWidget       *nlayouts_image2;
	GtkWidget       *nlayouts_spin;

	/* Layout page controls */
	GtkWidget       *layout1_head_label;
	GtkWidget       *layout1_nx_spin;
	GtkWidget       *layout1_ny_spin;
	GtkWidget       *layout1_x0_spin;
	GtkWidget       *layout1_y0_spin;
	GtkWidget       *layout1_dx_spin;
	GtkWidget       *layout1_dy_spin;
	GtkWidget       *layout2_head_label;
	GtkWidget       *layout2_nx_spin;
	GtkWidget       *layout2_ny_spin;
	GtkWidget       *layout2_x0_spin;
	GtkWidget       *layout2_y0_spin;
	GtkWidget       *layout2_dx_spin;
	GtkWidget       *layout2_dy_spin;
	GtkWidget       *layout_x0_units_label;
	GtkWidget       *layout_y0_units_label;
	GtkWidget       *layout_dx_units_label;
	GtkWidget       *layout_dy_units_label;
	GtkWidget       *mini_preview_vbox;
	GtkWidget       *layout_mini_preview;
	GtkWidget       *layout_test_button;

	/* Units related stuff */
	const gchar     *units_string;
	gdouble          units_per_point;
	gdouble          climb_rate;
	gint             digits;

        /* Prevent recursion */
	gboolean    stop_signals;
};

/* Page numbers for traversing GtkAssistant */
enum {
        START_PAGE_NUM = 0,
        NAME_PAGE_NUM,
        PG_SIZE_PAGE_NUM,
        SHAPE_PAGE_NUM,
        RECT_SIZE_PAGE_NUM,
        ELLIPSE_SIZE_PAGE_NUM,
        ROUND_SIZE_PAGE_NUM,
        CD_SIZE_PAGE_NUM,
        NLAYOUTS_PAGE_NUM,
        LAYOUT_PAGE_NUM,
        FINISH_PAGE_NUM
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_template_designer_finalize   	  (GObject                 *object);
static void gl_template_designer_construct        (glTemplateDesigner      *dialog);

static void     construct_start_page              (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_name_page               (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_pg_size_page            (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_shape_page              (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_rect_size_page          (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_ellipse_size_page       (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_round_size_page         (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_cd_size_page            (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_nlayouts_page           (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_layout_page             (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     construct_finish_page             (glTemplateDesigner      *dialog,
						   GdkPixbuf               *logo);

static void     cancel_cb                         (glTemplateDesigner      *dialog);
static void     apply_cb                          (glTemplateDesigner      *dialog);
static void     close_cb                          (glTemplateDesigner      *dialog);
static void     prepare_cb                        (glTemplateDesigner      *dialog,
                                                   GtkWidget               *page);

static gint     forward_page_function             (gint                     current_page,
                                                   gpointer                 data);

static void     name_page_changed_cb              (glTemplateDesigner      *dialog);

static void     pg_size_page_changed_cb           (glTemplateDesigner      *dialog);

static void     rect_size_page_prepare_cb         (glTemplateDesigner      *dialog);

static void     ellipse_size_page_prepare_cb      (glTemplateDesigner      *dialog);

static void     round_size_page_prepare_cb        (glTemplateDesigner      *dialog);

static void     cd_size_page_prepare_cb           (glTemplateDesigner      *dialog);

static void     layout_page_prepare_cb            (glTemplateDesigner      *dialog);

static void     layout_page_changed_cb            (glTemplateDesigner      *dialog);

static void     print_test_cb                     (glTemplateDesigner      *dialog);

static lglTemplate *build_template                (glTemplateDesigner      *dialog);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glTemplateDesigner, gl_template_designer, GTK_TYPE_ASSISTANT);


static void
gl_template_designer_class_init (glTemplateDesignerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

  	gl_template_designer_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_template_designer_finalize;  	
}


static void
gl_template_designer_init (glTemplateDesigner *dialog)
{
        gchar             *builder_filename;
        static gchar      *object_ids[] = { "start_page",
                                            "name_page",
                                            "pg_size_page",
                                            "shape_page",
                                            "rect_size_page",
                                            "ellipse_size_page",
                                            "round_size_page",
                                            "cd_size_page",
                                            "nlayouts_page",
                                            "layout_page",
                                            "finish_page",
                                            "adjustment1",  "adjustment2",  "adjustment3",
                                            "adjustment4",  "adjustment5",  "adjustment6",
                                            "adjustment7",  "adjustment8",  "adjustment9",
                                            "adjustment10", "adjustment11", "adjustment12",
                                            "adjustment13", "adjustment14", "adjustment15",
                                            "adjustment16", "adjustment17", "adjustment18",
                                            "adjustment19", "adjustment20", "adjustment21",
                                            "adjustment22", "adjustment23", "adjustment24",
                                            "adjustment25", "adjustment26", "adjustment27",
                                            "adjustment28", "adjustment29", "adjustment30",
                                            NULL };
        GError            *error = NULL;

	gl_debug (DEBUG_TEMPLATE, "START");

	dialog->priv = g_new0 (glTemplateDesignerPrivate, 1);

        dialog->priv->builder = gtk_builder_new ();
        builder_filename = g_build_filename (GLABELS_DATA_DIR, "ui", "template-designer.ui", NULL);
        gtk_builder_add_objects_from_file (dialog->priv->builder, builder_filename, object_ids, &error);
        g_free (builder_filename);
	if (error) {
		g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
                g_error_free (error);
		return;
	}

	gl_debug (DEBUG_TEMPLATE, "END");

        return;
}


static void 
gl_template_designer_finalize (GObject *object)
{
	glTemplateDesigner* dialog = GL_TEMPLATE_DESIGNER (object);
	
	gl_debug (DEBUG_TEMPLATE, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_TEMPLATE_DESIGNER (dialog));
	g_return_if_fail (dialog->priv != NULL);

        g_free (dialog->priv->edit_brand);
        g_free (dialog->priv->edit_part);

        if (dialog->priv->builder)
        {
                g_object_unref (dialog->priv->builder);
        }
	g_free (dialog->priv);

	G_OBJECT_CLASS (gl_template_designer_parent_class)->finalize (object);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*****************************************************************************/
/* NEW preferences dialog.                                                   */
/*****************************************************************************/
GtkWidget*
gl_template_designer_new (GtkWindow *parent)
{
	GtkWidget *dialog;

	gl_debug (DEBUG_TEMPLATE, "START");

	dialog = GTK_WIDGET (g_object_new (GL_TYPE_TEMPLATE_DESIGNER, NULL));

	if (parent)
		gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
	
	gl_template_designer_construct (GL_TEMPLATE_DESIGNER(dialog));


	gl_debug (DEBUG_TEMPLATE, "END");

	return dialog;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_template_designer_construct (glTemplateDesigner *dialog)
{
        lglUnits    units;
        gchar      *logo_filename;
	GdkPixbuf  *logo;

	gl_debug (DEBUG_TEMPLATE, "START");

	g_return_if_fail (dialog && GL_IS_TEMPLATE_DESIGNER (dialog));
	g_return_if_fail (dialog->priv != NULL);

	/* Initialize units stuff from prefs */
        units = gl_prefs_model_get_units (gl_prefs);
	dialog->priv->units_string    = lgl_units_get_name (units);
        dialog->priv->units_per_point = lgl_units_get_units_per_point (units);
        dialog->priv->climb_rate      = gl_units_util_get_step_size (units);
        dialog->priv->digits          = gl_units_util_get_precision (units);

	gtk_window_set_title (GTK_WINDOW(dialog), _("New gLabels Template"));

        logo_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "template-designer.png", NULL);
	logo = gdk_pixbuf_new_from_file (logo_filename, NULL);
        g_free (logo_filename);

        /* Construct and append pages (must be same order as PAGE_NUM enums. */
	construct_start_page (dialog, logo);
	construct_name_page (dialog, logo);
	construct_pg_size_page (dialog, logo);
	construct_shape_page (dialog, logo);
	construct_rect_size_page (dialog, logo);
	construct_ellipse_size_page (dialog, logo);
	construct_round_size_page (dialog, logo);
	construct_cd_size_page (dialog, logo);
	construct_nlayouts_page (dialog, logo);
	construct_layout_page (dialog, logo);
	construct_finish_page (dialog, logo);

        gtk_assistant_set_forward_page_func (GTK_ASSISTANT (dialog),
                                             forward_page_function,
                                             dialog,
                                             NULL);

	/* signals */
	g_signal_connect_swapped (G_OBJECT(dialog), "cancel",
				  G_CALLBACK(cancel_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog), "apply",
				  G_CALLBACK(apply_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog), "close",
				  G_CALLBACK(close_cb), dialog);
	g_signal_connect (G_OBJECT(dialog), "prepare",
                          G_CALLBACK(prepare_cb), NULL);

        gtk_widget_show_all (GTK_WIDGET(dialog));   

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct start page.                                          */
/*--------------------------------------------------------------------------*/
static void
construct_start_page (glTemplateDesigner      *dialog,
		      GdkPixbuf               *logo)
{
	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "start_page", &dialog->priv->start_page,
                                     NULL);

        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->start_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->start_page,
                                      _("Welcome"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->start_page,
                                             logo);
        gtk_assistant_set_page_type (GTK_ASSISTANT (dialog),
                                     dialog->priv->start_page,
                                     GTK_ASSISTANT_PAGE_INTRO);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->start_page,
                                         TRUE);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct name page.                                           */
/*--------------------------------------------------------------------------*/
static void
construct_name_page (glTemplateDesigner      *dialog,
		     GdkPixbuf               *logo)
{
	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "name_page",          &dialog->priv->name_page,
                                     "brand_entry",        &dialog->priv->brand_entry,
                                     "part_num_entry",     &dialog->priv->part_num_entry,
                                     "name_warning_image", &dialog->priv->name_warning_image,
                                     "name_warning_label", &dialog->priv->name_warning_label,
                                     "description_entry",  &dialog->priv->description_entry,
                                     NULL);


        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->name_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->name_page,
                                      _("Name and Description"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->name_page,
                                             logo);

	/* Connect a handler that listens for changes in these widgets */
	/* This controls whether we can progress to the next page. */
	g_signal_connect_swapped (G_OBJECT(dialog->priv->brand_entry), "changed",
				  G_CALLBACK(name_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->part_num_entry), "changed",
				  G_CALLBACK(name_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->description_entry), "changed",
				  G_CALLBACK(name_page_changed_cb), dialog);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE. Construct page size page.                                       */
/*--------------------------------------------------------------------------*/
static void
construct_pg_size_page (glTemplateDesigner      *dialog,
			GdkPixbuf               *logo)
{
	GList           *page_sizes;
	const gchar     *default_page_size_id;
	gchar           *default_page_size_name;

	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "pg_size_page",        &dialog->priv->pg_size_page,
                                     "pg_size_combo_hbox",  &dialog->priv->pg_size_combo_hbox,
                                     "pg_w_spin",           &dialog->priv->pg_w_spin,
                                     "pg_h_spin",           &dialog->priv->pg_h_spin,
                                     "pg_w_units_label",    &dialog->priv->pg_w_units_label,
                                     "pg_h_units_label",    &dialog->priv->pg_h_units_label,
                                     NULL);


        dialog->priv->pg_size_combo = gtk_combo_box_text_new ();
        gtk_box_pack_start (GTK_BOX (dialog->priv->pg_size_combo_hbox), dialog->priv->pg_size_combo, FALSE, FALSE, 0);

        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->pg_size_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->pg_size_page,
                                      _("Page Size"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->pg_size_page,
                                             logo);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->pg_size_page,
                                         TRUE);

	/* Load page size combo */
	page_sizes = lgl_db_get_paper_name_list ();
	gl_combo_util_set_strings (GTK_COMBO_BOX_TEXT (dialog->priv->pg_size_combo), page_sizes);
	lgl_db_free_paper_name_list (page_sizes);
	default_page_size_id = gl_prefs_model_get_default_page_size (gl_prefs);
	default_page_size_name = lgl_db_lookup_paper_name_from_id (default_page_size_id);
	gl_combo_util_set_active_text (GTK_COMBO_BOX (dialog->priv->pg_size_combo), default_page_size_name);
	g_free (default_page_size_name);

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->pg_w_units_label),
			    dialog->priv->units_string);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->pg_w_spin),
                                   0.0, MAX_PAGE_DIM_POINTS*dialog->priv->units_per_point);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->pg_h_units_label),
			    dialog->priv->units_string);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->pg_h_spin),
                                   0.0, MAX_PAGE_DIM_POINTS*dialog->priv->units_per_point);

	/* Connect a handler that listens for changes in these widgets */
	/* This controls sensitivity of related widgets. */
	g_signal_connect_swapped (G_OBJECT(dialog->priv->pg_size_combo), "changed",
				  G_CALLBACK(pg_size_page_changed_cb), dialog);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct shape page.                                          */
/*--------------------------------------------------------------------------*/
static void
construct_shape_page (glTemplateDesigner      *dialog,
		      GdkPixbuf               *logo)
{
	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "shape_page",          &dialog->priv->shape_page,
                                     "shape_rect_radio",    &dialog->priv->shape_rect_radio,
                                     "shape_ellipse_radio", &dialog->priv->shape_ellipse_radio,
                                     "shape_round_radio",   &dialog->priv->shape_round_radio,
                                     "shape_cd_radio",      &dialog->priv->shape_cd_radio,
                                     NULL);


        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->shape_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->shape_page,
                                      _("Label or Card Shape"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->shape_page,
                                             logo);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->shape_page,
                                         TRUE);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct rect size page.                                      */
/*--------------------------------------------------------------------------*/
static void
construct_rect_size_page (glTemplateDesigner      *dialog,
			  GdkPixbuf               *logo)
{
        gchar           *pixbuf_filename;
	GdkPixbuf       *pixbuf;

	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "rect_size_page",           &dialog->priv->rect_size_page,
                                     "rect_image",               &dialog->priv->rect_image,
                                     "rect_w_spin",              &dialog->priv->rect_w_spin,
                                     "rect_h_spin",              &dialog->priv->rect_h_spin,
                                     "rect_r_spin",              &dialog->priv->rect_r_spin,
                                     "rect_x_waste_spin",        &dialog->priv->rect_x_waste_spin,
                                     "rect_y_waste_spin",        &dialog->priv->rect_y_waste_spin,
                                     "rect_margin_spin",         &dialog->priv->rect_margin_spin,
                                     "rect_w_units_label",       &dialog->priv->rect_w_units_label,
                                     "rect_h_units_label",       &dialog->priv->rect_h_units_label,
                                     "rect_r_units_label",       &dialog->priv->rect_r_units_label,
                                     "rect_x_waste_units_label", &dialog->priv->rect_x_waste_units_label,
                                     "rect_y_waste_units_label", &dialog->priv->rect_y_waste_units_label,
                                     "rect_margin_units_label",  &dialog->priv->rect_margin_units_label,
                                     NULL);


        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->rect_size_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->rect_size_page,
                                      _("Label or Card Size"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->rect_size_page,
                                             logo);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->rect_size_page,
                                         TRUE);

	/* Initialize illustration. */
        pixbuf_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "ex-rect-size.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file (pixbuf_filename, NULL);
        g_free (pixbuf_filename);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dialog->priv->rect_image), pixbuf);

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->rect_w_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->rect_h_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->rect_r_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->rect_r_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->rect_r_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->rect_x_waste_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->rect_y_waste_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->rect_margin_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->rect_margin_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->rect_margin_units_label),
			    dialog->priv->units_string);

	/* Load some realistic defaults. */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin),
				   DEFAULT_RECT_W * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin),
				   DEFAULT_RECT_H * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_r_spin),
				   DEFAULT_RECT_R * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin),
				   DEFAULT_RECT_WASTE * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin),
				   DEFAULT_RECT_WASTE * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_margin_spin),
				   DEFAULT_MARGIN * dialog->priv->units_per_point);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct ellipse size page.                                   */
/*--------------------------------------------------------------------------*/
static void
construct_ellipse_size_page (glTemplateDesigner      *dialog,
                             GdkPixbuf               *logo)
{
        gchar           *pixbuf_filename;
	GdkPixbuf       *pixbuf;

	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "ellipse_size_page",           &dialog->priv->ellipse_size_page,
                                     "ellipse_image",               &dialog->priv->ellipse_image,
                                     "ellipse_w_spin",              &dialog->priv->ellipse_w_spin,
                                     "ellipse_h_spin",              &dialog->priv->ellipse_h_spin,
                                     "ellipse_waste_spin",          &dialog->priv->ellipse_waste_spin,
                                     "ellipse_margin_spin",         &dialog->priv->ellipse_margin_spin,
                                     "ellipse_w_units_label",       &dialog->priv->ellipse_w_units_label,
                                     "ellipse_h_units_label",       &dialog->priv->ellipse_h_units_label,
                                     "ellipse_waste_units_label",   &dialog->priv->ellipse_waste_units_label,
                                     "ellipse_margin_units_label",  &dialog->priv->ellipse_margin_units_label,
                                     NULL);


        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->ellipse_size_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->ellipse_size_page,
                                      _("Label or Card Size"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->ellipse_size_page,
                                             logo);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->ellipse_size_page,
                                         TRUE);

	/* Initialize illustration. */
        pixbuf_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "ex-ellipse-size.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file (pixbuf_filename, NULL);
        g_free (pixbuf_filename);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dialog->priv->ellipse_image), pixbuf);

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->ellipse_w_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->ellipse_w_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->ellipse_w_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->ellipse_h_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->ellipse_h_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->ellipse_h_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->ellipse_waste_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->ellipse_waste_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->ellipse_waste_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->ellipse_margin_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->ellipse_margin_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->ellipse_margin_units_label),
			    dialog->priv->units_string);

	/* Load some realistic defaults. */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_w_spin),
				   DEFAULT_ELLIPSE_W * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_h_spin),
				   DEFAULT_ELLIPSE_H * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_waste_spin),
				   DEFAULT_ELLIPSE_WASTE * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_margin_spin),
				   DEFAULT_MARGIN * dialog->priv->units_per_point);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct round size page.                                     */
/*--------------------------------------------------------------------------*/
static void
construct_round_size_page (glTemplateDesigner      *dialog,
			   GdkPixbuf               *logo)
{
        gchar           *pixbuf_filename;
	GdkPixbuf       *pixbuf;

	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "round_size_page",          &dialog->priv->round_size_page,
                                     "round_image",              &dialog->priv->round_image,
                                     "round_r_spin",             &dialog->priv->round_r_spin,
                                     "round_waste_spin",         &dialog->priv->round_waste_spin,
                                     "round_margin_spin",        &dialog->priv->round_margin_spin,
                                     "round_r_units_label",      &dialog->priv->round_r_units_label,
                                     "round_waste_units_label",  &dialog->priv->round_waste_units_label,
                                     "round_margin_units_label", &dialog->priv->round_margin_units_label,
                                     NULL);


        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->round_size_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->round_size_page,
                                      _("Label Size (round)"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->round_size_page,
                                             logo);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->round_size_page,
                                         TRUE);

	/* Initialize illustration. */
        pixbuf_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "ex-round-size.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file (pixbuf_filename, NULL);
        g_free (pixbuf_filename);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dialog->priv->round_image), pixbuf);

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->round_r_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->round_r_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->round_r_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->round_waste_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->round_margin_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->round_margin_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->round_margin_units_label),
			    dialog->priv->units_string);

	/* Load some realistic defaults. */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_r_spin),
				   DEFAULT_ROUND_R * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin),
				   DEFAULT_ROUND_WASTE * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_margin_spin),
				   DEFAULT_MARGIN * dialog->priv->units_per_point);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct CD/DVD size page.                                    */
/*--------------------------------------------------------------------------*/
static void
construct_cd_size_page (glTemplateDesigner      *dialog,
			GdkPixbuf               *logo)
{
        gchar           *pixbuf_filename;
	GdkPixbuf       *pixbuf;

	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "cd_size_page",          &dialog->priv->cd_size_page,
                                     "cd_image",              &dialog->priv->cd_image,
                                     "cd_radius_spin",        &dialog->priv->cd_radius_spin,
                                     "cd_hole_spin",          &dialog->priv->cd_hole_spin,
                                     "cd_w_spin",             &dialog->priv->cd_w_spin,
                                     "cd_h_spin",             &dialog->priv->cd_h_spin,
                                     "cd_waste_spin",         &dialog->priv->cd_waste_spin,
                                     "cd_margin_spin",        &dialog->priv->cd_margin_spin,
                                     "cd_radius_units_label", &dialog->priv->cd_radius_units_label,
                                     "cd_hole_units_label",   &dialog->priv->cd_hole_units_label,
                                     "cd_w_units_label",      &dialog->priv->cd_w_units_label,
                                     "cd_h_units_label",      &dialog->priv->cd_h_units_label,
                                     "cd_waste_units_label",  &dialog->priv->cd_waste_units_label,
                                     "cd_margin_units_label", &dialog->priv->cd_margin_units_label,
                                     NULL);


        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->cd_size_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->cd_size_page,
                                      _("Label Size (CD/DVD)"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->cd_size_page,
                                             logo);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->cd_size_page,
                                         TRUE);

	/* Initialize illustration. */
        pixbuf_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "ex-cd-size.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file (pixbuf_filename, NULL);
        g_free (pixbuf_filename);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dialog->priv->cd_image), pixbuf);

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->cd_radius_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->cd_radius_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->cd_radius_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->cd_hole_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->cd_hole_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->cd_hole_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->cd_w_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->cd_w_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->cd_w_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->cd_h_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->cd_h_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->cd_h_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->cd_waste_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->cd_waste_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->cd_waste_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->cd_margin_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->cd_margin_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->cd_margin_units_label),
			    dialog->priv->units_string);

	/* Load some realistic defaults. */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_radius_spin),
				   DEFAULT_CD_RADIUS * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_hole_spin),
				   DEFAULT_CD_HOLE * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_waste_spin),
				   DEFAULT_CD_WASTE * dialog->priv->units_per_point);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_margin_spin),
				   DEFAULT_MARGIN * dialog->priv->units_per_point);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct number of layouts page.                              */
/*--------------------------------------------------------------------------*/
static void
construct_nlayouts_page (glTemplateDesigner      *dialog,
			 GdkPixbuf               *logo)
{
        gchar           *pixbuf_filename;
	GdkPixbuf       *pixbuf;

	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "nlayouts_page",   &dialog->priv->nlayouts_page,
                                     "nlayouts_image1", &dialog->priv->nlayouts_image1,
                                     "nlayouts_image2", &dialog->priv->nlayouts_image2,
                                     "nlayouts_spin",   &dialog->priv->nlayouts_spin,
                                     NULL);


        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->nlayouts_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->nlayouts_page,
                                      _("Number of Layouts"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->nlayouts_page,
                                             logo);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->nlayouts_page,
                                         TRUE);

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->nlayouts_spin),
                                   1, 2);


	/* Initialize illustrations. */
        pixbuf_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "ex-1layout.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file (pixbuf_filename, NULL);
        g_free (pixbuf_filename);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dialog->priv->nlayouts_image1), pixbuf);
        pixbuf_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "ex-2layouts.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file (pixbuf_filename, NULL);
        g_free (pixbuf_filename);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dialog->priv->nlayouts_image2), pixbuf);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct layout page.                                         */
/*--------------------------------------------------------------------------*/
static void
construct_layout_page (glTemplateDesigner      *dialog,
		       GdkPixbuf               *logo)
{
	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "layout_page",           &dialog->priv->layout_page,
                                     "layout1_head_label",    &dialog->priv->layout1_head_label,
                                     "layout1_nx_spin",       &dialog->priv->layout1_nx_spin,
                                     "layout1_ny_spin",       &dialog->priv->layout1_ny_spin,
                                     "layout1_x0_spin",       &dialog->priv->layout1_x0_spin,
                                     "layout1_y0_spin",       &dialog->priv->layout1_y0_spin,
                                     "layout1_dx_spin",       &dialog->priv->layout1_dx_spin,
                                     "layout1_dy_spin",       &dialog->priv->layout1_dy_spin,
                                     "layout2_head_label",    &dialog->priv->layout2_head_label,
                                     "layout2_nx_spin",       &dialog->priv->layout2_nx_spin,
                                     "layout2_ny_spin",       &dialog->priv->layout2_ny_spin,
                                     "layout2_x0_spin",       &dialog->priv->layout2_x0_spin,
                                     "layout2_y0_spin",       &dialog->priv->layout2_y0_spin,
                                     "layout2_dx_spin",       &dialog->priv->layout2_dx_spin,
                                     "layout2_dy_spin",       &dialog->priv->layout2_dy_spin,
                                     "layout_x0_units_label", &dialog->priv->layout_x0_units_label,
                                     "layout_y0_units_label", &dialog->priv->layout_y0_units_label,
                                     "layout_dx_units_label", &dialog->priv->layout_dx_units_label,
                                     "layout_dy_units_label", &dialog->priv->layout_dy_units_label,
                                     "mini_preview_vbox",     &dialog->priv->mini_preview_vbox,
                                     "layout_test_button",    &dialog->priv->layout_test_button,
                                     NULL);

        dialog->priv->layout_mini_preview = gl_mini_preview_new (175, 200);
        gtk_container_add (GTK_CONTAINER (dialog->priv->mini_preview_vbox),
                           dialog->priv->layout_mini_preview);

        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->layout_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->layout_page,
                                      _("Layout(s)"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->layout_page,
                                             logo);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->layout_page,
                                         TRUE);

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->layout1_x0_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->layout1_x0_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->layout2_x0_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->layout2_x0_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->layout_x0_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->layout1_y0_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->layout1_y0_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->layout2_y0_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->layout2_y0_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->layout_y0_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->layout1_dx_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->layout1_dx_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->layout2_dx_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->layout2_dx_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->layout_dx_units_label),
			    dialog->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->layout1_dy_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->layout1_dy_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dialog->priv->layout2_dy_spin),
				    dialog->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dialog->priv->layout2_dy_spin),
                                        dialog->priv->climb_rate, 10.0*dialog->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dialog->priv->layout_dy_units_label),
			    dialog->priv->units_string);

	/* Connect a handler that listens for changes in these widgets */
	/* This controls sensitivity of related widgets. */
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout1_nx_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout1_ny_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout1_x0_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout1_y0_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout1_dx_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout1_dy_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout2_nx_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout2_ny_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout2_x0_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout2_y0_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout2_dx_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout2_dy_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dialog);


	/* Print button */
	g_signal_connect_swapped (G_OBJECT(dialog->priv->layout_test_button), "clicked",
				  G_CALLBACK(print_test_cb), dialog);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct finish page.                                         */
/*--------------------------------------------------------------------------*/
static void
construct_finish_page (glTemplateDesigner      *dialog,
		       GdkPixbuf               *logo)
{
	gl_debug (DEBUG_TEMPLATE, "START");

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "finish_page", &dialog->priv->finish_page,
                                     NULL);


        gtk_assistant_append_page (GTK_ASSISTANT (dialog),
                                   dialog->priv->finish_page);

        gtk_assistant_set_page_title (GTK_ASSISTANT (dialog),
                                      dialog->priv->finish_page,
                                      _("Design Completed"));
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (dialog),
                                             dialog->priv->finish_page,
                                             logo);
        gtk_assistant_set_page_type (GTK_ASSISTANT (dialog),
                                     dialog->priv->finish_page,
                                     GTK_ASSISTANT_PAGE_CONFIRM);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                         dialog->priv->finish_page,
                                         TRUE);

	gl_debug (DEBUG_TEMPLATE, "END");
}


/*****************************************************************************/
/* Initialize dialog from existing template.                                 */
/*****************************************************************************/
void
gl_template_designer_set_from_name (glTemplateDesigner *dialog,
                                    const gchar        *name)
{
        lglTemplate      *template;
        gdouble           upp;
        lglTemplateFrame *frame;
        gchar            *paper_name;
        GList            *p, *p_layout1, *p_layout2;

	gtk_window_set_title (GTK_WINDOW(dialog), _("Edit gLabels Template"));

        template = lgl_db_lookup_template_from_name (name);
        if ( lgl_template_does_category_match (template, "user-defined") )
        {

                dialog->priv->stop_signals = TRUE;

                dialog->priv->edit_flag  = TRUE;
                dialog->priv->edit_brand = g_strdup (template->brand);
                dialog->priv->edit_part  = g_strdup (template->part);

                upp = dialog->priv->units_per_point;

                gtk_entry_set_text (GTK_ENTRY (dialog->priv->brand_entry),       template->brand);
                gtk_entry_set_text (GTK_ENTRY (dialog->priv->part_num_entry),    template->part);
                gtk_entry_set_text (GTK_ENTRY (dialog->priv->description_entry), template->description);


                paper_name = lgl_db_lookup_paper_name_from_id (template->paper_id);
                gl_combo_util_set_active_text (GTK_COMBO_BOX (dialog->priv->pg_size_combo), paper_name);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin), template->page_width*upp);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin), template->page_height*upp);
                g_free (paper_name);


                frame = (lglTemplateFrame *)template->frames->data;
                switch (frame->shape)
                {
                case LGL_TEMPLATE_FRAME_SHAPE_RECT:
                default:
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->shape_rect_radio), TRUE);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin),       frame->rect.w*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin),       frame->rect.h*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_r_spin),       frame->rect.r*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin), frame->rect.x_waste*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin), frame->rect.y_waste*upp);
                        break;

                case LGL_TEMPLATE_FRAME_SHAPE_ELLIPSE:
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->shape_ellipse_radio), TRUE);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_w_spin),     frame->rect.w*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_h_spin),     frame->rect.h*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_waste_spin), frame->rect.y_waste*upp);
                        break;

                case LGL_TEMPLATE_FRAME_SHAPE_ROUND:
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->shape_round_radio), TRUE);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_r_spin),      frame->round.r*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin),  frame->round.waste*upp);
                        break;

                case LGL_TEMPLATE_FRAME_SHAPE_CD:
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->shape_cd_radio), TRUE);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_radius_spin),    frame->cd.r1*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_hole_spin),      frame->cd.r2*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_w_spin),         frame->cd.w*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_h_spin),         frame->cd.h*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin),  frame->round.waste*upp);
                        break;

                }
                for ( p = frame->all.markups; p != NULL; p=p->next )
                {
                        lglTemplateMarkup *markup = (lglTemplateMarkup *)p->data;

                        if ( markup->type == LGL_TEMPLATE_MARKUP_MARGIN )
                        {
                                gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_margin_spin),  markup->margin.size*upp);
                                gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_margin_spin),  markup->margin.size*upp);
                                gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_margin_spin), markup->margin.size*upp);
                                gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_margin_spin),    markup->margin.size*upp);
                        }
                }


                p_layout1 = g_list_nth (frame->all.layouts, 0);
                p_layout2 = g_list_nth (frame->all.layouts, 1);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->nlayouts_spin), p_layout2 ? 2 : 1 );
                if ( p_layout1 )
                {
                        lglTemplateLayout *layout1 = (lglTemplateLayout *)p_layout1->data;

                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_nx_spin), layout1->nx);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_ny_spin), layout1->ny);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_x0_spin), layout1->x0*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_y0_spin), layout1->y0*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_dx_spin), layout1->dx*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_dy_spin), layout1->dy*upp);
                }
                if ( p_layout2 )
                {
                        lglTemplateLayout *layout2 = (lglTemplateLayout *)p_layout2->data;

                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_nx_spin), layout2->nx);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_ny_spin), layout2->ny);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_x0_spin), layout2->x0*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_y0_spin), layout2->y0*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_dx_spin), layout2->dx*upp);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_dy_spin), layout2->dy*upp);
                }


                dialog->priv->stop_signals = FALSE;

        }
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  cancel callback.                                               */
/*--------------------------------------------------------------------------*/
static void
cancel_cb (glTemplateDesigner *dialog)
{
                                                                               
	gtk_widget_destroy (GTK_WIDGET(dialog));

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  apply callback                                                 */
/*--------------------------------------------------------------------------*/
static void
apply_cb (glTemplateDesigner *dialog)
{
        lglUnits     units;
	lglTemplate *template;
        gchar       *name;

        units = gl_prefs_model_get_units (gl_prefs);
        lgl_xml_set_default_units (units);

        if (dialog->priv->edit_flag)
        {
                lgl_db_delete_template_by_brand_part (dialog->priv->edit_brand, dialog->priv->edit_part);

                name = g_strdup_printf ("%s %s", dialog->priv->edit_brand, dialog->priv->edit_part);
                gl_mini_preview_pixbuf_cache_delete_by_name (name);
                g_free (name);
        }
	
	template = build_template (dialog);
	lgl_db_register_template (template);
        name = lgl_template_get_name (template);
        gl_mini_preview_pixbuf_cache_add_by_name (name);
        g_free (name);
}

                         
/*--------------------------------------------------------------------------*/
/* PRIVATE.  close callback                                                 */
/*--------------------------------------------------------------------------*/
static void
close_cb (glTemplateDesigner *dialog)
{
                                                                               
	gtk_widget_destroy (GTK_WIDGET(dialog));

}

                         
/*--------------------------------------------------------------------------*/
/* PRIVATE.  prepare page callback                                          */
/*--------------------------------------------------------------------------*/
static void
prepare_cb (glTemplateDesigner      *dialog,
            GtkWidget               *page)
{
        gint page_num;

        page_num = gtk_assistant_get_current_page (GTK_ASSISTANT (dialog));

        switch (page_num)
        {

        case NAME_PAGE_NUM:
                name_page_changed_cb (dialog); /* Use to prepare */
                break;

        case PG_SIZE_PAGE_NUM:
                pg_size_page_changed_cb (dialog); /* Use to prepare */
                break;

        case RECT_SIZE_PAGE_NUM:
                rect_size_page_prepare_cb (dialog);
                break;

        case ELLIPSE_SIZE_PAGE_NUM:
                ellipse_size_page_prepare_cb (dialog);
                break;

        case ROUND_SIZE_PAGE_NUM:
                round_size_page_prepare_cb (dialog);
                break;

        case CD_SIZE_PAGE_NUM:
                cd_size_page_prepare_cb (dialog);
                break;

        case LAYOUT_PAGE_NUM:
                layout_page_prepare_cb (dialog);
                break;

        default:
                /* No preparation needed */
                break;

        }
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Handle non-linear forward traversal.                           */
/*--------------------------------------------------------------------------*/
static gint
forward_page_function (gint     current_page,
                       gpointer data)
{
        glTemplateDesigner *dialog = GL_TEMPLATE_DESIGNER (data);

        switch (current_page)
        {
        case START_PAGE_NUM:
                return NAME_PAGE_NUM;

        case NAME_PAGE_NUM:
                return PG_SIZE_PAGE_NUM;

        case PG_SIZE_PAGE_NUM:
                return SHAPE_PAGE_NUM;

        case SHAPE_PAGE_NUM:
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_rect_radio))) {
                        return RECT_SIZE_PAGE_NUM;
		}
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_ellipse_radio))) {
                        return ELLIPSE_SIZE_PAGE_NUM;
		}
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_round_radio))) {
                        return ROUND_SIZE_PAGE_NUM;
		}
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_cd_radio))) {
                        return CD_SIZE_PAGE_NUM;
		}
                break;

        case RECT_SIZE_PAGE_NUM:
        case ELLIPSE_SIZE_PAGE_NUM:
        case ROUND_SIZE_PAGE_NUM:
        case CD_SIZE_PAGE_NUM:
                return NLAYOUTS_PAGE_NUM;

        case NLAYOUTS_PAGE_NUM:
                return LAYOUT_PAGE_NUM;

        case LAYOUT_PAGE_NUM:
                return FINISH_PAGE_NUM;

        case FINISH_PAGE_NUM:
        default:
                return -1;
        }

        return -1;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Widget on name page "changed" callback.                        */
/*--------------------------------------------------------------------------*/
static void
name_page_changed_cb (glTemplateDesigner *dialog)
{
	gchar *brand, *part_num, *desc;
        gchar *string;

	brand    = g_strstrip (gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->brand_entry), 0, -1));
	part_num = g_strstrip (gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->part_num_entry), 0, -1));
	desc     = gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->description_entry), 0, -1);

        gtk_image_clear (GTK_IMAGE (dialog->priv->name_warning_image));
        gtk_label_set_text (GTK_LABEL (dialog->priv->name_warning_label), "");

	if (brand && brand[0] && part_num && part_num[0])
        {

                if (lgl_db_does_template_exist (brand, part_num))
                {
                        /* Make exception for currently edited template. */
                        if ( dialog->priv->edit_brand && !lgl_str_utf8_casecmp (brand, dialog->priv->edit_brand) &&
                             dialog->priv->edit_part  && !lgl_str_utf8_casecmp (part_num, dialog->priv->edit_part) )
                        {
                                gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog), dialog->priv->name_page, TRUE);
                        }
                        else
                        {
                                gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog), dialog->priv->name_page, FALSE);

                                gtk_image_set_from_stock (GTK_IMAGE (dialog->priv->name_warning_image),
                                                          GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_BUTTON);
                                string = g_strdup_printf ("<span foreground='red' weight='bold'>%s</span>",
                                                          _("Brand and part# match an existing template!"));
                                gtk_label_set_markup (GTK_LABEL (dialog->priv->name_warning_label), string);
                                g_free (string);

                        }
                }
                else
                {
                        gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog), dialog->priv->name_page, TRUE);
                }

        }
        else
        {
                gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog), dialog->priv->name_page, FALSE);
        }

	g_free (brand);
	g_free (part_num);
	g_free (desc);
}
                                                                               

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Widget on page size page "changed" callback.                   */
/*--------------------------------------------------------------------------*/
static void
pg_size_page_changed_cb (glTemplateDesigner *dialog)
{
	gchar    *page_size_name;
	lglPaper *paper;
	

	page_size_name =
		gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (dialog->priv->pg_size_combo));

	if (page_size_name && strlen(page_size_name)) {

		paper = lgl_db_lookup_paper_from_name (page_size_name);
	

		if ( g_ascii_strcasecmp (paper->id, "Other") == 0 ) {
			gtk_widget_set_sensitive (GTK_WIDGET(dialog->priv->pg_w_spin), TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET(dialog->priv->pg_h_spin), TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET(dialog->priv->pg_w_units_label),
						  TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET(dialog->priv->pg_h_units_label),
						  TRUE);

		} else {
			gtk_widget_set_sensitive (GTK_WIDGET(dialog->priv->pg_w_spin), FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET(dialog->priv->pg_h_spin), FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET(dialog->priv->pg_w_units_label),
						  FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET(dialog->priv->pg_h_units_label),
						  FALSE);

			gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin),
						   paper->width * dialog->priv->units_per_point);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin),
						   paper->height * dialog->priv->units_per_point);
		}

		lgl_paper_free (paper);
	}

	g_free (page_size_name);

}
                                                                               

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare rectangular size page.                                 */
/*--------------------------------------------------------------------------*/
static void
rect_size_page_prepare_cb (glTemplateDesigner *dialog)
{
	gdouble max_w, max_h;
	gdouble w, h, r, x_waste, y_waste, margin;

	/* Limit label size based on already chosen page size. */
	max_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin));
	max_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin));

	w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin));
	h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin));
	r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_r_spin));
	x_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin));
	y_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin));
	margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_margin_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->rect_w_spin),
                                   dialog->priv->climb_rate, max_w);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->rect_h_spin),
                                   dialog->priv->climb_rate, max_h);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->rect_r_spin),
                                   0.0, MIN(max_w, max_h)/2.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->rect_x_waste_spin),
                                   0.0, MIN(max_w, max_h)/4.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->rect_y_waste_spin),
                                   0.0, MIN(max_w, max_h)/4.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->rect_margin_spin),
                                   0.0, MIN(max_w, max_h)/4.0);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin), w);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin), h);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_r_spin), r);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin), x_waste);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin), y_waste);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->rect_margin_spin), margin);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare ellipse size page.                                     */
/*--------------------------------------------------------------------------*/
static void
ellipse_size_page_prepare_cb (glTemplateDesigner *dialog)
{
	gdouble max_w, max_h;
	gdouble w, h, waste, margin;

	/* Limit label size based on already chosen page size. */
	max_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin));
	max_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin));

	w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_w_spin));
	h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_h_spin));
	waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_waste_spin));
	margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_margin_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->ellipse_w_spin),
                                   dialog->priv->climb_rate, max_w);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->ellipse_h_spin),
                                   dialog->priv->climb_rate, max_h);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->ellipse_waste_spin),
                                   0.0, MIN(max_w, max_h)/4.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->ellipse_margin_spin),
                                   0.0, MIN(max_w, max_h)/4.0);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_w_spin), w);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_h_spin), h);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_waste_spin), waste);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_margin_spin), margin);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare round size page.                                       */
/*--------------------------------------------------------------------------*/
static void
round_size_page_prepare_cb (glTemplateDesigner *dialog)
{
	gdouble max_w, max_h;
	gdouble r, waste, margin;

	/* Limit label size based on already chosen page size. */
	max_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin));
	max_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin));

	r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_r_spin));
	waste  = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin));
	margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_margin_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->round_r_spin),
                                   dialog->priv->climb_rate, MIN(max_w, max_h)/2.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->round_waste_spin),
                                   0.0, MIN(max_w, max_h)/4.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->round_margin_spin),
                                   0.0, MIN(max_w, max_h)/4.0);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_r_spin), r);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin), waste);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->round_margin_spin), margin);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare cd/dvd size page.                                      */
/*--------------------------------------------------------------------------*/
static void
cd_size_page_prepare_cb (glTemplateDesigner *dialog)
{
	gdouble max_w, max_h;
	gdouble radius, hole, w, h, waste, margin;

	/* Limit label size based on already chosen page size. */
	max_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin));
	max_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin));

	radius = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_radius_spin));
	hole = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_hole_spin));
	w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_w_spin));
	h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_h_spin));
	waste  = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_waste_spin));
	margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_margin_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->cd_radius_spin),
                                   dialog->priv->climb_rate, MIN(max_w, max_h)/2.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->cd_hole_spin),
                                   dialog->priv->climb_rate, MIN(max_w, max_h)/2.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->cd_w_spin),
                                   0.0, max_w);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->cd_h_spin),
                                   0.0, max_h);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->cd_waste_spin),
                                   0.0, MIN(max_w, max_h)/4.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->cd_margin_spin),
                                   0.0, MIN(max_w, max_h)/4.0);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_radius_spin), radius);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_hole_spin), hole);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_w_spin), w);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_h_spin), h);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_waste_spin), waste);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->cd_margin_spin), margin);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare Layout page cb.                                        */
/*--------------------------------------------------------------------------*/
static void
layout_page_prepare_cb (glTemplateDesigner *dialog)
{
	gdouble page_w, page_h;
	gdouble w=0, h=0, x_waste=0, y_waste=0;
	gint    max_nx, max_ny;
	gint    nlayouts;
	gdouble nx_1, ny_1, x0_1, y0_1, dx_1, dy_1;
	gdouble nx_2, ny_2, x0_2, y0_2, dx_2, dy_2;
	lglTemplate *template;

	if (dialog->priv->stop_signals) return;
	dialog->priv->stop_signals = TRUE;


	/* Limit ranges based on already chosen page and label sizes. */
	page_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin));
	page_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_rect_radio))) {
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin));
		x_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin));
		y_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin));
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_ellipse_radio))) {
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_h_spin));
		y_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_waste_spin));
		y_waste = x_waste;
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_round_radio))) {
		w = 2*gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_r_spin));
		h = w;
		x_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin));
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_cd_radio))) {
		w = 2*gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_radius_spin));
		h = w;
		x_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_waste_spin));
		y_waste = x_waste;
	}
	max_nx = MAX (floor (page_w/(w + 2*x_waste) + DELTA), 1.0);
	max_ny = MAX (floor (page_h/(h + 2*y_waste) + DELTA), 1.0);

	nx_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_nx_spin));
	ny_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_ny_spin));
	x0_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_x0_spin));
	y0_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_y0_spin));
	dx_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_dx_spin));
	dy_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_dy_spin));
	nx_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_nx_spin));
	ny_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_ny_spin));
	x0_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_x0_spin));
	y0_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_y0_spin));
	dx_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_dx_spin));
	dy_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_dy_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout1_nx_spin),
                                   1, max_nx);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout1_ny_spin),
                                   1, max_ny);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout1_x0_spin),
                                   x_waste, page_w - w - x_waste);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout1_y0_spin),
                                   y_waste, page_h - h - y_waste);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout1_dx_spin),
                                   w + 2*x_waste, page_w - w - 2*x_waste);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout1_dy_spin),
                                   h + 2*y_waste, page_h - h - 2*y_waste);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout2_nx_spin),
                                   1, max_nx);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout2_ny_spin),
                                   1, max_ny);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout2_x0_spin),
                                   x_waste, page_w - w - x_waste);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout2_y0_spin),
                                   y_waste, page_h - h - y_waste);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout2_dx_spin),
                                   w + 2*x_waste, page_w - w - 2*x_waste);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dialog->priv->layout2_dy_spin),
                                   h + 2*y_waste, page_h - h - 2*y_waste);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_nx_spin), nx_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_ny_spin), ny_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_x0_spin), x0_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_y0_spin), y0_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_dx_spin), dx_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout1_dy_spin), dy_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_nx_spin), nx_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_ny_spin), ny_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_x0_spin), x0_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_y0_spin), y0_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_dx_spin), dx_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->priv->layout2_dy_spin), dy_2);

	/* Set visibility of layout2 widgets as appropriate. */
	nlayouts = gtk_spin_button_get_value (GTK_SPIN_BUTTON (dialog->priv->nlayouts_spin));
	if ( nlayouts == 1 )
        {
		gtk_widget_hide (dialog->priv->layout1_head_label);
		gtk_widget_hide (dialog->priv->layout2_head_label);
		gtk_widget_hide (dialog->priv->layout2_nx_spin);
		gtk_widget_hide (dialog->priv->layout2_ny_spin);
		gtk_widget_hide (dialog->priv->layout2_x0_spin);
		gtk_widget_hide (dialog->priv->layout2_y0_spin);
		gtk_widget_hide (dialog->priv->layout2_dx_spin);
		gtk_widget_hide (dialog->priv->layout2_dy_spin);
	}
        else
        {
		gtk_widget_show (dialog->priv->layout1_head_label);
		gtk_widget_show (dialog->priv->layout2_head_label);
		gtk_widget_show (dialog->priv->layout2_nx_spin);
		gtk_widget_show (dialog->priv->layout2_ny_spin);
		gtk_widget_show (dialog->priv->layout2_x0_spin);
		gtk_widget_show (dialog->priv->layout2_y0_spin);
		gtk_widget_show (dialog->priv->layout2_dx_spin);
		gtk_widget_show (dialog->priv->layout2_dy_spin);
	}

	template = build_template (dialog);
	gl_mini_preview_set_template (GL_MINI_PREVIEW(dialog->priv->layout_mini_preview),
                                      template);
	lgl_template_free (template);


	dialog->priv->stop_signals = FALSE;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Layout page widget changed cb.                                 */
/*--------------------------------------------------------------------------*/
static void
layout_page_changed_cb (glTemplateDesigner *dialog)
{
	lglTemplate *template;

	if (dialog->priv->stop_signals) return;
	dialog->priv->stop_signals = TRUE;

	template = build_template (dialog);

	gl_mini_preview_set_template (GL_MINI_PREVIEW(dialog->priv->layout_mini_preview),
                                      template);

	lgl_template_free (template);

	dialog->priv->stop_signals = FALSE;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Print test sheet callback.                                     */
/*--------------------------------------------------------------------------*/
static void
print_test_cb (glTemplateDesigner      *dialog)
{
	GObject           *label;
	lglTemplate       *template;
	glPrintOpDialog   *print_op;

	label = gl_label_new ();

	template = build_template (dialog);
	gl_label_set_template (GL_LABEL(label), template, FALSE);

	print_op = gl_print_op_dialog_new (GL_LABEL(label));
	gl_print_op_force_outline (GL_PRINT_OP (print_op));
        gtk_print_operation_run (GTK_PRINT_OPERATION (print_op),
                                 GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                 GTK_WINDOW (dialog),
                                 NULL);

	lgl_template_free (template);
	g_object_unref (G_OBJECT(label));
}


/*--------------------------------------------------------------------------*/
/* Build a template based on current assistant settings.                    */
/*--------------------------------------------------------------------------*/
static lglTemplate *
build_template (glTemplateDesigner      *dialog)
{
	gdouble               upp;
	gchar                *brand, *part_num, *desc;
	gchar                *page_size_name;
	lglPaper             *paper;
	lglTemplateFrameShape shape;
	lglTemplateFrame     *frame=NULL;
	gdouble               w=0, h=0, r=0, radius=0, hole=0, waste=0, x_waste=0, y_waste=0, margin=0;
	gint                  nlayouts;
	gdouble               nx_1, ny_1, x0_1, y0_1, dx_1, dy_1;
	gdouble               nx_2, ny_2, x0_2, y0_2, dx_2, dy_2;
	lglTemplate          *template;

	upp = dialog->priv->units_per_point;

	brand    = g_strstrip (gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->brand_entry), 0, -1));
	part_num = g_strstrip (gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->part_num_entry), 0, -1));
	desc     = gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->description_entry), 0, -1);

	page_size_name =
		gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (dialog->priv->pg_size_combo));
	paper = lgl_db_lookup_paper_from_name (page_size_name);
	if ( g_ascii_strcasecmp (paper->id, "Other") == 0 ) {
		paper->width =
			gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin))
			/ upp;
		paper->height =
			gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin))
			 / upp;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_rect_radio))) {
		shape = LGL_TEMPLATE_FRAME_SHAPE_RECT;
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin));
		r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_r_spin));
		x_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin));
		y_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin));
		margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_margin_spin));
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_ellipse_radio))) {
		shape = LGL_TEMPLATE_FRAME_SHAPE_ELLIPSE;
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_h_spin));
		waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_waste_spin));
		margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->ellipse_margin_spin));
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_round_radio))) {
		shape = LGL_TEMPLATE_FRAME_SHAPE_ROUND;
		r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_r_spin));
		waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin));
		margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_margin_spin));
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_cd_radio))) {
		shape = LGL_TEMPLATE_FRAME_SHAPE_CD;
		radius = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_radius_spin));
		hole = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_hole_spin));
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_h_spin));
		waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_waste_spin));
		margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->cd_margin_spin));
	}

	nlayouts = gtk_spin_button_get_value (GTK_SPIN_BUTTON (dialog->priv->nlayouts_spin));
	nx_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_nx_spin));
	ny_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_ny_spin));
	x0_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_x0_spin));
	y0_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_y0_spin));
	dx_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_dx_spin));
	dy_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout1_dy_spin));
	nx_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_nx_spin));
	ny_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_ny_spin));
	x0_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_x0_spin));
	y0_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_y0_spin));
	dx_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_dx_spin));
	dy_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->layout2_dy_spin));


	template = lgl_template_new (brand, part_num, desc, paper->id, paper->width, paper->height);

	switch (shape) {
	case LGL_TEMPLATE_FRAME_SHAPE_RECT:
		frame = lgl_template_frame_rect_new ("0", w/upp, h/upp, r/upp, x_waste/upp, y_waste/upp);
		break;
	case LGL_TEMPLATE_FRAME_SHAPE_ELLIPSE:
		frame = lgl_template_frame_ellipse_new ("0", w/upp, h/upp, waste/upp);
		break;
	case LGL_TEMPLATE_FRAME_SHAPE_ROUND:
		frame = lgl_template_frame_round_new ("0", r/upp, waste/upp);
		break;
	case LGL_TEMPLATE_FRAME_SHAPE_CD:
		frame = lgl_template_frame_cd_new ("0", radius/upp, hole/upp, w/upp, h/upp, waste/upp);
		break;
        default:
                g_assert_not_reached ();
		break;
	}
	lgl_template_add_frame (template, frame);

	lgl_template_frame_add_markup (frame,
                                       lgl_template_markup_margin_new (margin/upp));

	lgl_template_frame_add_layout (frame,
                                       lgl_template_layout_new (nx_1, ny_1,
                                                                x0_1/upp,
                                                                y0_1/upp,
                                                                dx_1/upp,
                                                                dy_1/upp));
	if (nlayouts > 1) {
		lgl_template_frame_add_layout (frame,
                                               lgl_template_layout_new (nx_2, ny_2,
                                                                        x0_2/upp,
                                                                        y0_2/upp,
                                                                        dx_2/upp,
                                                                        dy_2/upp));
	}

	g_free (brand);
	g_free (part_num);
	g_free (desc);

	g_free (page_size_name);
	lgl_paper_free (paper);

	return template;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
