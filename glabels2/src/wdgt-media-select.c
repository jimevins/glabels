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
#include "template.h"
#include "wdgt-mini-preview.h"
#include "prefs.h"
#include "util.h"
#include "marshal.h"

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

static void gl_wdgt_media_select_class_init    (glWdgtMediaSelectClass * class);
static void gl_wdgt_media_select_instance_init (glWdgtMediaSelect * media_select);
static void gl_wdgt_media_select_finalize      (GObject * object);

static void gl_wdgt_media_select_construct     (glWdgtMediaSelect * media_select);

static void page_size_entry_changed_cb         (GtkEntry * entry,
						gpointer user_data);
static void template_entry_changed_cb          (GtkEntry * entry,
						gpointer user_data);

static void details_update                     (glWdgtMediaSelect * media_select,
						gchar * name);

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
			g_type_register_static (gtk_vbox_get_type (),
						"glWdgtMediaSelect",
						&wdgt_media_select_info, 0);
	}

	return wdgt_media_select_type;
}

static void
gl_wdgt_media_select_class_init (glWdgtMediaSelectClass * class)
{
	GObjectClass *object_class;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

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
gl_wdgt_media_select_instance_init (glWdgtMediaSelect * media_select)
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
gl_wdgt_media_select_finalize (GObject * object)
{
	glWdgtMediaSelect *media_select;
	glWdgtMediaSelectClass *class;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_MEDIA_SELECT (object));

	media_select = GL_WDGT_MEDIA_SELECT (object);

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
gl_wdgt_media_select_construct (glWdgtMediaSelect * media_select)
{
	GtkWidget *whbox, *wvbox, *wcombo, *wvbox1, *whbox1;
	gchar *name;
	GList *template_names, *page_sizes = NULL;
	const gchar *page_size;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	page_size = gl_prefs_get_page_size ();

	wvbox = GTK_WIDGET (media_select);
	gtk_box_set_spacing (GTK_BOX(wvbox), GL_HIG_SPACING);

	whbox = gtk_hbox_new (FALSE, GL_HIG_SPACING);
	gtk_box_pack_start (GTK_BOX (wvbox), whbox, TRUE, TRUE, 0);

	/* Page size selection control */
	gl_debug (DEBUG_MEDIA_SELECT, "Creating page size combo...");
	wcombo = gtk_combo_new ();
	page_sizes = gl_template_get_page_size_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), page_sizes);
	gl_template_free_page_size_list (&page_sizes);
	media_select->page_size_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (media_select->page_size_entry),
				FALSE);
	gtk_combo_set_value_in_list (GTK_COMBO(wcombo), TRUE, FALSE);
	gtk_widget_set_size_request (media_select->page_size_entry, 100, -1);
	gtk_entry_set_text (GTK_ENTRY (media_select->page_size_entry),
			    page_size);
	gtk_box_pack_start (GTK_BOX (whbox), wcombo, FALSE, FALSE, 0);

	/* Actual selection control */
	gl_debug (DEBUG_MEDIA_SELECT, "Creating template combo...");
	gl_debug (DEBUG_MEDIA_SELECT, "page_size = %s", page_size);
	template_names = gl_template_get_name_list (page_size);
	media_select->template_combo = gtk_combo_new ();
	gtk_combo_set_popdown_strings (GTK_COMBO (media_select->template_combo),
				       template_names);
	gl_template_free_name_list (&template_names);
	media_select->template_entry =
	    GTK_COMBO (media_select->template_combo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (media_select->template_entry),
				FALSE);
	gtk_combo_set_value_in_list (GTK_COMBO(media_select->template_combo),
				     TRUE, FALSE);
	gtk_widget_set_size_request (media_select->template_entry, 400, -1);
	gtk_box_pack_start (GTK_BOX (whbox), media_select->template_combo,
			    FALSE, FALSE, 0);

	whbox = gtk_hbox_new (FALSE, GL_HIG_SPACING);
	gtk_box_pack_start (GTK_BOX (wvbox), whbox, TRUE, TRUE, 0);

	/* mini_preview canvas */
	gl_debug (DEBUG_MEDIA_SELECT, "Creating mini preview...");
	media_select->mini_preview = gl_wdgt_mini_preview_new ( WDGT_MINI_PREVIEW_HEIGHT,
							   WDGT_MINI_PREVIEW_WIDTH);
	gtk_box_pack_start (GTK_BOX (whbox), media_select->mini_preview,
			    FALSE, FALSE, 0);

	/* Label column */
	gl_debug (DEBUG_MEDIA_SELECT, "Creating label column...");
	wvbox1 = gtk_vbox_new (FALSE, GL_HIG_SPACING);
	gtk_box_pack_start (GTK_BOX (whbox), wvbox1, FALSE, FALSE, 0);

	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Description:")),
			    FALSE, FALSE, 0);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Page size:")),
			    FALSE, FALSE, 0);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Label size:")),
			    FALSE, FALSE, 0);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Layout:")),
			    FALSE, FALSE, 0);

	/* detail widgets column */
	gl_debug (DEBUG_MEDIA_SELECT, "Creating details column...");
	wvbox1 = gtk_vbox_new (FALSE, GL_HIG_SPACING);
	gtk_box_pack_start (GTK_BOX (whbox), wvbox1, FALSE, FALSE, 0);

	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	media_select->desc_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (whbox1), media_select->desc_label,
			    FALSE, FALSE, 0);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	media_select->sheet_size_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (whbox1), media_select->sheet_size_label,
			    FALSE, FALSE, 0);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	media_select->label_size_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (whbox1), media_select->label_size_label,
			    FALSE, FALSE, 0);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	media_select->number_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (whbox1), media_select->number_label,
			    FALSE, FALSE, 0);

	/* Update mini_preview and details from default template */
	
	gl_debug (DEBUG_MEDIA_SELECT, "template_entry = %p",
		  media_select->template_entry);
	name =
	    gtk_editable_get_chars (GTK_EDITABLE (media_select->template_entry),
				    0, -1);
	gl_debug (DEBUG_MEDIA_SELECT, "name = \"%s\"", name);
	gl_wdgt_mini_preview_set_label (GL_WDGT_MINI_PREVIEW (media_select->mini_preview),
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

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
page_size_entry_changed_cb (GtkEntry * entry,
			    gpointer user_data)
{
	glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (user_data);
	gchar *page_size;
	GList *template_names;

	gl_debug (DEBUG_MEDIA_SELECT, "START");


	/* Update template selections for new page size */
	page_size = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if ( strlen(page_size) ) {
		gl_debug (DEBUG_MEDIA_SELECT, "page_size = \"%s\"", page_size);
		template_names = gl_template_get_name_list (page_size);
		gtk_combo_set_popdown_strings (GTK_COMBO (media_select->template_combo),
					       template_names);
		gl_template_free_name_list (&template_names);
	}
	g_free (page_size);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
template_entry_changed_cb (GtkEntry * entry,
			   gpointer user_data)
{
	glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (user_data);
	gchar *name;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

	/* Update mini_preview canvas & details with template */
	gl_debug (DEBUG_MEDIA_SELECT, "template_entry = %p", entry);
	name = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if ( strlen(name) ) {
		gl_debug (DEBUG_MEDIA_SELECT, "name = \"%s\"", name);
		gl_wdgt_mini_preview_set_label (GL_WDGT_MINI_PREVIEW (media_select->mini_preview),
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
/* PRIVATE. update "details" widgets from new template.               */
/*--------------------------------------------------------------------------*/
static void
details_update (glWdgtMediaSelect * media_select,
		gchar * name)
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

	text = gl_template_get_layout_desc (template);
	gtk_label_set_text (GTK_LABEL (media_select->number_label), text);
	g_free (text);

	text = gl_template_get_label_size_desc (template);
	gtk_label_set_text (GTK_LABEL (media_select->label_size_label), text);
	g_free (text);

	gl_template_free( &template );

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/****************************************************************************/
/* query selected label template name.                                      */
/****************************************************************************/
gchar *
gl_wdgt_media_select_get_name (glWdgtMediaSelect * media_select)
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
gl_wdgt_media_select_set_name (glWdgtMediaSelect * media_select,
			       gchar * name)
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
gl_wdgt_media_select_get_page_size (glWdgtMediaSelect * media_select)
{
	gl_debug (DEBUG_MEDIA_SELECT, "");

	return
	    gtk_editable_get_chars (GTK_EDITABLE
				    (media_select->page_size_entry), 0, -1);
}

/****************************************************************************/
/* set selected label template page size.                                   */
/****************************************************************************/
void
gl_wdgt_media_select_set_page_size (glWdgtMediaSelect * media_select,
				    gchar * page_size)
{
	gint pos;

	gl_debug (DEBUG_MEDIA_SELECT, "START");

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
				  page_size, strlen (page_size), &pos);

	gl_debug (DEBUG_MEDIA_SELECT, "END");
}
