/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_media_select.c:  media selection widget module
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

#include <config.h>

#include "wdgt-media-select.h"
#include "hig.h"
#include "wdgt-mini-preview.h"
#include "prefs.h"
#include "util.h"
#include "marshal.h"
#include <libglabels/paper.h>
#include <libglabels/template.h>

#include "debug.h"

#define WDGT_MINI_PREVIEW_WIDTH  160
#define WDGT_MINI_PREVIEW_HEIGHT 200

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtMediaSelectSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GObjectClass *parent_class;

static gint wdgt_media_select_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_media_select_class_init    (glWdgtMediaSelectClass *class);
static void gl_wdgt_media_select_instance_init (glWdgtMediaSelect      *media_select);
static void gl_wdgt_media_select_finalize      (GObject                *object);

static void gl_wdgt_media_select_construct     (glWdgtMediaSelect      *media_select);

static void page_size_entry_changed_cb         (GtkEntry               *entry,
						gpointer                user_data);
static void template_entry_changed_cb          (GtkEntry               *entry,
						gpointer                user_data);
static void prefs_changed_cb                   (glPrefsModel           *gl_prefs,
						gpointer                user_data);

static void details_update                     (glWdgtMediaSelect      *media_select,
						gchar                  *name);

static gchar *get_layout_desc                  (const glTemplate       *template);
static gchar *get_label_size_desc              (const glTemplate       *template);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_wdgt_media_select_get_type (void)
{
	static guint wdgt_media_select_type = 0;

	if (!wdgt_media_select_type) {
		GTypeInfo wdgt_media_select_info = {
			sizeof (glWdgtMediaSelectClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_media_select_class_init,
			NULL,
			NULL,
			sizeof (glWdgtMediaSelect),
			0,
			(GInstanceInitFunc) gl_wdgt_media_select_instance_init,
		};

		wdgt_media_select_type =
			g_type_register_static (gl_hig_vbox_get_type (),
						"glWdgtMediaSelect",
						&wdgt_media_select_info, 0);
	}

	return wdgt_media_select_type;
}

static void
gl_wdgt_media_select_class_init (glWdgtMediaSelectClass *class)
{
	GObjectClass *object_class;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_wdgt_media_select_finalize;

	wdgt_media_select_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtMediaSelectClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

static void
gl_wdgt_media_select_instance_init (glWdgtMediaSelect *media_select)
{
	gl_debug (DEBUG_MEDIA_SELECT, "START");

	media_select->page_size_entry = NULL;
	media_select->template_entry = NULL;

	media_select->mini_preview = NULL;

	media_select->desc_label = NULL;
	media_select->sheet_size_label = NULL;
	media_select->number_label = NULL;
	media_select->label_size_label = NULL;

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

static void
gl_wdgt_media_select_finalize (GObject *object)
{
	glWdgtMediaSelect *media_select;
	glWdgtMediaSelectClass *class;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_MEDIA_SELECT (object));

	media_select = GL_WDGT_MEDIA_SELECT (object);

	g_signal_handlers_disconnect_by_func (G_OBJECT(gl_prefs),
					      prefs_changed_cb, media_select);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

GtkWidget *
gl_wdgt_media_select_new (void)
{
	glWdgtMediaSelect *media_select;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	media_select = g_object_new (gl_wdgt_media_select_get_type (), NULL);

	gl_wdgt_media_select_construct (media_select);

	gl_debug (DEBUG_MEDIA_SELECT, "END");

	return GTK_WIDGET (media_select);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_media_select_construct (glWdgtMediaSelect *media_select)
{
	GtkWidget    *whbox, *wvbox, *wcombo, *wvbox1, *whbox1, *wlabel;
	GtkSizeGroup *label_size_group;
	gchar        *name;
	GList        *template_names, *page_sizes = NULL;
	const gchar  *page_size_id;
	gchar        *page_size_name;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	page_size_id = gl_prefs_get_page_size ();
	page_size_name = gl_paper_lookup_name_from_id (page_size_id);

	wvbox = GTK_WIDGET (media_select);

	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Page size selection control */
	gl_debug (DEBUG_MEDIA_SELECT, "Creating page size combo...");
	wcombo = gtk_combo_new ();
	page_sizes = gl_paper_get_name_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), page_sizes);
	gl_paper_free_name_list (page_sizes);
	media_select->page_size_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (media_select->page_size_entry),
				FALSE);
	gtk_combo_set_value_in_list (GTK_COMBO(wcombo), TRUE, FALSE);
	gtk_widget_set_size_request (media_select->page_size_entry, 100, -1);
	gtk_entry_set_text (GTK_ENTRY (media_select->page_size_entry),
			    page_size_name);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wcombo);

	/* Actual selection control */
	gl_debug (DEBUG_MEDIA_SELECT, "Creating template combo...");
	gl_debug (DEBUG_MEDIA_SELECT, "page_size_name = %s", page_size_name);
	template_names = gl_template_get_name_list (page_size_id);
	media_select->template_combo = gtk_combo_new ();
	gtk_combo_set_popdown_strings (GTK_COMBO (media_select->template_combo),
				       template_names);
	media_select->template_entry =
	    GTK_COMBO (media_select->template_combo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (media_select->template_entry),
				FALSE);
	gtk_combo_set_value_in_list (GTK_COMBO(media_select->template_combo),
				     TRUE, FALSE);
	gtk_widget_set_size_request (media_select->template_entry, 400, -1);
	gtk_entry_set_text (GTK_ENTRY (media_select->template_entry),
			    template_names->data);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox),
				media_select->template_combo);
	gl_template_free_name_list (template_names);

	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* mini_preview canvas */
	gl_debug (DEBUG_MEDIA_SELECT, "Creating mini preview...");
	media_select->mini_preview = gl_wdgt_mini_preview_new ( WDGT_MINI_PREVIEW_HEIGHT,
							   WDGT_MINI_PREVIEW_WIDTH);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox),
				media_select->mini_preview);

	/* ---- Information area ---- */
	wvbox1 = gl_hig_vbox_new (GL_HIG_VBOX_INNER);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wvbox1);
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	/* blank line */
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox1), gtk_label_new (""));

	/* Description line */
	whbox1 = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox1), whbox1);
	wlabel = gtk_label_new (_("Description:"));
	gtk_size_group_add_widget (label_size_group, wlabel);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.0);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), wlabel);
	media_select->desc_label = gtk_label_new ("");
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), media_select->desc_label);

	/* Page size line */
	whbox1 = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox1), whbox1);
	wlabel = gtk_label_new (_("Page size:"));
	gtk_size_group_add_widget (label_size_group, wlabel);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.0);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), wlabel);
	media_select->sheet_size_label = gtk_label_new ("");
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1),
				media_select->sheet_size_label);

	/* Label size line */
	whbox1 = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox1), whbox1);
	wlabel = gtk_label_new (_("Label size:"));
	gtk_size_group_add_widget (label_size_group, wlabel);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.0);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), wlabel);
	media_select->label_size_label = gtk_label_new ("");
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1),
				media_select->label_size_label);

	/* Layout line */
	whbox1 = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox1), whbox1);
	wlabel = gtk_label_new (_("Layout:"));
	gtk_size_group_add_widget (label_size_group, wlabel);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.0);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), wlabel);
	media_select->number_label = gtk_label_new ("");
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1),
				media_select->number_label);

	/* Update mini_preview and details from default template */
	
	gl_debug (DEBUG_MEDIA_SELECT, "template_entry = %p",
		  media_select->template_entry);
	name =
	    gtk_editable_get_chars (GTK_EDITABLE (media_select->template_entry),
				    0, -1);
	gl_debug (DEBUG_MEDIA_SELECT, "name = \"%s\"", name);
	gl_wdgt_mini_preview_set_label_by_name (GL_WDGT_MINI_PREVIEW (media_select->mini_preview),
						name);
	details_update (media_select, name);
	g_free (name);

	/* Connect signals to controls */
	g_signal_connect (G_OBJECT (media_select->page_size_entry), "changed",
			  G_CALLBACK (page_size_entry_changed_cb),
			  media_select);
	g_signal_connect (G_OBJECT (media_select->template_entry), "changed",
			  G_CALLBACK (template_entry_changed_cb),
			  media_select);
	g_signal_connect (G_OBJECT (gl_prefs), "changed",
			  G_CALLBACK (prefs_changed_cb),
			  media_select);

	g_free (page_size_name);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
