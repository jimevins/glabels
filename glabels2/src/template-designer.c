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

#include <math.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <glade/glade-xml.h>

#include "template-designer.h"
#include "prefs.h"
#include "paper.h"
#include "wdgt-mini-preview.h"
#include "print-dialog.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define ICON_PIXMAP gnome_program_locate_file (NULL,\
                                               GNOME_FILE_DOMAIN_APP_PIXMAP,\
                                               "glabels/glabels-icon.png",\
                                               FALSE, NULL)

#define EX_1LAYOUT_PIXMAP gnome_program_locate_file (NULL,\
                                               GNOME_FILE_DOMAIN_APP_PIXMAP,\
                                               "glabels/ex-1layout.png",\
                                               FALSE, NULL)

#define EX_2LAYOUTS_PIXMAP gnome_program_locate_file (NULL,\
                                               GNOME_FILE_DOMAIN_APP_PIXMAP,\
                                               "glabels/ex-2layouts.png",\
                                               FALSE, NULL)
#define CONTENTS_BG 0xF5F5F5FF
#define DELTA 0.01
#define MAX_PAGE_DIM_POINTS 5000.0

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glTemplateDesignerPrivate
{
	GladeXML        *gui;
	GtkWidget       *druid;

	/* Druid pages */
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
	GtkWidget       *pg_size_entry;
	GtkWidget       *pg_w_spin;
	GtkWidget       *pg_h_spin;
	GtkWidget       *pg_w_units_label;
	GtkWidget       *pg_h_units_label;

	/* Shape page controls */
	GtkWidget       *shape_rect_radio;
	GtkWidget       *shape_round_radio;
	GtkWidget       *shape_cd_radio;

	/* Label size (rectangular) page controls */
	GtkWidget       *rect_w_spin;
	GtkWidget       *rect_h_spin;
	GtkWidget       *rect_r_spin;
	GtkWidget       *rect_waste_spin;
	GtkWidget       *rect_w_units_label;
	GtkWidget       *rect_h_units_label;
	GtkWidget       *rect_r_units_label;
	GtkWidget       *rect_waste_units_label;

	/* Label size (round) page controls */
	GtkWidget       *round_r_spin;
	GtkWidget       *round_waste_spin;
	GtkWidget       *round_r_units_label;
	GtkWidget       *round_waste_units_label;

	/* Label size (cd) page controls */
	GtkWidget       *cd_radius_spin;
	GtkWidget       *cd_hole_spin;
	GtkWidget       *cd_w_spin;
	GtkWidget       *cd_h_spin;
	GtkWidget       *cd_waste_spin;
	GtkWidget       *cd_radius_units_label;
	GtkWidget       *cd_hole_units_label;
	GtkWidget       *cd_w_units_label;
	GtkWidget       *cd_h_units_label;
	GtkWidget       *cd_waste_units_label;

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

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GtkWindowClass* parent_class = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_template_designer_class_init 	  (glTemplateDesignerClass *klass);
static void gl_template_designer_init       	  (glTemplateDesigner      *dlg);
static void gl_template_designer_finalize   	  (GObject                 *object);
static void gl_template_designer_construct        (glTemplateDesigner      *dlg);

static void     construct_start_page              (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_name_page               (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_pg_size_page            (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_shape_page              (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_rect_size_page          (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_round_size_page         (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_cd_size_page            (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_nlayouts_page           (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_layout_page             (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_finish_page             (glTemplateDesigner      *dlg,
						   GdkPixbuf               *logo);

static void     construct_edge_page_boilerplate   (glTemplateDesigner      *dlg,
						   GnomeDruidPageEdge      *page,
						   GdkPixbuf               *logo);

static void     construct_page_boilerplate        (glTemplateDesigner      *dlg,
						   GnomeDruidPageStandard  *page,
						   GdkPixbuf               *logo);

static void     cancel_cb                         (glTemplateDesigner      *dlg);

static void     name_page_changed_cb              (glTemplateDesigner      *dlg);

static void     pg_size_page_changed_cb           (glTemplateDesigner      *dlg);

static void     rect_size_page_prepare_cb         (glTemplateDesigner      *dlg);

static void     round_size_page_prepare_cb        (glTemplateDesigner      *dlg);

static void     cd_size_page_prepare_cb           (glTemplateDesigner      *dlg);

static void     layout_page_prepare_cb            (glTemplateDesigner      *dlg);

static void     layout_page_changed_cb            (glTemplateDesigner      *dlg);

static void     print_test_cb                     (glTemplateDesigner      *dlg);

static glTemplate *build_template                 (glTemplateDesigner      *dlg);

static gboolean next_cb                           (GnomeDruidPage          *druidpage,
						   GtkWidget               *widget,
						   glTemplateDesigner      *dlg);

static gboolean back_cb                           (GnomeDruidPage          *druidpage,
						   GtkWidget               *widget,
						   glTemplateDesigner      *dlg);

static void finish_cb                             (glTemplateDesigner      *dlg);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_template_designer_get_type (void)
{
	static GType dialog_type = 0;

	if (!dialog_type)
    	{
      		static const GTypeInfo dialog_info =
      		{
			sizeof (glTemplateDesignerClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) gl_template_designer_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (glTemplateDesigner),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gl_template_designer_init
      		};

     		dialog_type = g_type_register_static (GTK_TYPE_WINDOW,
						      "glTemplateDesigner",
						      &dialog_info, 
						      0);
    	}

	return dialog_type;
}

static void
gl_template_designer_class_init (glTemplateDesignerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	gl_debug (DEBUG_TEMPLATE, "");
	
  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gl_template_designer_finalize;  	
}

static void
gl_template_designer_init (glTemplateDesigner *dlg)
{
	gl_debug (DEBUG_TEMPLATE, "START");

	dlg->priv = g_new0 (glTemplateDesignerPrivate, 1);

	dlg->priv->gui = glade_xml_new (GLABELS_GLADE_DIR "template-designer.glade",
					"druid",
					NULL);

	if (!dlg->priv->gui) {
		g_warning ("Could not open template-designer.glade, reinstall glabels!");
		return;
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

static void 
gl_template_designer_finalize (GObject *object)
{
	glTemplateDesigner* dlg;
	
	gl_debug (DEBUG_TEMPLATE, "START");

	g_return_if_fail (object != NULL);
	
   	dlg = GL_TEMPLATE_DESIGNER (object);

	g_return_if_fail (GL_IS_TEMPLATE_DESIGNER (dlg));
	g_return_if_fail (dlg->priv != NULL);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	g_free (dlg->priv);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*****************************************************************************/
/* NEW preferences dialog.                                                   */
/*****************************************************************************/
GtkWidget*
gl_template_designer_new (GtkWindow *parent)
{
	GtkWidget *dlg;

	gl_debug (DEBUG_TEMPLATE, "START");

	dlg = GTK_WIDGET (g_object_new (GL_TYPE_TEMPLATE_DESIGNER, NULL));

	if (parent)
		gtk_window_set_transient_for (GTK_WINDOW (dlg), parent);
	
	gl_template_designer_construct (GL_TEMPLATE_DESIGNER(dlg));


	gl_debug (DEBUG_TEMPLATE, "END");

	return dlg;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_template_designer_construct (glTemplateDesigner *dlg)
{
	GdkPixbuf  *logo;

	g_return_if_fail (dlg && GL_IS_TEMPLATE_DESIGNER (dlg));
	g_return_if_fail (dlg->priv != NULL);

	/* Initialize units stuff from prefs */
	dlg->priv->units_string    = gl_prefs_get_units_string ();
        dlg->priv->units_per_point = gl_prefs_get_units_per_point ();
        dlg->priv->climb_rate      = gl_prefs_get_units_step_size ();
        dlg->priv->digits          = gl_prefs_get_units_precision ();

	gtk_window_set_title (GTK_WINDOW(dlg), _("gLabels Template Designer"));

	logo = gdk_pixbuf_new_from_file (ICON_PIXMAP, NULL);

	dlg->priv->druid = glade_xml_get_widget (dlg->priv->gui, "druid");
	gtk_container_add (GTK_CONTAINER(dlg), dlg->priv->druid);

	construct_start_page (dlg, logo);
	construct_name_page (dlg, logo);
	construct_pg_size_page (dlg, logo);
	construct_shape_page (dlg, logo);
	construct_rect_size_page (dlg, logo);
	construct_round_size_page (dlg, logo);
	construct_cd_size_page (dlg, logo);
	construct_nlayouts_page (dlg, logo);
	construct_layout_page (dlg, logo);
	construct_finish_page (dlg, logo);

	/* Cancel button */
	g_signal_connect_swapped (G_OBJECT(dlg->priv->druid), "cancel",
				  G_CALLBACK(cancel_cb), dlg);

        gtk_widget_show_all (GTK_WIDGET(dlg));   
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct start page.                                          */
/*--------------------------------------------------------------------------*/
static void
construct_start_page (glTemplateDesigner      *dlg,
		      GdkPixbuf               *logo)
{
	dlg->priv->start_page =
		glade_xml_get_widget (dlg->priv->gui, "start_page");

	construct_edge_page_boilerplate (dlg,
					 GNOME_DRUID_PAGE_EDGE(dlg->priv->start_page),
					 logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct name page.                                           */
/*--------------------------------------------------------------------------*/
static void
construct_name_page (glTemplateDesigner      *dlg,
		     GdkPixbuf               *logo)
{
	dlg->priv->name_page =
		glade_xml_get_widget (dlg->priv->gui, "name_page");

	/* Name Page Widgets */
	dlg->priv->brand_entry =
		glade_xml_get_widget (dlg->priv->gui, "brand_entry");
	dlg->priv->part_num_entry =
		glade_xml_get_widget (dlg->priv->gui, "part_num_entry");
	dlg->priv->description_entry =
		glade_xml_get_widget (dlg->priv->gui, "description_entry");

	/* Connect a handler that listens for changes in these widgets */
	/* This controls whether we can progress to the next page. */
	g_signal_connect_swapped (G_OBJECT(dlg->priv->brand_entry), "changed",
				  G_CALLBACK(name_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->part_num_entry), "changed",
				  G_CALLBACK(name_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->description_entry), "changed",
				  G_CALLBACK(name_page_changed_cb), dlg);

	/* Use this same handler to prepare the page. */
	g_signal_connect_data (G_OBJECT(dlg->priv->name_page), "prepare",
			       G_CALLBACK(name_page_changed_cb), dlg,
			       NULL, (G_CONNECT_AFTER|G_CONNECT_SWAPPED));

	construct_page_boilerplate (dlg,
				    GNOME_DRUID_PAGE_STANDARD(dlg->priv->name_page),
				    logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Construct page size page.                                       */
/*--------------------------------------------------------------------------*/
static void
construct_pg_size_page (glTemplateDesigner      *dlg,
			GdkPixbuf               *logo)
{
	GList *page_sizes;
	const gchar *default_page_size_id;
	gchar       *default_page_size_name;

	dlg->priv->pg_size_page =
		glade_xml_get_widget (dlg->priv->gui, "pg_size_page");

	/* Page Size Page Widgets */
	dlg->priv->pg_size_combo =
		glade_xml_get_widget (dlg->priv->gui, "pg_size_combo");
	dlg->priv->pg_size_entry =
		glade_xml_get_widget (dlg->priv->gui, "pg_size_entry");
	dlg->priv->pg_w_spin =
		glade_xml_get_widget (dlg->priv->gui, "pg_w_spin");
	dlg->priv->pg_h_spin =
		glade_xml_get_widget (dlg->priv->gui, "pg_h_spin");
	dlg->priv->pg_w_units_label =
		glade_xml_get_widget (dlg->priv->gui, "pg_w_units_label");
	dlg->priv->pg_h_units_label =
		glade_xml_get_widget (dlg->priv->gui, "pg_h_units_label");

	/* Load page size combo */
	page_sizes = gl_paper_get_name_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO(dlg->priv->pg_size_combo), page_sizes);
	gl_paper_free_name_list (&page_sizes);
	default_page_size_id = gl_prefs_get_page_size ();
	default_page_size_name = gl_paper_lookup_name_from_id (default_page_size_id);
	gtk_entry_set_text (GTK_ENTRY(dlg->priv->pg_size_entry), default_page_size_name);
	g_free (default_page_size_name);

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->pg_w_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->pg_w_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->pg_w_units_label),
			    dlg->priv->units_string);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->pg_w_spin),
                                   0.0, MAX_PAGE_DIM_POINTS*dlg->priv->units_per_point);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->pg_h_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->pg_h_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->pg_h_units_label),
			    dlg->priv->units_string);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->pg_h_spin),
                                   0.0, MAX_PAGE_DIM_POINTS*dlg->priv->units_per_point);

	/* Connect a handler that listens for changes in these widgets */
	/* This controls sensitivity of related widgets. */
	g_signal_connect_swapped (G_OBJECT(dlg->priv->pg_size_entry), "changed",
				  G_CALLBACK(pg_size_page_changed_cb), dlg);

	/* Use this same handler to prepare the page. */
	g_signal_connect_data (G_OBJECT(dlg->priv->pg_size_page), "prepare",
			       G_CALLBACK(pg_size_page_changed_cb), dlg,
			       NULL, (G_CONNECT_AFTER|G_CONNECT_SWAPPED));

	construct_page_boilerplate (dlg,
				    GNOME_DRUID_PAGE_STANDARD(dlg->priv->pg_size_page),
				    logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct shape page.                                          */
/*--------------------------------------------------------------------------*/
static void
construct_shape_page (glTemplateDesigner      *dlg,
		      GdkPixbuf               *logo)
{
	dlg->priv->shape_page =
		glade_xml_get_widget (dlg->priv->gui, "shape_page");

	/* Shape Page Widgets */
	dlg->priv->shape_rect_radio =
		glade_xml_get_widget (dlg->priv->gui, "shape_rect_radio");
	dlg->priv->shape_round_radio =
		glade_xml_get_widget (dlg->priv->gui, "shape_round_radio");
	dlg->priv->shape_cd_radio =
		glade_xml_get_widget (dlg->priv->gui, "shape_cd_radio");

	construct_page_boilerplate (dlg,
				    GNOME_DRUID_PAGE_STANDARD(dlg->priv->shape_page),
				    logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct rect size page.                                      */
/*--------------------------------------------------------------------------*/
static void
construct_rect_size_page (glTemplateDesigner      *dlg,
			  GdkPixbuf               *logo)
{
	dlg->priv->rect_size_page =
		glade_xml_get_widget (dlg->priv->gui, "rect_size_page");

	/* Rect Size Page Widgets */
	dlg->priv->rect_w_spin =
		glade_xml_get_widget (dlg->priv->gui, "rect_w_spin");
	dlg->priv->rect_h_spin =
		glade_xml_get_widget (dlg->priv->gui, "rect_h_spin");
	dlg->priv->rect_r_spin =
		glade_xml_get_widget (dlg->priv->gui, "rect_r_spin");
	dlg->priv->rect_waste_spin =
		glade_xml_get_widget (dlg->priv->gui, "rect_waste_spin");
	dlg->priv->rect_w_units_label =
		glade_xml_get_widget (dlg->priv->gui, "rect_w_units_label");
	dlg->priv->rect_h_units_label =
		glade_xml_get_widget (dlg->priv->gui, "rect_h_units_label");
	dlg->priv->rect_r_units_label =
		glade_xml_get_widget (dlg->priv->gui, "rect_r_units_label");
	dlg->priv->rect_waste_units_label =
		glade_xml_get_widget (dlg->priv->gui, "rect_waste_units_label");

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->rect_w_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->rect_w_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->rect_w_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->rect_h_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->rect_h_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->rect_h_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->rect_r_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->rect_r_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->rect_r_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->rect_waste_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->rect_waste_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->rect_waste_units_label),
			    dlg->priv->units_string);

	/* Handler to prepare the page. */
	g_signal_connect_data (G_OBJECT(dlg->priv->rect_size_page), "prepare",
			       G_CALLBACK(rect_size_page_prepare_cb), dlg,
			       NULL, (G_CONNECT_AFTER|G_CONNECT_SWAPPED));

	construct_page_boilerplate (dlg,
				    GNOME_DRUID_PAGE_STANDARD(dlg->priv->rect_size_page),
				    logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct round size page.                                     */
/*--------------------------------------------------------------------------*/
static void
construct_round_size_page (glTemplateDesigner      *dlg,
			   GdkPixbuf               *logo)
{
	dlg->priv->round_size_page =
		glade_xml_get_widget (dlg->priv->gui, "round_size_page");

	/* Round Size Page Widgets */
	dlg->priv->round_r_spin =
		glade_xml_get_widget (dlg->priv->gui, "round_r_spin");
	dlg->priv->round_waste_spin =
		glade_xml_get_widget (dlg->priv->gui, "round_waste_spin");
	dlg->priv->round_r_units_label =
		glade_xml_get_widget (dlg->priv->gui, "round_r_units_label");
	dlg->priv->round_waste_units_label =
		glade_xml_get_widget (dlg->priv->gui, "round_waste_units_label");

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->round_r_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->round_r_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->round_r_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->round_waste_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->round_waste_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->round_waste_units_label),
			    dlg->priv->units_string);

	/* Handler to prepare the page. */
	g_signal_connect_data (G_OBJECT(dlg->priv->round_size_page), "prepare",
			       G_CALLBACK(round_size_page_prepare_cb), dlg,
			       NULL, (G_CONNECT_AFTER|G_CONNECT_SWAPPED));

	construct_page_boilerplate (dlg,
				    GNOME_DRUID_PAGE_STANDARD(dlg->priv->round_size_page),
				    logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct CD/DVD size page.                                    */
/*--------------------------------------------------------------------------*/
static void
construct_cd_size_page (glTemplateDesigner      *dlg,
			GdkPixbuf               *logo)
{
	dlg->priv->cd_size_page =
		glade_xml_get_widget (dlg->priv->gui, "cd_size_page");

	/* Cd Size Page Widgets */
	dlg->priv->cd_radius_spin =
		glade_xml_get_widget (dlg->priv->gui, "cd_radius_spin");
	dlg->priv->cd_hole_spin =
		glade_xml_get_widget (dlg->priv->gui, "cd_hole_spin");
	dlg->priv->cd_w_spin =
		glade_xml_get_widget (dlg->priv->gui, "cd_w_spin");
	dlg->priv->cd_h_spin =
		glade_xml_get_widget (dlg->priv->gui, "cd_h_spin");
	dlg->priv->cd_waste_spin =
		glade_xml_get_widget (dlg->priv->gui, "cd_waste_spin");
	dlg->priv->cd_radius_units_label =
		glade_xml_get_widget (dlg->priv->gui, "cd_radius_units_label");
	dlg->priv->cd_hole_units_label =
		glade_xml_get_widget (dlg->priv->gui, "cd_hole_units_label");
	dlg->priv->cd_w_units_label =
		glade_xml_get_widget (dlg->priv->gui, "cd_w_units_label");
	dlg->priv->cd_h_units_label =
		glade_xml_get_widget (dlg->priv->gui, "cd_h_units_label");
	dlg->priv->cd_waste_units_label =
		glade_xml_get_widget (dlg->priv->gui, "cd_waste_units_label");

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->cd_radius_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->cd_radius_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->cd_radius_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->cd_hole_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->cd_hole_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->cd_hole_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->cd_w_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->cd_w_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->cd_w_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->cd_h_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->cd_h_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->cd_h_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->cd_waste_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->cd_waste_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->cd_waste_units_label),
			    dlg->priv->units_string);

	/* Handler to prepare the page. */
	g_signal_connect_data (G_OBJECT(dlg->priv->cd_size_page), "prepare",
			       G_CALLBACK(cd_size_page_prepare_cb), dlg,
			       NULL, (G_CONNECT_AFTER|G_CONNECT_SWAPPED));

	construct_page_boilerplate (dlg,
				    GNOME_DRUID_PAGE_STANDARD(dlg->priv->cd_size_page),
				    logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct number of layouts page.                              */
/*--------------------------------------------------------------------------*/
static void
construct_nlayouts_page (glTemplateDesigner      *dlg,
			 GdkPixbuf               *logo)
{
	GdkPixbuf *pixbuf;

	dlg->priv->nlayouts_page =
		glade_xml_get_widget (dlg->priv->gui, "nlayouts_page");

	/* Widgets */
	dlg->priv->nlayouts_image1 =
		glade_xml_get_widget (dlg->priv->gui, "nlayouts_image1");
	dlg->priv->nlayouts_image2 =
		glade_xml_get_widget (dlg->priv->gui, "nlayouts_image2");
	dlg->priv->nlayouts_spin =
		glade_xml_get_widget (dlg->priv->gui, "nlayouts_spin");

	/* Initialize illustrations. */
	pixbuf = gdk_pixbuf_new_from_file (EX_1LAYOUT_PIXMAP, NULL);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dlg->priv->nlayouts_image1), pixbuf);
	pixbuf = gdk_pixbuf_new_from_file (EX_2LAYOUTS_PIXMAP, NULL);
	gtk_image_set_from_pixbuf (GTK_IMAGE(dlg->priv->nlayouts_image2), pixbuf);

	construct_page_boilerplate (dlg,
				    GNOME_DRUID_PAGE_STANDARD(dlg->priv->nlayouts_page),
				    logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct layout page.                                         */
/*--------------------------------------------------------------------------*/
static void
construct_layout_page (glTemplateDesigner      *dlg,
		       GdkPixbuf               *logo)
{
	dlg->priv->layout_page =
		glade_xml_get_widget (dlg->priv->gui, "layout_page");

	/* Page Size Page Widgets */
	dlg->priv->layout1_head_label =
		glade_xml_get_widget (dlg->priv->gui, "layout1_head_label");
	dlg->priv->layout1_nx_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout1_nx_spin");
	dlg->priv->layout1_ny_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout1_ny_spin");
	dlg->priv->layout1_x0_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout1_x0_spin");
	dlg->priv->layout1_y0_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout1_y0_spin");
	dlg->priv->layout1_dx_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout1_dx_spin");
	dlg->priv->layout1_dy_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout1_dy_spin");
	dlg->priv->layout2_head_label =
		glade_xml_get_widget (dlg->priv->gui, "layout2_head_label");
	dlg->priv->layout2_nx_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout2_nx_spin");
	dlg->priv->layout2_ny_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout2_ny_spin");
	dlg->priv->layout2_x0_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout2_x0_spin");
	dlg->priv->layout2_y0_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout2_y0_spin");
	dlg->priv->layout2_dx_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout2_dx_spin");
	dlg->priv->layout2_dy_spin =
		glade_xml_get_widget (dlg->priv->gui, "layout2_dy_spin");
	dlg->priv->layout_x0_units_label =
		glade_xml_get_widget (dlg->priv->gui, "layout_x0_units_label");
	dlg->priv->layout_y0_units_label =
		glade_xml_get_widget (dlg->priv->gui, "layout_y0_units_label");
	dlg->priv->layout_dx_units_label =
		glade_xml_get_widget (dlg->priv->gui, "layout_dx_units_label");
	dlg->priv->layout_dy_units_label =
		glade_xml_get_widget (dlg->priv->gui, "layout_dy_units_label");
	dlg->priv->layout_mini_preview =
		glade_xml_get_widget (dlg->priv->gui, "layout_mini_preview");
	dlg->priv->layout_test_button =
		glade_xml_get_widget (dlg->priv->gui, "layout_test_button");

	/* Apply units to spinbuttons and units labels. */
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->layout1_x0_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->layout1_x0_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->layout2_x0_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->layout2_x0_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->layout_x0_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->layout1_y0_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->layout1_y0_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->layout2_y0_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->layout2_y0_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->layout_y0_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->layout1_dx_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->layout1_dx_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->layout2_dx_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->layout2_dx_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->layout_dx_units_label),
			    dlg->priv->units_string);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->layout1_dy_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->layout1_dy_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON(dlg->priv->layout2_dy_spin),
				    dlg->priv->digits);
        gtk_spin_button_set_increments (GTK_SPIN_BUTTON(dlg->priv->layout2_dy_spin),
                                        dlg->priv->climb_rate, 10.0*dlg->priv->climb_rate);
        gtk_label_set_text (GTK_LABEL(dlg->priv->layout_dy_units_label),
			    dlg->priv->units_string);

	/* Adjust preview */
	gl_wdgt_mini_preview_set_bg_color (GL_WDGT_MINI_PREVIEW(dlg->priv->layout_mini_preview),
					   CONTENTS_BG);

	/* Connect a handler that listens for changes in these widgets */
	/* This controls sensitivity of related widgets. */
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout1_nx_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout1_ny_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout1_x0_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout1_y0_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout1_dx_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout1_dy_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout2_nx_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout2_ny_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout2_x0_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout2_y0_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout2_dx_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout2_dy_spin), "changed",
				  G_CALLBACK(layout_page_changed_cb), dlg);


	/* Print button */
	g_signal_connect_swapped (G_OBJECT(dlg->priv->layout_test_button), "clicked",
				  G_CALLBACK(print_test_cb), dlg);

	/* Handler to prepare the page. */
	g_signal_connect_data (G_OBJECT(dlg->priv->layout_page), "prepare",
			       G_CALLBACK(layout_page_prepare_cb), dlg,
			       NULL, (G_CONNECT_AFTER|G_CONNECT_SWAPPED));

	construct_page_boilerplate (dlg,
				    GNOME_DRUID_PAGE_STANDARD(dlg->priv->layout_page),
				    logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct finish page.                                         */
/*--------------------------------------------------------------------------*/
static void
construct_finish_page (glTemplateDesigner      *dlg,
		       GdkPixbuf               *logo)
{

	dlg->priv->finish_page =
		glade_xml_get_widget (dlg->priv->gui, "finish_page");

	/* Accept button */
	g_signal_connect_swapped (G_OBJECT(dlg->priv->finish_page), "finish",
				  G_CALLBACK(finish_cb), dlg);

	construct_edge_page_boilerplate (dlg,
					 GNOME_DRUID_PAGE_EDGE(dlg->priv->start_page),
					 logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Common construction tasks for start and finish page.           */
/*--------------------------------------------------------------------------*/
static void
construct_edge_page_boilerplate (glTemplateDesigner      *dlg,
				 GnomeDruidPageEdge      *page,
				 GdkPixbuf               *logo)
{
	gnome_druid_page_edge_set_logo (page, logo);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Common construction tasks for all other pages.                 */
/*--------------------------------------------------------------------------*/
static void
construct_page_boilerplate (glTemplateDesigner      *dlg,
			    GnomeDruidPageStandard  *page,
			    GdkPixbuf               *logo)
{
	gnome_druid_page_standard_set_logo (page, logo);

	g_signal_connect (G_OBJECT(page), "next", G_CALLBACK(next_cb), dlg);
	g_signal_connect (G_OBJECT(page), "back", G_CALLBACK(back_cb), dlg);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Cancel druid.                                                  */
/*--------------------------------------------------------------------------*/
static void
cancel_cb (glTemplateDesigner *dlg)
{
                                                                               
	gtk_widget_destroy (GTK_WIDGET(dlg));

}
                         
/*--------------------------------------------------------------------------*/
/* PRIVATE.  Widget on name page "changed" callback.                        */
/*--------------------------------------------------------------------------*/
static void
name_page_changed_cb (glTemplateDesigner *dlg)
{
	gchar *brand, *part_num, *desc;

	brand    = gtk_editable_get_chars (GTK_EDITABLE(dlg->priv->brand_entry), 0, -1);
	part_num = gtk_editable_get_chars (GTK_EDITABLE(dlg->priv->part_num_entry), 0, -1);
	desc     = gtk_editable_get_chars (GTK_EDITABLE(dlg->priv->description_entry), 0, -1);

	if (brand && brand[0] && part_num && part_num[0] && desc && desc[0]) {

		gnome_druid_set_buttons_sensitive (GNOME_DRUID(dlg->priv->druid),
						   TRUE, TRUE, TRUE, FALSE);

	} else {

		gnome_druid_set_buttons_sensitive (GNOME_DRUID(dlg->priv->druid),
						   TRUE, FALSE, TRUE, FALSE);

	}

	g_free (brand);
	g_free (part_num);
	g_free (desc);

}
                                                                               
/*--------------------------------------------------------------------------*/
/* PRIVATE.  Widget on page size page "changed" callback.                   */
/*--------------------------------------------------------------------------*/
static void
pg_size_page_changed_cb (glTemplateDesigner *dlg)
{
	gchar   *page_size_name;
	glPaper *paper;
	

	page_size_name =
		gtk_editable_get_chars (GTK_EDITABLE(dlg->priv->pg_size_entry), 0, -1);

	if (page_size_name && strlen(page_size_name)) {

		paper = gl_paper_from_name (page_size_name);
	

		if ( g_strcasecmp (paper->id, "Other") == 0 ) {
			gtk_widget_set_sensitive (GTK_WIDGET(dlg->priv->pg_w_spin), TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET(dlg->priv->pg_h_spin), TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET(dlg->priv->pg_w_units_label),
						  TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET(dlg->priv->pg_h_units_label),
						  TRUE);

		} else {
			gtk_widget_set_sensitive (GTK_WIDGET(dlg->priv->pg_w_spin), FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET(dlg->priv->pg_h_spin), FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET(dlg->priv->pg_w_units_label),
						  FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET(dlg->priv->pg_h_units_label),
						  FALSE);

			gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->pg_w_spin),
						   paper->width * dlg->priv->units_per_point);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->pg_h_spin),
						   paper->height * dlg->priv->units_per_point);
		}

		gl_paper_free (&paper);
	}

	g_free (page_size_name);

}
                                                                               
/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare rectangular size page.                                 */
/*--------------------------------------------------------------------------*/
static void
rect_size_page_prepare_cb (glTemplateDesigner *dlg)
{
	gdouble max_w, max_h;
	gdouble w, h, r, waste;

	/* Limit label size based on already chosen page size. */
	max_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_w_spin));
	max_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_h_spin));

	w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_w_spin));
	h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_h_spin));
	r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_r_spin));
	waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_waste_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->rect_w_spin),
                                   dlg->priv->climb_rate, max_w);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->rect_h_spin),
                                   dlg->priv->climb_rate, max_h);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->rect_r_spin),
                                   0.0, MIN(max_w, max_h)/2.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->rect_waste_spin),
                                   0.0, MIN(max_w, max_h)/2.0);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->rect_w_spin), w);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->rect_h_spin), h);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->rect_r_spin), r);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->rect_waste_spin), waste);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare round size page.                                       */
