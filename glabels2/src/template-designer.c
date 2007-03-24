/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  template-designer.c:  Template designer module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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

#include "template-designer.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include <string.h>
#include <math.h>

#include "prefs.h"
#include <libglabels/paper.h>
#include <libglabels/template.h>
#include "wdgt-mini-preview.h"
#include "print-op.h"
#include "util.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define ICON_PIXMAP        (GLABELS_ICON_DIR "glabels.png")

#define EX_RECT_IMAGE      (GLABELS_PIXMAP_DIR "ex-rect-size.png")
#define EX_ROUND_IMAGE     (GLABELS_PIXMAP_DIR "ex-round-size.png")
#define EX_CD_IMAGE        (GLABELS_PIXMAP_DIR "ex-cd-size.png")
#define EX_NLAYOUTS_IMAGE1 (GLABELS_PIXMAP_DIR "ex-1layout.png")
#define EX_NLAYOUTS_IMAGE2 (GLABELS_PIXMAP_DIR "ex-2layouts.png")

#define DEFAULT_MARGIN 9.0

#define DEFAULT_RECT_W      252.0
#define DEFAULT_RECT_H      144.0
#define DEFAULT_RECT_R        0.0
#define DEFAULT_RECT_WASTE    0.0 /* Should never exceed 1/2 the distance between items. */

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
	/* Assistant pages */
	GtkWidget       *start_page;
	GtkWidget       *name_page;
	GtkWidget       *pg_size_page;
	GtkWidget       *shape_page;
	GtkWidget       *rect_size_page;
	GtkWidget       *round_size_page;
	GtkWidget       *cd_size_page;
	GtkWidget       *nlayouts_page;
	GtkWidget       *layout_page;
	GtkWidget       *finish_page;

	/* Name page controls */
	GtkWidget       *brand_entry;
	GtkWidget       *part_num_entry;
	GtkWidget       *description_entry;

	/* Page size page controls */
	GtkWidget       *pg_size_combo;
	GtkWidget       *pg_w_spin;
	GtkWidget       *pg_h_spin;
	GtkWidget       *pg_w_units_label;
	GtkWidget       *pg_h_units_label;

	/* Shape page controls */
	GtkWidget       *shape_rect_radio;
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
	GtkWidget       *layout_mini_preview;
	GtkWidget       *layout_test_button;

	/* Units related stuff */
	const gchar     *units_string;
	gdouble          units_per_point;
	gdouble          climb_rate;
	gint             digits;

};

/* Page numbers for traversing GtkAssistant */
enum {
        START_PAGE_NUM = 0,
        NAME_PAGE_NUM,
        PG_SIZE_PAGE_NUM,
        SHAPE_PAGE_NUM,
        RECT_SIZE_PAGE_NUM,
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

static void     round_size_page_prepare_cb        (glTemplateDesigner      *dialog);

static void     cd_size_page_prepare_cb           (glTemplateDesigner      *dialog);

static void     layout_page_prepare_cb            (glTemplateDesigner      *dialog);

static void     layout_page_changed_cb            (glTemplateDesigner      *dialog);

static void     print_test_cb                     (glTemplateDesigner      *dialog);

static glTemplate *build_template                 (glTemplateDesigner      *dialog);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glTemplateDesigner, gl_template_designer, GTK_TYPE_ASSISTANT);

static void
gl_template_designer_class_init (glTemplateDesignerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_TEMPLATE, "");
	
  	gl_template_designer_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_template_designer_finalize;  	
}