page_size_entry_changed_cb (GtkEntry *entry,
			    gpointer  user_data)
{
	glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (user_data);
	gchar             *page_size_name, *page_size_id;
	GList             *template_names;

	gl_debug (DEBUG_MEDIA_SELECT, "START");


	/* Update template selections for new page size */
	page_size_name = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if ( strlen(page_size_name) ) {
		gl_debug (DEBUG_MEDIA_SELECT, "page_size_name = \"%s\"", page_size_name);
		page_size_id = gl_paper_lookup_id_from_name (page_size_name);
		template_names = gl_template_get_name_list (page_size_id);
		if (template_names == NULL) {
			template_names = g_list_append (template_names, g_strdup(""));
		}
		gtk_combo_set_popdown_strings (GTK_COMBO (media_select->template_combo),
					       template_names);
		gtk_entry_set_text (GTK_ENTRY (media_select->template_entry),
				    template_names->data);
		gl_template_free_name_list (template_names);
		g_free (page_size_id);
	}
	g_free (page_size_name);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
template_entry_changed_cb (GtkEntry *entry,
			   gpointer  user_data)
{
	glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (user_data);
	gchar *name;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	/* Update mini_preview canvas & details with template */
	gl_debug (DEBUG_MEDIA_SELECT, "template_entry = %p", entry);
	name = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if ( strlen(name) ) {
		gl_debug (DEBUG_MEDIA_SELECT, "name = \"%s\"", name);
		gl_wdgt_mini_preview_set_label_by_name (GL_WDGT_MINI_PREVIEW (media_select->mini_preview),
							name);
		gl_debug (DEBUG_MEDIA_SELECT, "m1");
		details_update (media_select, name);
		gl_debug (DEBUG_MEDIA_SELECT, "m2");

		/* Emit our "changed" signal */
		g_signal_emit (G_OBJECT (user_data),
			       wdgt_media_select_signals[CHANGED], 0);
	}
	g_free (name);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in prefs                           */
/*--------------------------------------------------------------------------*/
static void
prefs_changed_cb (glPrefsModel *gl_prefs,
		  gpointer      user_data)
{
	glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (user_data);
	gchar *name;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	/* Update mini_preview canvas & details with template */
	name = gtk_editable_get_chars (GTK_EDITABLE (media_select->template_entry), 0, -1);
	if ( strlen(name) ) {
		gl_debug (DEBUG_MEDIA_SELECT, "name = \"%s\"", name);
		details_update (media_select, name);
	}
	g_free (name);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. update "details" widgets from new template.               */
/*--------------------------------------------------------------------------*/
static void
details_update (glWdgtMediaSelect *media_select,
		gchar             *name)
{
	glTemplate *template;
	gchar *text;
	glPrefsUnits units;
	const gchar *units_string;
	gdouble units_per_point;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	units = gl_prefs_get_units ();
	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();

	/* Fetch template */
	template = gl_template_from_name (name);

	gtk_label_set_text (GTK_LABEL (media_select->desc_label),
			    template->description);

	gtk_label_set_text (GTK_LABEL (media_select->sheet_size_label),
			    template->page_size);

	text = get_layout_desc (template);
	gtk_label_set_text (GTK_LABEL (media_select->number_label), text);
	g_free (text);

	text = get_label_size_desc (template);
	gtk_label_set_text (GTK_LABEL (media_select->label_size_label), text);
	g_free (text);

	gl_template_free (template);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/****************************************************************************/
/* query selected label template name.                                      */
/****************************************************************************/
gchar *
gl_wdgt_media_select_get_name (glWdgtMediaSelect *media_select)
{
	gl_debug (DEBUG_MEDIA_SELECT, "");
	return
	    gtk_editable_get_chars (GTK_EDITABLE (media_select->template_entry),
				    0, -1);
}

/****************************************************************************/
/* set selected label template name.                                        */
/****************************************************************************/
void
gl_wdgt_media_select_set_name (glWdgtMediaSelect *media_select,
			       gchar             *name)
{
	gint pos;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	g_signal_handlers_block_by_func (G_OBJECT(media_select->template_entry),
					G_CALLBACK(template_entry_changed_cb),
					media_select);
	gtk_editable_delete_text (GTK_EDITABLE (media_select->template_entry),
				  0, -1);
	g_signal_handlers_unblock_by_func (G_OBJECT(media_select->template_entry),
					   G_CALLBACK(template_entry_changed_cb),
					   media_select);

	pos = 0;
	gtk_editable_insert_text (GTK_EDITABLE (media_select->template_entry),
				  name, strlen (name), &pos);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/****************************************************************************/
/* query selected label template page size.                                 */
/****************************************************************************/
gchar *
gl_wdgt_media_select_get_page_size (glWdgtMediaSelect *media_select)
{
	gchar *page_size_name, *page_size_id;

	gl_debug (DEBUG_MEDIA_SELECT, "");

	page_size_name =
	    gtk_editable_get_chars (GTK_EDITABLE
				    (media_select->page_size_entry), 0, -1);

	page_size_id = gl_paper_lookup_id_from_name (page_size_name);

	g_free (page_size_name);

	return page_size_id;
}

/****************************************************************************/
/* set selected label template page size.                                   */
/****************************************************************************/
void
gl_wdgt_media_select_set_page_size (glWdgtMediaSelect *media_select,
				    gchar             *page_size_id)
{
	gint   pos;
	gchar *page_size_name;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	page_size_name = gl_paper_lookup_name_from_id (page_size_id);

	g_signal_handlers_block_by_func (G_OBJECT(media_select->page_size_entry),
					G_CALLBACK(page_size_entry_changed_cb),
					media_select);
	gtk_editable_delete_text (GTK_EDITABLE (media_select->page_size_entry),
				  0, -1);
	g_signal_handlers_unblock_by_func (G_OBJECT(media_select->page_size_entry),
					  G_CALLBACK(page_size_entry_changed_cb),
					  media_select);

	pos = 0;
	gtk_editable_insert_text (GTK_EDITABLE (media_select->page_size_entry),
				  page_size_name, strlen (page_size_name), &pos);

	g_free (page_size_name);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Get a description of the layout and number of labels.          */
/*--------------------------------------------------------------------------*/
static gchar *
get_layout_desc (const glTemplate *template)
{
	const glTemplateLabelType *label_type;
	gint                       n_labels;
	glTemplateLayout          *layout;
	gchar                     *string;

	label_type = gl_template_get_first_label_type (template);

	n_labels = gl_template_get_n_labels (label_type);

	if ( label_type->layouts->next == NULL ) {
		layout = (glTemplateLayout *)label_type->layouts->data;
		string = g_strdup_printf (_("%d x %d  (%d per sheet)"),
					  layout->nx, layout->ny,
					  n_labels);
	} else {
		string = g_strdup_printf (_("%d per sheet"),
					  n_labels);
	}

	return string;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Get label size description.                                    */ 
/*--------------------------------------------------------------------------*/
static gchar *
get_label_size_desc (const glTemplate *template)
{
	glPrefsUnits               units;
	const gchar               *units_string;
	gdouble                    units_per_point;
	const glTemplateLabelType *label_type;
	gchar                     *string = NULL;

	units           = gl_prefs_get_units ();
	units_string    = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();

	label_type = gl_template_get_first_label_type (template);

	switch (label_type->shape) {
	case GL_TEMPLATE_SHAPE_RECT:
		if ( units == GL_UNITS_INCHES ) {
			gchar *xstr, *ystr;

			xstr = gl_util_fraction (label_type->size.rect.w*units_per_point);
			ystr = gl_util_fraction (label_type->size.rect.h*units_per_point);
			string = g_strdup_printf (_("%s x %s %s"),
						  xstr, ystr, units_string);
			g_free (xstr);
			g_free (ystr);
		} else {
			string = g_strdup_printf (_("%.5g x %.5g %s"),
						  label_type->size.rect.w*units_per_point,
						  label_type->size.rect.h*units_per_point,
						  units_string);
		}
		break;
	case GL_TEMPLATE_SHAPE_ROUND:
		if ( units == GL_UNITS_INCHES ) {
			gchar *dstr;

			dstr = gl_util_fraction (2.0*label_type->size.round.r*units_per_point);
			string = g_strdup_printf (_("%s %s diameter"),
						  dstr, units_string);
			g_free (dstr);
		} else {
			string = g_strdup_printf (_("%.5g %s diameter"),
						  2.0*label_type->size.round.r*units_per_point,
						  units_string);
		}
		break;
	case GL_TEMPLATE_SHAPE_CD:
		if ( units == GL_UNITS_INCHES ) {
			gchar *dstr;

			dstr = gl_util_fraction (2.0*label_type->size.cd.r1*units_per_point);
			string = g_strdup_printf (_("%s %s diameter"),
						  dstr, units_string);
			g_free (dstr);
		} else {
			string = g_strdup_printf (_("%.5g %s diameter"),
						  2.0*label_type->size.cd.r1*units_per_point,
						  units_string);
		}
		break;
	default:
		break;
	}

	return string;
}