/*--------------------------------------------------------------------------*/
static void
round_size_page_prepare_cb (glTemplateDesigner *dlg)
{
	gdouble max_w, max_h;
	gdouble r, waste;

	/* Limit label size based on already chosen page size. */
	max_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_w_spin));
	max_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_h_spin));

	r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->round_r_spin));
	waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->round_waste_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->round_r_spin),
                                   dlg->priv->climb_rate, MIN(max_w, max_h)/2.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->round_waste_spin),
                                   0.0, MIN(max_w, max_h)/2.0);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->round_r_spin), r);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->round_waste_spin), waste);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare cd/dvd size page.                                      */
/*--------------------------------------------------------------------------*/
static void
cd_size_page_prepare_cb (glTemplateDesigner *dlg)
{
	gdouble max_w, max_h;
	gdouble radius, hole, w, h, waste;

	/* Limit label size based on already chosen page size. */
	max_w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_w_spin));
	max_h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_h_spin));

	radius = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_radius_spin));
	hole = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_hole_spin));
	w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_w_spin));
	h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_h_spin));
	waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_waste_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->cd_radius_spin),
                                   dlg->priv->climb_rate, MIN(max_w, max_h)/2.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->cd_hole_spin),
                                   dlg->priv->climb_rate, MIN(max_w, max_h)/2.0);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->cd_w_spin),
                                   0.0, max_w);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->cd_h_spin),
                                   0.0, max_h);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->cd_waste_spin),
                                   0.0, MIN(max_w, max_h)/2.0);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->cd_radius_spin), radius);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->cd_hole_spin), hole);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->cd_w_spin), w);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->cd_h_spin), h);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->cd_waste_spin), waste);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Layout page widget changed cb.                                 */
