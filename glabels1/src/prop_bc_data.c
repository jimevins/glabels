/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_bc_data.c:  barcode data widget module
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

#include "prop_bc_data.h"
#include "merge.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glPropBCDataSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint prop_bc_data_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_prop_bc_data_class_init (glPropBCDataClass * class);
static void gl_prop_bc_data_init (glPropBCData * bc_data);
static void gl_prop_bc_data_destroy (GtkObject * object);
static void gl_prop_bc_data_construct (glPropBCData * bc_data,
				       gchar * label, GList * field_defs);

static void changed_cb (glPropBCData * bc_data);
static void radio_toggled_cb (GtkToggleButton * togglebutton,
			      glPropBCData * bc_data);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_prop_bc_data_get_type (void)
{
	static guint prop_bc_data_type = 0;

	if (!prop_bc_data_type) {
		GtkTypeInfo prop_bc_data_info = {
			"glPropBCData",
			sizeof (glPropBCData),
			sizeof (glPropBCDataClass),
			(GtkClassInitFunc) gl_prop_bc_data_class_init,
			(GtkObjectInitFunc) gl_prop_bc_data_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		prop_bc_data_type = gtk_type_unique (gtk_vbox_get_type (),
						     &prop_bc_data_info);
	}

	return prop_bc_data_type;
}

static void
gl_prop_bc_data_class_init (glPropBCDataClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_prop_bc_data_destroy;

	prop_bc_data_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glPropBCDataClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class,
				      prop_bc_data_signals, LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_prop_bc_data_init (glPropBCData * bc_data)
{
	bc_data->literal_radio = NULL;
	bc_data->literal_entry = NULL;

	bc_data->key_radio = NULL;
	bc_data->key_entry = NULL;
}

static void
gl_prop_bc_data_destroy (GtkObject * object)
{
	glPropBCData *bc_data;
	glPropBCDataClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROP_BC_DATA (object));

	bc_data = GL_PROP_BC_DATA (object);
	class = GL_PROP_BC_DATA_CLASS (GTK_OBJECT (bc_data)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_prop_bc_data_new (gchar * label,
		     GList * field_defs)
{
	glPropBCData *bc_data;

	bc_data = gtk_type_new (gl_prop_bc_data_get_type ());

	gl_prop_bc_data_construct (bc_data, label, field_defs);

	return GTK_WIDGET (bc_data);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_prop_bc_data_construct (glPropBCData * bc_data,
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
	    gtk_radio_button_group (GTK_RADIO_BUTTON (bc_data->literal_radio));
	gtk_signal_connect (GTK_OBJECT (bc_data->literal_radio), "toggled",
			    GTK_SIGNAL_FUNC (radio_toggled_cb),
			    GTK_OBJECT (bc_data));
	gtk_table_attach_defaults (GTK_TABLE (wtable), bc_data->literal_radio,
				   0, 1, 0, 1);

	/* Literal entry widget */
	bc_data->literal_entry = gtk_entry_new ();
	gtk_widget_set_usize (bc_data->literal_entry, 200, 0);
	gtk_signal_connect_object (GTK_OBJECT (bc_data->literal_entry),
				   "changed", GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (bc_data));
	gtk_table_attach_defaults (GTK_TABLE (wtable), bc_data->literal_entry,
				   1, 2, 0, 1);

	/* Key radio */
	bc_data->key_radio = gtk_radio_button_new_with_label (radio_group,
							      _("Key:"));
	gtk_signal_connect (GTK_OBJECT (bc_data->key_radio), "toggled",
			    GTK_SIGNAL_FUNC (radio_toggled_cb),
			    GTK_OBJECT (bc_data));
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
	gtk_widget_set_usize (wcombo, 200, 0);
	gtk_signal_connect_object (GTK_OBJECT (bc_data->key_entry), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (bc_data));
	gtk_table_attach_defaults (GTK_TABLE (wtable), wcombo, 1, 2, 1, 2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glPropBCData * bc_data)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (bc_data), prop_bc_data_signals[CHANGED]);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback to handle toggling of radio buttons                   */
/*--------------------------------------------------------------------------*/
static void
radio_toggled_cb (GtkToggleButton * togglebutton,
		  glPropBCData * bc_data)
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
	gtk_signal_emit (GTK_OBJECT (bc_data), prop_bc_data_signals[CHANGED]);
}

/*--------------------------------------------------------------------------*/
/* Get widget data.                                                         */
/*--------------------------------------------------------------------------*/
void
gl_prop_bc_data_get_data (glPropBCData * bc_data,
			  gboolean * field_flag,
			  gchar ** data,
			  gchar ** key)
{
	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (bc_data->literal_radio))) {
		*field_flag = FALSE;
		*data =
		    gtk_editable_get_chars (GTK_EDITABLE
					    (bc_data->literal_entry), 0, -1);
		*key = NULL;
	} else {
		*field_flag = TRUE;
		*data = NULL;
		*key =
		    gtk_editable_get_chars (GTK_EDITABLE (bc_data->key_entry),
					    0, -1);
	}
}

/*--------------------------------------------------------------------------*/
/* Set widget data.                                                         */
/*--------------------------------------------------------------------------*/
void
gl_prop_bc_data_set_data (glPropBCData * bc_data,
			  gboolean merge_flag,
			  gboolean field_flag,
			  gchar * data,
			  gchar * key)
{
	gint pos;

	gtk_widget_set_sensitive (bc_data->key_radio, merge_flag);

	if (!field_flag) {

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (bc_data->literal_radio), TRUE);

		gtk_widget_set_sensitive (bc_data->literal_entry, TRUE);
		gtk_widget_set_sensitive (bc_data->key_entry, FALSE);

		gtk_signal_handler_block_by_func (GTK_OBJECT
						  (bc_data->literal_entry),
						  GTK_SIGNAL_FUNC (changed_cb),
						  bc_data);
		gtk_editable_delete_text (GTK_EDITABLE (bc_data->literal_entry),
					  0, -1);
		gtk_signal_handler_unblock_by_func (GTK_OBJECT
						    (bc_data->literal_entry),
						    GTK_SIGNAL_FUNC
						    (changed_cb), bc_data);

		pos = 0;
		gtk_editable_insert_text (GTK_EDITABLE (bc_data->literal_entry),
					  data, strlen (data), &pos);
	} else {

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (bc_data->key_radio), TRUE);

		gtk_widget_set_sensitive (bc_data->literal_entry, FALSE);
		gtk_widget_set_sensitive (bc_data->key_entry, TRUE);

		gtk_signal_handler_block_by_func (GTK_OBJECT
						  (bc_data->key_entry),
						  GTK_SIGNAL_FUNC (changed_cb),
						  bc_data);
		gtk_editable_delete_text (GTK_EDITABLE (bc_data->key_entry), 0,
					  -1);
		gtk_signal_handler_unblock_by_func (GTK_OBJECT
						    (bc_data->key_entry),
						    GTK_SIGNAL_FUNC
						    (changed_cb), bc_data);

		pos = 0;
		gtk_editable_insert_text (GTK_EDITABLE (bc_data->key_entry),
					  data, strlen (data), &pos);
	}

}
