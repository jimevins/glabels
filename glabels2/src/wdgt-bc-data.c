/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_bc_data.c:  barcode data widget module
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

#include "wdgt-bc-data.h"
#include "merge.h"
#include "marshal.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtBCDataSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint wdgt_bc_data_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_bc_data_class_init (glWdgtBCDataClass * class);
static void gl_wdgt_bc_data_instance_init (glWdgtBCData * bc_data);
static void gl_wdgt_bc_data_finalize (GObject * object);
static void gl_wdgt_bc_data_construct (glWdgtBCData * bc_data,
				       gchar * label, GList * field_defs);

static void changed_cb (glWdgtBCData * bc_data);
static void radio_toggled_cb (GtkToggleButton * togglebutton,
			      glWdgtBCData * bc_data);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_wdgt_bc_data_get_type (void)
{
	static guint wdgt_bc_data_type = 0;

	if (!wdgt_bc_data_type) {
		GTypeInfo wdgt_bc_data_info = {
			sizeof (glWdgtBCDataClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_bc_data_class_init,
			NULL,
			NULL,
			sizeof (glWdgtBCData),
			0,
			(GInstanceInitFunc) gl_wdgt_bc_data_instance_init,
		};

		wdgt_bc_data_type =
			g_type_register_static (gtk_vbox_get_type (),
						"glWdgtBCData",
						&wdgt_bc_data_info, 0);
	}

	return wdgt_bc_data_type;
}

static void
gl_wdgt_bc_data_class_init (glWdgtBCDataClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->finalize = gl_wdgt_bc_data_finalize;

	wdgt_bc_data_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtBCDataClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_bc_data_instance_init (glWdgtBCData * bc_data)
{
	bc_data->literal_radio = NULL;
	bc_data->literal_entry = NULL;

	bc_data->key_radio = NULL;
	bc_data->key_entry = NULL;
}

static void
gl_wdgt_bc_data_finalize (GObject * object)
{
	glWdgtBCData *bc_data;
	glWdgtBCDataClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_BC_DATA (object));

	bc_data = GL_WDGT_BC_DATA (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_bc_data_new (gchar * label,
		     GList * field_defs)
{
	glWdgtBCData *bc_data;

	bc_data = g_object_new (gl_wdgt_bc_data_get_type (), NULL);

	gl_wdgt_bc_data_construct (bc_data, label, field_defs);

	return GTK_WIDGET (bc_data);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_wdgt_bc_data_construct (glWdgtBCData * bc_data,
			   gchar * label,
			   GList * field_defs)
{
	GtkWidget *wvbox, *wframe, *wtable, *wcombo;
	GSList *radio_group = NULL;
	GList *keys;

	wvbox = GTK_WIDGET (bc_data);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wtable = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_container_add (GTK_CONTAINER (wframe), wtable);

	/* Literal radio */
	bc_data->literal_radio = gtk_radio_button_new_with_label (radio_group,
								  _
								  ("Literal:"));
	radio_group =
	    gtk_radio_button_get_group (GTK_RADIO_BUTTON (bc_data->literal_radio));
	g_signal_connect (G_OBJECT (bc_data->literal_radio), "toggled",
			  G_CALLBACK (radio_toggled_cb),
			  G_OBJECT (bc_data));
	gtk_table_attach_defaults (GTK_TABLE (wtable), bc_data->literal_radio,
				   0, 1, 0, 1);

	/* Literal entry widget */
	bc_data->literal_entry = gtk_entry_new ();
	gtk_widget_set_size_request (bc_data->literal_entry, 200, -1);
	g_signal_connect_swapped (G_OBJECT (bc_data->literal_entry),
				  "changed", G_CALLBACK (changed_cb),
				  G_OBJECT (bc_data));
	gtk_table_attach_defaults (GTK_TABLE (wtable), bc_data->literal_entry,
				   1, 2, 0, 1);

	/* Key radio */
	bc_data->key_radio = gtk_radio_button_new_with_label (radio_group,
							      _("Key:"));
	g_signal_connect (G_OBJECT (bc_data->key_radio), "toggled",
			  G_CALLBACK (radio_toggled_cb),
			  G_OBJECT (bc_data));
	gtk_table_attach_defaults (GTK_TABLE (wtable), bc_data->key_radio,
				   0, 1, 1, 2);

	/* Key entry widget */
	wcombo = gtk_combo_new ();
	keys = gl_merge_get_key_list (field_defs);
	if (keys != NULL)
		gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), keys);
	gl_merge_free_key_list (&keys);
	bc_data->key_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (bc_data->key_entry), FALSE);
	gtk_widget_set_size_request (wcombo, 200, -1);
	g_signal_connect_swapped (G_OBJECT (bc_data->key_entry), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (bc_data));
	gtk_table_attach_defaults (GTK_TABLE (wtable), wcombo, 1, 2, 1, 2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtBCData * bc_data)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (bc_data), wdgt_bc_data_signals[CHANGED], 0);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback to handle toggling of radio buttons                   */
/*--------------------------------------------------------------------------*/
static void
radio_toggled_cb (GtkToggleButton * togglebutton,
		  glWdgtBCData * bc_data)
{
	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (bc_data->literal_radio))) {
		gtk_widget_set_sensitive (bc_data->literal_entry, TRUE);
		gtk_widget_set_sensitive (bc_data->key_entry, FALSE);
	} else {
		gtk_widget_set_sensitive (bc_data->literal_entry, FALSE);
		gtk_widget_set_sensitive (bc_data->key_entry, TRUE);
	}

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (bc_data), wdgt_bc_data_signals[CHANGED], 0);
}