/*--------------------------------------------------------------------------*/
static void
layout_page_prepare_cb (glTemplateDesigner *dlg)
{
	gdouble min_x, min_y;
	gdouble max_x, max_y;
	gint    max_nx, max_ny;
	gint    nlayouts;
	gdouble nx_1, ny_1, x0_1, y0_1, dx_1, dy_1;
	gdouble nx_2, ny_2, x0_2, y0_2, dx_2, dy_2;

	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout1_nx_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout1_ny_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout1_x0_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout1_y0_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout1_dx_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout1_dy_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout2_nx_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout2_ny_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout2_x0_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout2_y0_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout2_dx_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));
	g_signal_handlers_block_by_func (G_OBJECT(dlg->priv->layout2_dy_spin),
					 G_CALLBACK(layout_page_changed_cb),
					 G_OBJECT(dlg));

	/* Limit ranges based on already chosen page and label sizes. */
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_rect_radio))) {
		min_x = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_w_spin));
		min_y = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_h_spin));
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_round_radio))) {
		min_x = 2*gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->round_r_spin));
		min_y = 2*min_x;
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_cd_radio))) {
		min_x = 2*gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_radius_spin));
		min_y = 2*min_x;
	}
	max_x = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_w_spin));
	max_y = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_h_spin));
	max_nx = MAX (floor (max_x/min_x + DELTA), 1.0);
	max_ny = MAX (floor (max_y/min_y + DELTA), 1.0);

	nx_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_nx_spin));
	ny_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_ny_spin));
	x0_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_x0_spin));
	y0_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_y0_spin));
	dx_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_dx_spin));
	dy_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_dy_spin));
	nx_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_nx_spin));
	ny_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_ny_spin));
	x0_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_x0_spin));
	y0_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_y0_spin));
	dx_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_dx_spin));
	dy_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_dy_spin));

	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout1_nx_spin),
                                   1, max_nx);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout1_ny_spin),
                                   1, max_ny);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout1_x0_spin),
                                   0.0, max_x-min_x);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout1_y0_spin),
                                   0.0, max_y-min_y);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout1_dx_spin),
                                   min_x, max_x);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout1_dy_spin),
                                   min_y, max_y);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout2_nx_spin),
                                   1, max_nx);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout2_ny_spin),
                                   1, max_ny);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout2_x0_spin),
                                   0.0, max_x-min_x);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout2_y0_spin),
                                   0.0, max_y-min_y);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout2_dx_spin),
                                   min_x, max_x);
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (dlg->priv->layout2_dy_spin),
                                   min_y, max_y);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout1_nx_spin), nx_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout1_ny_spin), ny_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout1_x0_spin), x0_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout1_y0_spin), y0_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout1_dx_spin), dx_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout1_dy_spin), dy_1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout2_nx_spin), nx_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout2_ny_spin), ny_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout2_x0_spin), x0_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout2_y0_spin), y0_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout2_dx_spin), dx_2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dlg->priv->layout2_dy_spin), dy_2);

	/* Set visibility of layout2 widgets as appropriate. */
	nlayouts = gtk_spin_button_get_value (GTK_SPIN_BUTTON (dlg->priv->nlayouts_spin));
	if ( nlayouts == 1 ) {

		gtk_widget_hide (dlg->priv->layout1_head_label);
		gtk_widget_hide (dlg->priv->layout2_head_label);
		gtk_widget_hide (dlg->priv->layout2_nx_spin);
		gtk_widget_hide (dlg->priv->layout2_ny_spin);
		gtk_widget_hide (dlg->priv->layout2_x0_spin);
		gtk_widget_hide (dlg->priv->layout2_y0_spin);
		gtk_widget_hide (dlg->priv->layout2_dx_spin);
		gtk_widget_hide (dlg->priv->layout2_dy_spin);

	} else {

		gtk_widget_show (dlg->priv->layout1_head_label);
		gtk_widget_show (dlg->priv->layout2_head_label);
		gtk_widget_show (dlg->priv->layout2_nx_spin);
		gtk_widget_show (dlg->priv->layout2_ny_spin);
		gtk_widget_show (dlg->priv->layout2_x0_spin);
		gtk_widget_show (dlg->priv->layout2_y0_spin);
		gtk_widget_show (dlg->priv->layout2_dx_spin);
		gtk_widget_show (dlg->priv->layout2_dy_spin);

	}

	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout1_nx_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout1_ny_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout1_x0_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout1_y0_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout1_dx_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout1_dy_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout2_nx_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout2_ny_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout2_x0_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout2_y0_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout2_dx_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (G_OBJECT(dlg->priv->layout2_dy_spin),
					   G_CALLBACK(layout_page_changed_cb),
					   G_OBJECT(dlg));

	layout_page_changed_cb (dlg);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Layout page widget changed cb.                                 */
