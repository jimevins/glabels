/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_bc_style.c:  barcode style selection widget module
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

#include "prop_bc_style.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glPropBCStyleSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint prop_bc_style_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_prop_bc_style_class_init (glPropBCStyleClass * class);
static void gl_prop_bc_style_init (glPropBCStyle * prop_style);
static void gl_prop_bc_style_destroy (GtkObject * object);
static void gl_prop_bc_style_construct (glPropBCStyle * prop_style,
					gchar * label);
static void changed_cb (glPropBCStyle * prop_style);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_prop_bc_style_get_type (void)
{
	static guint prop_bc_style_type = 0;

	if (!prop_bc_style_type) {
		GtkTypeInfo prop_bc_style_info = {
			"glPropBCStyle",
			sizeof (glPropBCStyle),
			sizeof (glPropBCStyleClass),
			(GtkClassInitFunc) gl_prop_bc_style_class_init,
			(GtkObjectInitFunc) gl_prop_bc_style_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		prop_bc_style_type = gtk_type_unique (gtk_vbox_get_type (),
						      &prop_bc_style_info);
	}

	return prop_bc_style_type;
}

static void
gl_prop_bc_style_class_init (glPropBCStyleClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_prop_bc_style_destroy;

	prop_bc_style_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glPropBCStyleClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, prop_bc_style_signals,
				      LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_prop_bc_style_init (glPropBCStyle * prop_style)
{
	prop_style->postnet_radio = NULL;
	prop_style->ean_radio = NULL;
	prop_style->upc_radio = NULL;
	prop_style->isbn_radio = NULL;
	prop_style->code39_radio = NULL;
	prop_style->code128_radio = NULL;
	prop_style->code128c_radio = NULL;
	prop_style->code128b_radio = NULL;
	prop_style->i25_radio = NULL;
	prop_style->cbr_radio = NULL;
	prop_style->msi_radio = NULL;
	prop_style->pls_radio = NULL;
}

static void
gl_prop_bc_style_destroy (GtkObject * object)
{
	glPropBCStyle *prop_style;
	glPropBCStyleClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROP_BC_STYLE (object));

	prop_style = GL_PROP_BC_STYLE (object);
	class = GL_PROP_BC_STYLE_CLASS (GTK_OBJECT (prop_style)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_prop_bc_style_new (gchar * label)
{
	glPropBCStyle *prop_style;

	prop_style = gtk_type_new (gl_prop_bc_style_get_type ());

	gl_prop_bc_style_construct (prop_style, label);

	return GTK_WIDGET (prop_style);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_prop_bc_style_construct (glPropBCStyle * prop,
			    gchar * label)
{
	GtkWidget *wvbox, *wframe, *wvbox1, *whbox2, *wvbox2;
	GSList *radio_group = NULL;

	wvbox = GTK_WIDGET (prop);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wvbox1 = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox1), 10);
	gtk_container_add (GTK_CONTAINER (wframe), wvbox1);

	whbox2 = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_container_add (GTK_CONTAINER (wvbox1), whbox2);

	/* ... Start column ... */
	wvbox2 = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox2), 10);
	gtk_box_pack_start (GTK_BOX (whbox2), wvbox2, FALSE, FALSE, 0);

	/* POSTNET button */
	prop->postnet_radio =
	    gtk_radio_button_new_with_label (radio_group, "POSTNET");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->postnet_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->postnet_radio,
			    FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->postnet_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* EAN button */
	prop->ean_radio = gtk_radio_button_new_with_label (radio_group, "EAN");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->ean_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->ean_radio, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->ean_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* UPC button */
	prop->upc_radio = gtk_radio_button_new_with_label (radio_group, "UPC");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->upc_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->upc_radio, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->upc_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* ISBN button */
	prop->isbn_radio =
	    gtk_radio_button_new_with_label (radio_group, "ISBN");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->isbn_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->isbn_radio,
			    FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->isbn_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* CODE39 button */
	prop->code39_radio =
	    gtk_radio_button_new_with_label (radio_group, "Code 39");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->code39_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->code39_radio,
			    FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->code39_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* CODE128 button */
	prop->code128_radio =
	    gtk_radio_button_new_with_label (radio_group, "Code 128");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->code128_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->code128_radio,
			    FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->code128_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* ... Start column ... */
	wvbox2 = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox2), 10);
	gtk_box_pack_start (GTK_BOX (whbox2), wvbox2, FALSE, FALSE, 0);

	/* CODE128B button */
	prop->code128b_radio =
	    gtk_radio_button_new_with_label (radio_group, "Code 128-B");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->code128b_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->code128b_radio,
			    FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->code128b_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* CODE128C button */
	prop->code128c_radio =
	    gtk_radio_button_new_with_label (radio_group, "Code 128-C");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->code128c_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->code128c_radio,
			    FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->code128c_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* I25 button */
	prop->i25_radio =
	    gtk_radio_button_new_with_label (radio_group, "Interleaved 2 of 5");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->i25_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->i25_radio, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->i25_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* CBR button */
	prop->cbr_radio =
	    gtk_radio_button_new_with_label (radio_group, "Codabar");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->cbr_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->cbr_radio, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->cbr_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* MSI button */
	prop->msi_radio = gtk_radio_button_new_with_label (radio_group, "MSI");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->msi_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->msi_radio, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->msi_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* PLS button */
	prop->pls_radio =
	    gtk_radio_button_new_with_label (radio_group, "Plessey");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (prop->pls_radio));
	gtk_box_pack_start (GTK_BOX (wvbox2), prop->pls_radio, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->pls_radio), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

	/* ... Separator ... */
	gtk_box_pack_start (GTK_BOX (wvbox1), gtk_hseparator_new (), FALSE,
			    FALSE, 0);

	/* Text checkbox widget */
	prop->text_check =
	    gtk_check_button_new_with_label (_("Show text with barcode"));
	gtk_box_pack_start (GTK_BOX (wvbox1), prop->text_check,
			    FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->text_check), "toggled",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glPropBCStyle * prop_style)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (prop_style),
			 prop_bc_style_signals[CHANGED]);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_prop_bc_style_get_params (glPropBCStyle * prop,
			     glBarcodeStyle * style,
			     gboolean * text_flag)
{
	*text_flag =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prop->text_check));

	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (prop->postnet_radio))) {
		*style = GL_BARCODE_STYLE_POSTNET;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->ean_radio))) {
		*style = GL_BARCODE_STYLE_EAN;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->upc_radio))) {
		*style = GL_BARCODE_STYLE_UPC;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->isbn_radio))) {
		*style = GL_BARCODE_STYLE_ISBN;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->code39_radio))) {
		*style = GL_BARCODE_STYLE_39;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->code128_radio))) {
		*style = GL_BARCODE_STYLE_128;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->code128c_radio))) {
		*style = GL_BARCODE_STYLE_128C;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->code128b_radio))) {
		*style = GL_BARCODE_STYLE_128B;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->i25_radio))) {
		*style = GL_BARCODE_STYLE_I25;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->cbr_radio))) {
		*style = GL_BARCODE_STYLE_CBR;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->msi_radio))) {
		*style = GL_BARCODE_STYLE_MSI;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (prop->pls_radio))) {
		*style = GL_BARCODE_STYLE_PLS;
	}

}

/*====================================================================*/
/* fill in values and ranges for controls.                            */
/*====================================================================*/
void
gl_prop_bc_style_set_params (glPropBCStyle * prop,
			     glBarcodeStyle style,
			     gboolean text_flag)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prop->text_check),
				      text_flag);

	switch (style) {
	case GL_BARCODE_STYLE_POSTNET:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->postnet_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_EAN:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->ean_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_UPC:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->upc_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_ISBN:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->isbn_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_39:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->code39_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_128:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->code128_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_128C:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->code128c_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_128B:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->code128b_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_I25:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->i25_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_CBR:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->cbr_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_MSI:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->msi_radio), TRUE);
		break;
	case GL_BARCODE_STYLE_PLS:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->pls_radio), TRUE);
		break;
	default:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (prop->postnet_radio), TRUE);
		break;
	}
}
