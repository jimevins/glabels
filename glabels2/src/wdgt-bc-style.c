/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_bc_style.c:  barcode style selection widget module
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

#include "wdgt-bc-style.h"
#include "marshal.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtBCStyleSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static glHigVBoxClass *parent_class;

static gint wdgt_bc_style_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_bc_style_class_init    (glWdgtBCStyleClass *class);
static void gl_wdgt_bc_style_instance_init (glWdgtBCStyle      *bc_style);
static void gl_wdgt_bc_style_finalize      (GObject            *object);
static void gl_wdgt_bc_style_construct     (glWdgtBCStyle      *bc_style);

static void changed_cb                     (glWdgtBCStyle      *bc_style);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_wdgt_bc_style_get_type (void)
{
	static guint wdgt_bc_style_type = 0;

	if (!wdgt_bc_style_type) {
		GTypeInfo wdgt_bc_style_info = {
			sizeof (glWdgtBCStyleClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_bc_style_class_init,
			NULL,
			NULL,
			sizeof (glWdgtBCStyle),
			0,
			(GInstanceInitFunc) gl_wdgt_bc_style_instance_init,
		};

		wdgt_bc_style_type =
			g_type_register_static (gl_hig_vbox_get_type (),
						"glWdgtBCStyle",
						&wdgt_bc_style_info, 0);
	}

	return wdgt_bc_style_type;
}

static void
gl_wdgt_bc_style_class_init (glWdgtBCStyleClass *class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_wdgt_bc_style_finalize;

	wdgt_bc_style_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtBCStyleClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_bc_style_instance_init (glWdgtBCStyle *bc_style)
{
	bc_style->style_label  = NULL;
	bc_style->style_entry  = NULL;
	bc_style->text_check   = NULL;
}

static void
gl_wdgt_bc_style_finalize (GObject *object)
{
	glWdgtBCStyle *bc_style;
	glWdgtBCStyleClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_BC_STYLE (object));

	bc_style = GL_WDGT_BC_STYLE (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/****************************************************************************/
/* New widget.                                                              */
/****************************************************************************/
GtkWidget *
gl_wdgt_bc_style_new (void)
{
	glWdgtBCStyle *bc_style;

	bc_style = g_object_new (gl_wdgt_bc_style_get_type (), NULL);

	gl_wdgt_bc_style_construct (bc_style);

	return GTK_WIDGET (bc_style);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_bc_style_construct (glWdgtBCStyle *bc_style)
{
	GtkWidget *wvbox, *whbox, *wcombo;
	GList *style_list;

	wvbox = GTK_WIDGET (bc_style);

	/* ---- Style line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Style Label */
	bc_style->style_label = gtk_label_new (_("Style:"));
	gtk_misc_set_alignment (GTK_MISC (bc_style->style_label), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), bc_style->style_label);

	/* Style entry widget */
	wcombo = gtk_combo_new ();
	style_list = NULL;
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_POSTNET));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_EAN));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_UPC));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_ISBN));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_39));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_128));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_128B));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_128C));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_I25));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_CBR));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_MSI));
	style_list = g_list_append (style_list,
				    (gchar *)gl_barcode_style_to_text(GL_BARCODE_STYLE_PLS));
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), style_list);
	g_list_free (style_list);
	bc_style->style_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (bc_style->style_entry), FALSE);
	gtk_widget_set_size_request (wcombo, 200, -1);
	g_signal_connect_swapped (G_OBJECT (bc_style->style_entry), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (bc_style));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wcombo);


	/* Text checkbox widget */
	bc_style->text_check =
	    gtk_check_button_new_with_label (_("Show text with barcode"));
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), bc_style->text_check);
	g_signal_connect_swapped (G_OBJECT (bc_style->text_check), "toggled",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (bc_style));

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtBCStyle *bc_style)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (bc_style),
		       wdgt_bc_style_signals[CHANGED], 0);
}

/****************************************************************************/
/* query values from controls.                                              */
/****************************************************************************/
void
gl_wdgt_bc_style_get_params (glWdgtBCStyle  *bc_style,
			     glBarcodeStyle *style,
			     gboolean       *text_flag)
{
	gchar *style_string;

	style_string =
		gtk_editable_get_chars (GTK_EDITABLE(bc_style->style_entry),
					0, -1);
	*style = gl_barcode_text_to_style (style_string);

	*text_flag =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bc_style->text_check));

	g_free (style_string);
}

/****************************************************************************/
/* fill in values and ranges for controls.                                  */
/****************************************************************************/
void
gl_wdgt_bc_style_set_params (glWdgtBCStyle  *bc_style,
			     glBarcodeStyle  style,
			     gboolean        text_flag)
{
	const gchar *style_string;
	gint         pos;

	style_string = gl_barcode_style_to_text (style);

	g_signal_handlers_block_by_func (G_OBJECT(bc_style->style_entry),
					 G_CALLBACK (changed_cb),
					 bc_style);
	gtk_editable_delete_text (GTK_EDITABLE (bc_style->style_entry),
				  0, -1);
	g_signal_handlers_unblock_by_func (G_OBJECT(bc_style->style_entry),
					   G_CALLBACK(changed_cb),
					   bc_style);

	pos = 0;
	gtk_editable_insert_text (GTK_EDITABLE (bc_style->style_entry),
				  style_string,
				  strlen (style_string),
				  &pos);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bc_style->text_check),
				      text_flag);

}

/****************************************************************************/
/* Set size group for internal labels                                       */
/****************************************************************************/
void
gl_wdgt_bc_style_set_label_size_group (glWdgtBCStyle  *bc_style,
				       GtkSizeGroup   *label_size_group)
{
	gtk_size_group_add_widget (label_size_group, bc_style->style_label);
}