/*--------------------------------------------------------------------------*/
static void
layout_page_changed_cb (glTemplateDesigner *dlg)
{
	glTemplate *template;

	template = build_template (dlg);

	gl_wdgt_mini_preview_set_template (GL_WDGT_MINI_PREVIEW(dlg->priv->layout_mini_preview),
					   template);

	gl_template_free (&template);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Print test sheet callback.                                     */
/*--------------------------------------------------------------------------*/
static void
print_test_cb (glTemplateDesigner      *dlg)
{
	GObject    *label;
	glTemplate *template;
	GtkWidget  *print_dialog;

	label = gl_label_new ();

	template = build_template (dlg);
	gl_label_set_template (GL_LABEL(label), template);

	print_dialog = gl_print_dialog_new (GL_LABEL(label), NULL);
	gl_print_dialog_force_outline_flag (GL_PRINT_DIALOG(print_dialog));
	gtk_widget_show (print_dialog);

	gl_template_free (&template);
	g_object_unref (G_OBJECT(label));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Finish druid.                                                  */
/*--------------------------------------------------------------------------*/
static void
finish_cb (glTemplateDesigner *dlg)
{
	glTemplate *template;
	
	template = build_template (dlg);
	gl_template_register (template);
                                                                               
	gtk_widget_destroy (GTK_WIDGET(dlg));

}
                         
/*--------------------------------------------------------------------------*/
/* Build a template based on current druid settings.                        */
/*--------------------------------------------------------------------------*/
static glTemplate *
build_template (glTemplateDesigner      *dlg)
{
	gchar                *brand, *part_num, *desc;
	gchar                *page_size_name;
	glPaper              *paper;
	glTemplateLabelStyle  shape;
	gdouble               w, h, r, radius, hole, waste;
	gint                  nlayouts;
	gdouble               nx_1, ny_1, x0_1, y0_1, dx_1, dy_1;
	gdouble               nx_2, ny_2, x0_2, y0_2, dx_2, dy_2;
	glTemplate           *template;

	brand    = gtk_editable_get_chars (GTK_EDITABLE(dlg->priv->brand_entry), 0, -1);
	part_num = gtk_editable_get_chars (GTK_EDITABLE(dlg->priv->part_num_entry), 0, -1);
	desc     = gtk_editable_get_chars (GTK_EDITABLE(dlg->priv->description_entry), 0, -1);

	page_size_name =
		gtk_editable_get_chars (GTK_EDITABLE(dlg->priv->pg_size_entry), 0, -1);
	paper = gl_paper_from_name (page_size_name);
	if ( g_strcasecmp (paper->id, "Other") == 0 ) {
		paper->width =
			gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_w_spin))
			/ dlg->priv->units_per_point;
		paper->height =
			gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->pg_h_spin))
			 / dlg->priv->units_per_point;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_rect_radio))) {
		shape = GL_TEMPLATE_STYLE_RECT;
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_h_spin));
		r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_r_spin));
		waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->rect_waste_spin));
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_round_radio))) {
		shape = GL_TEMPLATE_STYLE_ROUND;
		r = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->round_r_spin));
		waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->round_waste_spin));
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_cd_radio))) {
		shape = GL_TEMPLATE_STYLE_CD;
		radius = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_radius_spin));
		hole = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_hole_spin));
		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_w_spin));
		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_h_spin));
		waste = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->cd_waste_spin));
	}

	nlayouts = gtk_spin_button_get_value (GTK_SPIN_BUTTON (dlg->priv->nlayouts_spin));
	nx_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_nx_spin));
	ny_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_ny_spin));
	x0_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_x0_spin));
	y0_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_y0_spin));
	dx_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_dx_spin));
	dy_1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout1_dy_spin));
	nx_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_nx_spin));
	ny_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_ny_spin));
	x0_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_x0_spin));
	y0_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_y0_spin));
	dx_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_dx_spin));
	dy_2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->layout2_dy_spin));

	template = g_new0 (glTemplate, 1);

	template->name = g_strdup_printf ("%s %s", brand, part_num);
	template->alias = g_list_append (template->alias, g_strdup (template->name));
	template->page_size = g_strdup (paper->id);
	template->page_width = paper->width;
	template->page_height = paper->height;
	template->description = g_strdup (desc);

	template->label.style = shape;
	switch (shape) {
	case GL_TEMPLATE_STYLE_RECT:
		template->label.rect.w = w / dlg->priv->units_per_point;
		template->label.rect.h = h / dlg->priv->units_per_point;
		template->label.rect.r = r / dlg->priv->units_per_point;
		template->label.rect.waste = waste / dlg->priv->units_per_point;
		break;
	case GL_TEMPLATE_STYLE_ROUND:
		template->label.round.r = r / dlg->priv->units_per_point;
		template->label.round.waste = waste / dlg->priv->units_per_point;
		break;
	case GL_TEMPLATE_STYLE_CD:
		template->label.cd.r1 = radius / dlg->priv->units_per_point;
		template->label.cd.r2 = hole / dlg->priv->units_per_point;
		template->label.cd.w = w / dlg->priv->units_per_point;
		template->label.cd.h = h / dlg->priv->units_per_point;
		template->label.cd.waste = waste / dlg->priv->units_per_point;
		break;
	}
	template->label.any.layouts =
		g_list_append (template->label.any.layouts,
			       gl_template_layout_new (nx_1, ny_1,
						       x0_1 / dlg->priv->units_per_point,
						       y0_1 / dlg->priv->units_per_point,
						       dx_1 / dlg->priv->units_per_point,
						       dy_1 / dlg->priv->units_per_point));
	if (nlayouts > 1) {
		template->label.any.layouts =
			g_list_append (template->label.any.layouts,
				       gl_template_layout_new (nx_2, ny_2,
							       x0_2 / dlg->priv->units_per_point,
							       y0_2 / dlg->priv->units_per_point,
							       dx_2 / dlg->priv->units_per_point,
							       dy_2 / dlg->priv->units_per_point));
	}

	g_free (brand);
	g_free (part_num);
	g_free (desc);

	g_free (page_size_name);
	gl_paper_free (&paper);

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Handle non-linear forward traversal.                           */
/*--------------------------------------------------------------------------*/
static gboolean
next_cb (GnomeDruidPage     *druidpage,
	 GtkWidget          *widget,
	 glTemplateDesigner *dlg)
{

	if ( GTK_WIDGET(druidpage) == dlg->priv->shape_page ) {

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_rect_radio))) {
			gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
					      GNOME_DRUID_PAGE(dlg->priv->rect_size_page));
			return TRUE;
		}

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_round_radio))) {
			gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
					      GNOME_DRUID_PAGE(dlg->priv->round_size_page));
			return TRUE;
		}

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_cd_radio))) {
			gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
					      GNOME_DRUID_PAGE(dlg->priv->cd_size_page));
			return TRUE;
		}

	}

	if ( GTK_WIDGET(druidpage) == dlg->priv->rect_size_page ) {

		gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
				      GNOME_DRUID_PAGE(dlg->priv->nlayouts_page));
		return TRUE;

	}

	if ( GTK_WIDGET(druidpage) == dlg->priv->round_size_page ) {

		gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
				      GNOME_DRUID_PAGE(dlg->priv->nlayouts_page));
		return TRUE;

	}

	if ( GTK_WIDGET(druidpage) == dlg->priv->cd_size_page ) {

		gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
				      GNOME_DRUID_PAGE(dlg->priv->nlayouts_page));
		return TRUE;

	}

	/* Default case is linear. */
	return FALSE;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Handle non-linear reverse traversal.                           */