/*--------------------------------------------------------------------------*/
/* Get widget data.                                                         */
/*--------------------------------------------------------------------------*/
glTextNode *
gl_wdgt_bc_data_get_data (glWdgtBCData * bc_data)
{
	glTextNode *text_node;

	text_node = g_new0(glTextNode,1);

	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (bc_data->literal_radio))) {
		text_node->field_flag = FALSE;
		text_node->data =
		    gtk_editable_get_chars (GTK_EDITABLE
					    (bc_data->literal_entry), 0, -1);
	} else {
		text_node->field_flag = TRUE;
		text_node->data = 
		    gtk_editable_get_chars (GTK_EDITABLE (bc_data->key_entry),
					    0, -1);
	}
}

/*--------------------------------------------------------------------------*/
/* Set widget data.                                                         */
/*--------------------------------------------------------------------------*/
void
gl_wdgt_bc_data_set_data (glWdgtBCData * bc_data,
			  gboolean merge_flag,
			  glTextNode *text_node)
{
	gint pos;

	gtk_widget_set_sensitive (bc_data->key_radio, merge_flag);

	if (!text_node->field_flag) {

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (bc_data->literal_radio), TRUE);

		gtk_widget_set_sensitive (bc_data->literal_entry, TRUE);
		gtk_widget_set_sensitive (bc_data->key_entry, FALSE);

		g_signal_handlers_block_by_func (G_OBJECT
						(bc_data->literal_entry),
						G_CALLBACK (changed_cb),
						bc_data);
		gtk_editable_delete_text (GTK_EDITABLE (bc_data->literal_entry),
					  0, -1);
		g_signal_handlers_unblock_by_func (G_OBJECT
						  (bc_data->literal_entry),
						  G_CALLBACK
						  (changed_cb), bc_data);

		pos = 0;
		gtk_editable_insert_text (GTK_EDITABLE (bc_data->literal_entry),
					  text_node->data,
					  strlen (text_node->data),
					  &pos);
	} else {

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (bc_data->key_radio), TRUE);

		gtk_widget_set_sensitive (bc_data->literal_entry, FALSE);
		gtk_widget_set_sensitive (bc_data->key_entry, TRUE);

		g_signal_handlers_block_by_func (G_OBJECT
						(bc_data->key_entry),
						G_CALLBACK (changed_cb),
						bc_data);
		gtk_editable_delete_text (GTK_EDITABLE (bc_data->key_entry), 0,
					  -1);
		g_signal_handlers_unblock_by_func (G_OBJECT
						  (bc_data->key_entry),
						  G_CALLBACK
						  (changed_cb), bc_data);

		pos = 0;
		gtk_editable_insert_text (GTK_EDITABLE (bc_data->key_entry),
					  text_node->data,
					  strlen (text_node->data),
					  &pos);
	}

}