static void
gl_template_designer_init (glTemplateDesigner *dialog)
{
	gl_debug (DEBUG_TEMPLATE, "START");

	dialog->priv = g_new0 (glTemplateDesignerPrivate, 1);

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
	GdkPixbuf  *logo;

	g_return_if_fail (dialog && GL_IS_TEMPLATE_DESIGNER (dialog));
	g_return_if_fail (dialog->priv != NULL);

	/* Initialize units stuff from prefs */
	dialog->priv->units_string    = gl_prefs_get_units_string ();
        dialog->priv->units_per_point = gl_prefs_get_units_per_point ();
        dialog->priv->climb_rate      = gl_prefs_get_units_step_size ();
        dialog->priv->digits          = gl_prefs_get_units_precision ();

	gtk_window_set_title (GTK_WINDOW(dialog), _("gLabels Template Designer"));

	logo = gdk_pixbuf_new_from_file (ICON_PIXMAP, NULL);

        /* Costruct and append pages (must be same order as PAGE_NUM enums. */
	construct_start_page (dialog, logo);
	construct_name_page (dialog, logo);
	construct_pg_size_page (dialog, logo);
	construct_shape_page (dialog, logo);
	construct_rect_size_page (dialog, logo);
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
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct start page.                                          */
/*--------------------------------------------------------------------------*/
static void
construct_start_page (glTemplateDesigner      *dialog,
		      GdkPixbuf               *logo)
{
	GladeXML        *gui;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "start_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->start_page = glade_xml_get_widget (gui, "start_page");

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
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct name page.                                           */
/*--------------------------------------------------------------------------*/
static void
construct_name_page (glTemplateDesigner      *dialog,
		     GdkPixbuf               *logo)
{
	GladeXML        *gui;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "name_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->name_page         = glade_xml_get_widget (gui, "name_page");
	dialog->priv->brand_entry       = glade_xml_get_widget (gui, "brand_entry");
	dialog->priv->part_num_entry    = glade_xml_get_widget (gui, "part_num_entry");
	dialog->priv->description_entry = glade_xml_get_widget (gui, "description_entry");

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

}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Construct page size page.                                       */
/*--------------------------------------------------------------------------*/
static void
construct_pg_size_page (glTemplateDesigner      *dialog,
			GdkPixbuf               *logo)
{
	GladeXML        *gui;
	GList           *page_sizes;
	const gchar     *default_page_size_id;
	gchar           *default_page_size_name;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "pg_size_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->pg_size_page     = glade_xml_get_widget (gui, "pg_size_page");
	dialog->priv->pg_size_combo    = glade_xml_get_widget (gui, "pg_size_combo");
	dialog->priv->pg_w_spin        = glade_xml_get_widget (gui, "pg_w_spin");
	dialog->priv->pg_h_spin        = glade_xml_get_widget (gui, "pg_h_spin");
	dialog->priv->pg_w_units_label = glade_xml_get_widget (gui, "pg_w_units_label");
	dialog->priv->pg_h_units_label = glade_xml_get_widget (gui, "pg_h_units_label");

	gl_util_combo_box_add_text_model (GTK_COMBO_BOX (dialog->priv->pg_size_combo));

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
	page_sizes = gl_paper_get_name_list ();
	gl_util_combo_box_set_strings (GTK_COMBO_BOX (dialog->priv->pg_size_combo), page_sizes);
	gl_paper_free_name_list (page_sizes);
	default_page_size_id = gl_prefs_get_page_size ();
	default_page_size_name = gl_paper_lookup_name_from_id (default_page_size_id);
	gl_util_combo_box_set_active_text (GTK_COMBO_BOX (dialog->priv->pg_size_combo), default_page_size_name);
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
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct shape page.                                          */
/*--------------------------------------------------------------------------*/
static void
construct_shape_page (glTemplateDesigner      *dialog,
		      GdkPixbuf               *logo)
{
	GladeXML        *gui;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "shape_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->shape_page        = glade_xml_get_widget (gui, "shape_page");
	dialog->priv->shape_rect_radio  = glade_xml_get_widget (gui, "shape_rect_radio");
	dialog->priv->shape_round_radio = glade_xml_get_widget (gui, "shape_round_radio");
	dialog->priv->shape_cd_radio    = glade_xml_get_widget (gui, "shape_cd_radio");

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
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct rect size page.                                      */
/*--------------------------------------------------------------------------*/
static void
construct_rect_size_page (glTemplateDesigner      *dialog,
			  GdkPixbuf               *logo)
{
	GladeXML        *gui;
	GdkPixbuf       *pixbuf;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "rect_size_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->rect_size_page           = glade_xml_get_widget (gui, "rect_size_page");
	dialog->priv->rect_image               = glade_xml_get_widget (gui, "rect_image");
	dialog->priv->rect_w_spin              = glade_xml_get_widget (gui, "rect_w_spin");
	dialog->priv->rect_h_spin              = glade_xml_get_widget (gui, "rect_h_spin");
	dialog->priv->rect_r_spin              = glade_xml_get_widget (gui, "rect_r_spin");
	dialog->priv->rect_x_waste_spin        = glade_xml_get_widget (gui, "rect_x_waste_spin");
	dialog->priv->rect_y_waste_spin        = glade_xml_get_widget (gui, "rect_y_waste_spin");
	dialog->priv->rect_margin_spin         = glade_xml_get_widget (gui, "rect_margin_spin");
	dialog->priv->rect_w_units_label       = glade_xml_get_widget (gui, "rect_w_units_label");
	dialog->priv->rect_h_units_label       = glade_xml_get_widget (gui, "rect_h_units_label");
	dialog->priv->rect_r_units_label       = glade_xml_get_widget (gui, "rect_r_units_label");
	dialog->priv->rect_x_waste_units_label = glade_xml_get_widget (gui, "rect_x_waste_units_label");
	dialog->priv->rect_y_waste_units_label = glade_xml_get_widget (gui, "rect_y_waste_units_label");
	dialog->priv->rect_margin_units_label  = glade_xml_get_widget (gui, "rect_margin_units_label");

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
	pixbuf = gdk_pixbuf_new_from_file (EX_RECT_IMAGE, NULL);
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
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct round size page.                                     */
/*--------------------------------------------------------------------------*/
static void
construct_round_size_page (glTemplateDesigner      *dialog,
			   GdkPixbuf               *logo)
{
	GladeXML        *gui;
	GdkPixbuf       *pixbuf;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "round_size_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->round_size_page          = glade_xml_get_widget (gui, "round_size_page");
	dialog->priv->round_image              = glade_xml_get_widget (gui, "round_image");
	dialog->priv->round_r_spin             = glade_xml_get_widget (gui, "round_r_spin");
	dialog->priv->round_waste_spin         = glade_xml_get_widget (gui, "round_waste_spin");
	dialog->priv->round_margin_spin        = glade_xml_get_widget (gui, "round_margin_spin");
	dialog->priv->round_r_units_label      = glade_xml_get_widget (gui, "round_r_units_label");
	dialog->priv->round_waste_units_label  = glade_xml_get_widget (gui, "round_waste_units_label");
	dialog->priv->round_margin_units_label = glade_xml_get_widget (gui, "round_margin_units_label");

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
	pixbuf = gdk_pixbuf_new_from_file (EX_ROUND_IMAGE, NULL);
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

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct CD/DVD size page.                                    */
/*--------------------------------------------------------------------------*/
static void
construct_cd_size_page (glTemplateDesigner      *dialog,
			GdkPixbuf               *logo)
{
	GladeXML        *gui;
	GdkPixbuf       *pixbuf;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "cd_size_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->cd_size_page = glade_xml_get_widget (gui, "cd_size_page");
	dialog->priv->cd_image     = glade_xml_get_widget (gui, "cd_image");
	dialog->priv->cd_radius_spin = glade_xml_get_widget (gui, "cd_radius_spin");
	dialog->priv->cd_hole_spin   = glade_xml_get_widget (gui, "cd_hole_spin");
	dialog->priv->cd_w_spin      = glade_xml_get_widget (gui, "cd_w_spin");
	dialog->priv->cd_h_spin      = glade_xml_get_widget (gui, "cd_h_spin");
	dialog->priv->cd_waste_spin  = glade_xml_get_widget (gui, "cd_waste_spin");
	dialog->priv->cd_margin_spin = glade_xml_get_widget (gui, "cd_margin_spin");
	dialog->priv->cd_radius_units_label = glade_xml_get_widget (gui, "cd_radius_units_label");
	dialog->priv->cd_hole_units_label   = glade_xml_get_widget (gui, "cd_hole_units_label");
	dialog->priv->cd_w_units_label      = glade_xml_get_widget (gui, "cd_w_units_label");
	dialog->priv->cd_h_units_label      = glade_xml_get_widget (gui, "cd_h_units_label");
	dialog->priv->cd_waste_units_label  = glade_xml_get_widget (gui, "cd_waste_units_label");
	dialog->priv->cd_margin_units_label = glade_xml_get_widget (gui, "cd_margin_units_label");

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
	pixbuf = gdk_pixbuf_new_from_file (EX_CD_IMAGE, NULL);
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

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct number of layouts page.                              */
/*--------------------------------------------------------------------------*/
static void
construct_nlayouts_page (glTemplateDesigner      *dialog,
			 GdkPixbuf               *logo)
{
	GladeXML        *gui;
	GdkPixbuf       *pixbuf;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "nlayouts_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->nlayouts_page   = glade_xml_get_widget (gui, "nlayouts_page");
	dialog->priv->nlayouts_image1 = glade_xml_get_widget (gui, "nlayouts_image1");
	dialog->priv->nlayouts_image2 = glade_xml_get_widget (gui, "nlayouts_image2");
	dialog->priv->nlayouts_spin   = glade_xml_get_widget (gui, "nlayouts_spin");

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

	/* Initialize illustrations. */
	pixbuf = gdk_pixbuf_new_from_file (EX_NLAYOUTS_IMAGE1, NULL);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dialog->priv->nlayouts_image1), pixbuf);
	pixbuf = gdk_pixbuf_new_from_file (EX_NLAYOUTS_IMAGE2, NULL);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dialog->priv->nlayouts_image2), pixbuf);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct layout page.                                         */
/*--------------------------------------------------------------------------*/
static void
construct_layout_page (glTemplateDesigner      *dialog,
		       GdkPixbuf               *logo)
{
	GladeXML        *gui;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "layout_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->layout_page           = glade_xml_get_widget (gui, "layout_page");
	dialog->priv->layout1_head_label    = glade_xml_get_widget (gui, "layout1_head_label");
	dialog->priv->layout1_nx_spin       = glade_xml_get_widget (gui, "layout1_nx_spin");
	dialog->priv->layout1_ny_spin       = glade_xml_get_widget (gui, "layout1_ny_spin");
	dialog->priv->layout1_x0_spin       = glade_xml_get_widget (gui, "layout1_x0_spin");
	dialog->priv->layout1_y0_spin       = glade_xml_get_widget (gui, "layout1_y0_spin");
	dialog->priv->layout1_dx_spin       = glade_xml_get_widget (gui, "layout1_dx_spin");
	dialog->priv->layout1_dy_spin       = glade_xml_get_widget (gui, "layout1_dy_spin");
	dialog->priv->layout2_head_label    = glade_xml_get_widget (gui, "layout2_head_label");
	dialog->priv->layout2_nx_spin       = glade_xml_get_widget (gui, "layout2_nx_spin");
	dialog->priv->layout2_ny_spin       = glade_xml_get_widget (gui, "layout2_ny_spin");
	dialog->priv->layout2_x0_spin       = glade_xml_get_widget (gui, "layout2_x0_spin");
	dialog->priv->layout2_y0_spin       = glade_xml_get_widget (gui, "layout2_y0_spin");
	dialog->priv->layout2_dx_spin       = glade_xml_get_widget (gui, "layout2_dx_spin");
	dialog->priv->layout2_dy_spin       = glade_xml_get_widget (gui, "layout2_dy_spin");
	dialog->priv->layout_x0_units_label = glade_xml_get_widget (gui, "layout_x0_units_label");
	dialog->priv->layout_y0_units_label = glade_xml_get_widget (gui, "layout_y0_units_label");
	dialog->priv->layout_dx_units_label = glade_xml_get_widget (gui, "layout_dx_units_label");
	dialog->priv->layout_dy_units_label = glade_xml_get_widget (gui, "layout_dy_units_label");
	dialog->priv->layout_mini_preview   = glade_xml_get_widget (gui, "layout_mini_preview");
	dialog->priv->layout_test_button    = glade_xml_get_widget (gui, "layout_test_button");

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

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct finish page.                                         */
/*--------------------------------------------------------------------------*/
static void
construct_finish_page (glTemplateDesigner      *dialog,
		       GdkPixbuf               *logo)
{
	GladeXML        *gui;

	gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
                             "finish_page", NULL);
	if (!gui)
        {
                g_critical ("Could not open template-designer.glade. gLabels may not be installed correctly!");
                return;
        }

	dialog->priv->finish_page = glade_xml_get_widget (gui, "finish_page");

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
	glTemplate *template;
	
	template = build_template (dialog);
	gl_template_register (template);
                                                                               
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
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_round_radio))) {
                        return ROUND_SIZE_PAGE_NUM;
		}
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_cd_radio))) {
                        return CD_SIZE_PAGE_NUM;
		}
                break;

        case RECT_SIZE_PAGE_NUM:
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

	brand    = gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->brand_entry), 0, -1);
	part_num = gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->part_num_entry), 0, -1);
	desc     = gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->description_entry), 0, -1);

	if (brand && brand[0] && part_num && part_num[0] && desc && desc[0])
        {

                gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                                 dialog->priv->name_page,
                                                 TRUE);
	}
        else
        {

                gtk_assistant_set_page_complete (GTK_ASSISTANT (dialog),
                                                 dialog->priv->name_page,
                                                 FALSE);
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
	gchar   *page_size_name;
	glPaper *paper;
	

	page_size_name =
		gtk_combo_box_get_active_text (GTK_COMBO_BOX (dialog->priv->pg_size_combo));

	if (page_size_name && strlen(page_size_name)) {

		paper = gl_paper_from_name (page_size_name);
	

		if ( g_strcasecmp (paper->id, "Other") == 0 ) {
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

		gl_paper_free (paper);
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
/* PRIVATE.  Layout page widget changed cb.                                 */
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

	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout1_nx_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout1_ny_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout1_x0_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout1_y0_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout1_dx_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout1_dy_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout2_nx_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout2_ny_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout2_x0_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout2_y0_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout2_dx_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));
	g_signal_handlers_block_by_func (G_OBJECT(dialog->priv->layout2_dy_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dialog));

	/* Limit ranges based on already chosen page and label sizes. */
	page_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin));
	page_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_rect_radio))) {
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin));
		x_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin));
		y_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin));
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_round_radio))) {
		w = 2*gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_r_spin));
		h = w;
		x_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin));
		y_waste = x_waste;
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
	if ( nlayouts == 1 ) {

		gtk_widget_hide (dialog->priv->layout1_head_label);
		gtk_widget_hide (dialog->priv->layout2_head_label);
		gtk_widget_hide (dialog->priv->layout2_nx_spin);
		gtk_widget_hide (dialog->priv->layout2_ny_spin);
		gtk_widget_hide (dialog->priv->layout2_x0_spin);
		gtk_widget_hide (dialog->priv->layout2_y0_spin);
		gtk_widget_hide (dialog->priv->layout2_dx_spin);
		gtk_widget_hide (dialog->priv->layout2_dy_spin);

	} else {

		gtk_widget_show (dialog->priv->layout1_head_label);
		gtk_widget_show (dialog->priv->layout2_head_label);
		gtk_widget_show (dialog->priv->layout2_nx_spin);
		gtk_widget_show (dialog->priv->layout2_ny_spin);
		gtk_widget_show (dialog->priv->layout2_x0_spin);
		gtk_widget_show (dialog->priv->layout2_y0_spin);
		gtk_widget_show (dialog->priv->layout2_dx_spin);
		gtk_widget_show (dialog->priv->layout2_dy_spin);

	}

	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout1_nx_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout1_ny_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout1_x0_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout1_y0_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout1_dx_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout1_dy_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout2_nx_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout2_ny_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout2_x0_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout2_y0_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout2_dx_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));
	g_signal_handlers_unblock_by_func (G_OBJECT(dialog->priv->layout2_dy_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dialog));

	layout_page_changed_cb (dialog);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Layout page widget changed cb.                                 */