/*--------------------------------------------------------------------------*/
static gboolean
back_cb (GnomeDruidPage     *druidpage,
	 GtkWidget          *widget,
	 glTemplateDesigner *dlg)
{

	if ( GTK_WIDGET(druidpage) == dlg->priv->rect_size_page ) {

		gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
				      GNOME_DRUID_PAGE(dlg->priv->shape_page));
		return TRUE;

	}

	if ( GTK_WIDGET(druidpage) == dlg->priv->round_size_page ) {

		gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
				      GNOME_DRUID_PAGE(dlg->priv->shape_page));
		return TRUE;

	}

	if ( GTK_WIDGET(druidpage) == dlg->priv->cd_size_page ) {

		gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
				      GNOME_DRUID_PAGE(dlg->priv->shape_page));
		return TRUE;

	}

	if ( GTK_WIDGET(druidpage) == dlg->priv->nlayouts_page ) {

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_rect_radio))) {
			gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
					      GNOME_DRUID_PAGE(dlg->priv->rect_size_page));
			return TRUE;
		}

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_round_radio))) {
			gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
					      GNOME_DRUID_PAGE(dlg->priv->round_size_page));
			return TRUE;
		}

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dlg->priv->shape_cd_radio))) {
			gnome_druid_set_page (GNOME_DRUID(dlg->priv->druid),
					      GNOME_DRUID_PAGE(dlg->priv->cd_size_page));
			return TRUE;
		}

	}

	/* Default case is linear. */
	return FALSE;
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