/*--------------------------------------------------------------------------*/
static void
layout_page_changed_cb (glTemplateDesigner *dialog)
{
	glTemplate *template;

	template = build_template (dialog);

	gl_wdgt_mini_preview_set_template (GL_WDGT_MINI_PREVIEW(dialog->priv->layout_mini_preview),
					   template);

	gl_template_free (template);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Print test sheet callback.                                     */
/*--------------------------------------------------------------------------*/
static void
print_test_cb (glTemplateDesigner      *dialog)
{
	GObject    *label;
	glTemplate *template;
	glPrintOp  *print_op;

	label = gl_label_new ();

	template = build_template (dialog);
	gl_label_set_template (GL_LABEL(label), template);

	print_op = gl_print_op_new (GL_LABEL(label));
	gl_print_op_force_outline_flag (print_op);
        gtk_print_operation_run (GTK_PRINT_OPERATION (print_op),
                                 GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                 GTK_WINDOW (dialog),
                                 NULL);

	gl_template_free (template);
	g_object_unref (G_OBJECT(label));
}

/*--------------------------------------------------------------------------*/
/* Build a template based on current assistant settings.                    */
/*--------------------------------------------------------------------------*/
static glTemplate *
build_template (glTemplateDesigner      *dialog)
{
	gdouble               upp;
	gchar                *brand, *part_num, *name, *desc;
	gchar                *page_size_name;
	glPaper              *paper;
	glTemplateLabelShape  shape;
	glTemplateLabelType  *label_type=NULL;
	gdouble               w=0, h=0, r=0, radius=0, hole=0, waste=0, x_waste=0, y_waste=0, margin=0;
	gint                  nlayouts;
	gdouble               nx_1, ny_1, x0_1, y0_1, dx_1, dy_1;
	gdouble               nx_2, ny_2, x0_2, y0_2, dx_2, dy_2;
	glTemplate           *template;

	upp = dialog->priv->units_per_point;

	brand    = gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->brand_entry), 0, -1);
	part_num = gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->part_num_entry), 0, -1);
	name     = g_strdup_printf ("%s %s", brand, part_num);
	desc     = gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->description_entry), 0, -1);

	page_size_name =
		gtk_combo_box_get_active_text (GTK_COMBO_BOX (dialog->priv->pg_size_combo));
	paper = gl_paper_from_name (page_size_name);
	if ( g_strcasecmp (paper->id, "Other") == 0 ) {
		paper->width =
			gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_w_spin))
			/ upp;
		paper->height =
			gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->pg_h_spin))
			 / upp;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_rect_radio))) {
		shape = GL_TEMPLATE_SHAPE_RECT;
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_h_spin));
		r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_r_spin));
		x_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_x_waste_spin));
		y_waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_y_waste_spin));
		margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->rect_margin_spin));
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_round_radio))) {
		shape = GL_TEMPLATE_SHAPE_ROUND;
		r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_r_spin));
		waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_waste_spin));
		margin = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->round_margin_spin));
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->priv->shape_cd_radio))) {
		shape = GL_TEMPLATE_SHAPE_CD;
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


	template = gl_template_new (name, desc, paper->id, paper->width, paper->height);

	switch (shape) {
	case GL_TEMPLATE_SHAPE_RECT:
		label_type =
			gl_template_rect_label_type_new ("0",
							 w/upp, h/upp, r/upp,
							 x_waste/upp, y_waste/upp);
		break;
	case GL_TEMPLATE_SHAPE_ROUND:
		label_type =
			gl_template_round_label_type_new ("0", r/upp, waste/upp);
		break;
	case GL_TEMPLATE_SHAPE_CD:
		label_type =
			gl_template_cd_label_type_new ("0",
						       radius/upp, hole/upp,
						       w/upp, h/upp,
						       waste/upp);
		break;
	}
	gl_template_add_label_type (template, label_type);

	gl_template_add_markup (label_type,
				gl_template_markup_margin_new (margin/upp));

	gl_template_add_layout (label_type,
				gl_template_layout_new (nx_1, ny_1,
							x0_1/upp,
							y0_1/upp,
							dx_1/upp,
							dy_1/upp));
	if (nlayouts > 1) {
		gl_template_add_layout (label_type,
					gl_template_layout_new (nx_2, ny_2,
								x0_2/upp,
								y0_2/upp,
								dx_2/upp,
								dy_2/upp));
	}

	g_free (brand);
	g_free (part_num);
	g_free (name);
	g_free (desc);

	g_free (page_size_name);
	gl_paper_free (paper);

	return template;
}

/*****************************************************************************/
/* Construct mini preview widget.                                            */
/*****************************************************************************/
GtkWidget *
gl_template_designer_construct_mini_preview (gchar *name,
					     gchar *string1,
					     gchar *string2,
					     gint   int1,
					     gint   int2)
{
	gint width  = int1;
	gint height = int2;

	return gl_wdgt_mini_preview_new (height, width);
}

