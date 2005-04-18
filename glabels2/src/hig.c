/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  hig.c:  HIG inspired dialogs and layout tools
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include "hig.h"

#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include <glib/gmessages.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define HIG_ALERT_BORDER               6
#define HIG_ALERT_SPACING             12

#define HIG_DIALOG_BORDER             12
#define HIG_DIALOG_VBOX_SPACING       18
#define HIG_DIALOG_OUTER_VBOX_SPACING 12

#define HIG_VBOX_OUTER_SPACING        18
#define HIG_VBOX_OUTER_BORDER         12

#define HIG_VBOX_INNER_SPACING         6

#define HIG_HBOX_SPACING              12

/*===========================================*/
/* Private globals                           */
/*===========================================*/
static GtkDialogClass *hig_alert_parent_class;
static GtkDialogClass *hig_dialog_parent_class;
static GtkVBoxClass   *hig_category_parent_class;
static GtkVBoxClass   *hig_vbox_parent_class;
static GtkHBoxClass   *hig_hbox_parent_class;


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_hig_alert_class_init    (glHigAlertClass *class);
static void       gl_hig_alert_init          (glHigAlert *hig_alert);
static void       gl_hig_alert_finalize      (GObject *object);

static void       gl_hig_dialog_class_init   (glHigDialogClass *class);
static void       gl_hig_dialog_init         (glHigDialog *hig_dialog);
static void       gl_hig_dialog_finalize     (GObject *object);

static void       add_buttons_valist         (glHigDialog *dialog,
					      const gchar *first_button_text,
					      va_list      args);

static void       gl_hig_category_class_init (glHigCategoryClass *class);
static void       gl_hig_category_init       (glHigCategory *hig_category);
static void       gl_hig_category_finalize   (GObject *object);

static void       gl_hig_vbox_class_init     (glHigVBoxClass *class);
static void       gl_hig_vbox_init           (glHigVBox *hig_vbox);
static void       gl_hig_vbox_finalize       (GObject *object);

static void       gl_hig_hbox_class_init     (glHigHBoxClass *class);
static void       gl_hig_hbox_init           (glHigHBox *hig_hbox);
static void       gl_hig_hbox_finalize       (GObject *object);


/****************************************************************************/
/****************************************************************************/
/* Boilerplate Alert Object stuff.                                          */
/****************************************************************************/
/****************************************************************************/
GType
gl_hig_alert_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glHigAlertClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_hig_alert_class_init,
			NULL,
			NULL,
			sizeof (glHigAlert),
			0,
			(GInstanceInitFunc) gl_hig_alert_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_DIALOG,
					       "glHigAlert", &info, 0);
	}

	return type;
}

static void
gl_hig_alert_class_init (glHigAlertClass *class)
{
	GObjectClass *object_class = (GObjectClass *) class;

	hig_alert_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_hig_alert_finalize;
}

static void
gl_hig_alert_init (glHigAlert *hig_alert)
{
}

static void
gl_hig_alert_finalize (GObject *object)
{
	glHigAlert *hig_alert;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_HIG_ALERT (object));

	hig_alert = GL_HIG_ALERT (object);

	G_OBJECT_CLASS (hig_alert_parent_class)->finalize (object);
}


/****************************************************************************/
/* Create a message dialog that attempts to be HIG compliant.               */
/****************************************************************************/
GtkWidget* gl_hig_alert_new      (GtkWindow      *parent,
				  GtkDialogFlags  flags,
				  GtkMessageType  type,
				  GtkButtonsType  buttons,
				  const gchar    *primary_text,
				  const gchar    *secondary_text)
{
	GtkWidget    *dialog, *hbox, *image, *label;
	gchar        *label_text;
	const gchar  *stock_id = NULL;
	GtkStockItem  item;
  
	/* Bare dialog */
	dialog = g_object_new (gl_hig_alert_get_type (), NULL);

	/* Parent */
	gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

	/* Flags */
	if ( flags & GTK_DIALOG_MODAL ) {
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
	}
	if ( flags & GTK_DIALOG_DESTROY_WITH_PARENT ) {
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);
	}

	/* Create HBOX */
	hbox = gtk_hbox_new (FALSE, HIG_ALERT_SPACING);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox),
			    hbox, FALSE, FALSE, 0);

	/* Create image */
	switch (type) {
	case GTK_MESSAGE_INFO:
		stock_id = GTK_STOCK_DIALOG_INFO;
		break;
	case GTK_MESSAGE_QUESTION:
		stock_id = GTK_STOCK_DIALOG_QUESTION;
		break;
	case GTK_MESSAGE_WARNING:
		stock_id = GTK_STOCK_DIALOG_WARNING;
		break;
	case GTK_MESSAGE_ERROR:
		stock_id = GTK_STOCK_DIALOG_ERROR;
		break;
	default:
		stock_id = GTK_STOCK_DIALOG_INFO;
		g_warning ("Unknown GtkMessageType %d", type);
		break;
	}
	if (gtk_stock_lookup (stock_id, &item)) {
		image = gtk_image_new_from_stock (stock_id,
						  GTK_ICON_SIZE_DIALOG);
	} else {
		image = gtk_image_new_from_stock (NULL,
						  GTK_ICON_SIZE_DIALOG);
		g_warning ("Stock dialog ID doesn't exist?");
	}
	gtk_box_pack_start (GTK_BOX(hbox), image, FALSE, FALSE, 0);

	/* Create label containing primary and secondary text */
	label_text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s",
				      primary_text, secondary_text);
	label = gtk_label_new (label_text);
	g_free (label_text);
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);

	/* Adjust dialog properties */
	gtk_window_set_title (GTK_WINDOW(dialog), "");
	gtk_container_set_border_width (GTK_CONTAINER(dialog),
					HIG_ALERT_BORDER);
	gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);

	/* Adjust VBOX properties */
	gtk_box_set_spacing (GTK_BOX(GTK_DIALOG(dialog)->vbox),
			     HIG_ALERT_SPACING);

	/* Adjust HBOX properties */
	gtk_box_set_spacing (GTK_BOX(hbox), HIG_ALERT_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), HIG_ALERT_BORDER);

	/* Adjust IMAGE properties */
	gtk_misc_set_alignment (GTK_MISC(image), 0.5, 0.0);

	/* Adjust LABEL properties */
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
	gtk_misc_set_alignment (GTK_MISC(image), 0.5, 0.0);

	/* Add buttons */
	switch (buttons) {
	case GTK_BUTTONS_NONE:
		/* nothing */
		break;
	case GTK_BUTTONS_OK:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_OK, GTK_RESPONSE_OK);
		break;
	case GTK_BUTTONS_CLOSE:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
		break;
	case GTK_BUTTONS_CANCEL:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		break;
	case GTK_BUTTONS_YES_NO:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_NO, GTK_RESPONSE_NO);
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_YES, GTK_RESPONSE_YES);
		break;
	case GTK_BUTTONS_OK_CANCEL:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_OK, GTK_RESPONSE_OK);
		break;
	default:
		g_warning ("Unknown GtkButtonsType");
		break;
	}

	/* Show dialog widgets */
	gtk_widget_show_all (hbox);

	return dialog;
}


/****************************************************************************/
/****************************************************************************/
/* Boilerplate Dialog Object stuff.                                         */
/****************************************************************************/
/****************************************************************************/
GType
gl_hig_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glHigDialogClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_hig_dialog_class_init,
			NULL,
			NULL,
			sizeof (glHigDialog),
			0,
			(GInstanceInitFunc) gl_hig_dialog_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_DIALOG,
					       "glHigDialog", &info, 0);
	}

	return type;
}

static void
gl_hig_dialog_class_init (glHigDialogClass *class)
{
	GObjectClass *object_class = (GObjectClass *) class;

	hig_dialog_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_hig_dialog_finalize;
}

static void
gl_hig_dialog_init (glHigDialog *hig_dialog)
{
	gtk_container_set_border_width (GTK_CONTAINER(hig_dialog),
					HIG_DIALOG_BORDER);

	hig_dialog->vbox = gtk_vbox_new (FALSE, HIG_DIALOG_VBOX_SPACING);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(hig_dialog)->vbox),
			    hig_dialog->vbox, FALSE, FALSE, 0);

	gtk_box_set_spacing (GTK_BOX(GTK_DIALOG(hig_dialog)->vbox),
			     HIG_DIALOG_OUTER_VBOX_SPACING);
}

static void
gl_hig_dialog_finalize (GObject *object)
{
	glHigDialog *hig_dialog;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_HIG_DIALOG (object));

	hig_dialog = GL_HIG_DIALOG (object);

	G_OBJECT_CLASS (hig_dialog_parent_class)->finalize (object);
}


/****************************************************************************/
/* Create a dialog that attempts to be HIG compliant.                       */
/****************************************************************************/
GtkWidget* gl_hig_dialog_new (void)
{
	GtkWidget    *dialog;

	dialog = g_object_new (gl_hig_dialog_get_type (), NULL);

	return dialog;
}



/****************************************************************************/
/* Create a dialog that attempts to be HIG compliant with buttons.          */
/****************************************************************************/
GtkWidget *gl_hig_dialog_new_with_buttons (const gchar    *title,
					   GtkWindow      *parent,
					   GtkDialogFlags  flags,
					   const gchar    *first_button_text,
					   ...)
{
	GtkWidget    *dialog;
	va_list       args;
  
	/* Create bare dialog */
	dialog = g_object_new (gl_hig_dialog_get_type (), NULL);

	/* Title */
	gtk_window_set_title (GTK_WINDOW(dialog), title);

	/* Parent */
	gtk_window_set_transient_for (GTK_WINDOW(dialog), parent);

	/* Flags */
	if ( flags & GTK_DIALOG_MODAL ) {
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
	}
	if ( flags & GTK_DIALOG_DESTROY_WITH_PARENT ) {
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);
	}

	/* Buttons */
	va_start (args, first_button_text);
	add_buttons_valist (GL_HIG_DIALOG(dialog), first_button_text, args);
	va_end (args);


	return dialog;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Add buttons to dialog from va_list.                             */
/*---------------------------------------------------------------------------*/
static void
add_buttons_valist(glHigDialog    *dialog,
		   const gchar    *first_button_text,
		   va_list         args)
{
  const gchar* text;
  gint response_id;

  g_return_if_fail (GL_IS_HIG_DIALOG (dialog));
  
  if (first_button_text == NULL)
    return;
  
  text = first_button_text;
  response_id = va_arg (args, gint);

  while (text != NULL)
    {
      gtk_dialog_add_button (GTK_DIALOG(dialog), text, response_id);

      text = va_arg (args, gchar*);
      if (text == NULL)
        break;
      response_id = va_arg (args, int);
    }
}

/****************************************************************************/
/* Add widget (from top) to dialog's vbox.                                  */
/****************************************************************************/
void
gl_hig_dialog_add_widget (glHigDialog   *dialog,
			  GtkWidget     *widget)
{
	gtk_box_pack_start (GTK_BOX (dialog->vbox), widget, FALSE, FALSE, 0);
}


/****************************************************************************/
/****************************************************************************/
/* Boilerplate Category Object stuff.                                       */
/****************************************************************************/
/****************************************************************************/
GType
gl_hig_category_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glHigCategoryClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_hig_category_class_init,
			NULL,
			NULL,
			sizeof (glHigCategory),
			0,
			(GInstanceInitFunc) gl_hig_category_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_VBOX,
					       "glHigCategory", &info, 0);
	}

	return type;
}

static void
gl_hig_category_class_init (glHigCategoryClass *class)
{
	GObjectClass *object_class = (GObjectClass *) class;

	hig_category_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_hig_category_finalize;
}

static void
gl_hig_category_init (glHigCategory *hig_category)
{
	GtkWidget *hbox;

	gtk_box_set_spacing (GTK_BOX(hig_category), HIG_VBOX_INNER_SPACING);

	/* 1st row: Label */
	hig_category->label = gtk_label_new ("");
	gtk_label_set_use_markup (GTK_LABEL(hig_category->label), TRUE);
	gtk_misc_set_alignment (GTK_MISC(hig_category->label), 0.0, 0.0);
	gtk_box_pack_start (GTK_BOX(hig_category),
			    hig_category->label, FALSE, FALSE, 0);
	
	/* 2nd row: HBOX */
	hbox = gtk_hbox_new (FALSE, HIG_VBOX_INNER_SPACING);
	gtk_box_pack_start (GTK_BOX(hig_category), hbox, FALSE, FALSE, 0);

	/* 2nd row, Column 1: Indentation spacing */
	gtk_box_pack_start (GTK_BOX(hbox),
			    gtk_label_new ("    "), FALSE, FALSE, 0);

	/* 2nd row, Column 2: User area (inner vbox) */
	hig_category->vbox = gtk_vbox_new (FALSE, HIG_VBOX_INNER_SPACING);
	gtk_box_pack_start (GTK_BOX(hbox),
			    hig_category->vbox, TRUE, TRUE, 0);
}

static void
gl_hig_category_finalize (GObject *object)
{
	glHigCategory *hig_category;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_HIG_CATEGORY (object));

	hig_category = GL_HIG_CATEGORY (object);

	G_OBJECT_CLASS (hig_category_parent_class)->finalize (object);
}


/****************************************************************************/
/* Create a category layout container that attempts to be HIG compliant.    */
/****************************************************************************/
GtkWidget* gl_hig_category_new (const gchar *header)
{
	GtkWidget    *category;
	gchar        *marked_up_header;

	category = g_object_new (gl_hig_category_get_type (), NULL);

	marked_up_header = g_strdup_printf ("<span weight=\"bold\">%s</span>",
					    header);
	gtk_label_set_text (GTK_LABEL(GL_HIG_CATEGORY(category)->label),
			    marked_up_header);
	g_free (marked_up_header);

	gtk_label_set_use_markup (GTK_LABEL(GL_HIG_CATEGORY(category)->label),
				  TRUE);

	return category;
}

/****************************************************************************/
/* Add widget (from top) to category's vbox.                                */
/****************************************************************************/
void
gl_hig_category_add_widget (glHigCategory *cat,
			    GtkWidget     *widget)
{
	gtk_box_pack_start (GTK_BOX (cat->vbox), widget, FALSE, FALSE, 0);
}


/****************************************************************************/
/****************************************************************************/
/* Boilerplate VBox Object stuff.                                           */
/****************************************************************************/
/****************************************************************************/
GType
gl_hig_vbox_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glHigVBoxClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_hig_vbox_class_init,
			NULL,
			NULL,
			sizeof (glHigVBox),
			0,
			(GInstanceInitFunc) gl_hig_vbox_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_VBOX,
					       "glHigVBox", &info, 0);
	}

	return type;
}

static void
gl_hig_vbox_class_init (glHigVBoxClass *class)
{
	GObjectClass *object_class = (GObjectClass *) class;

	hig_vbox_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_hig_vbox_finalize;
}

static void
gl_hig_vbox_init (glHigVBox *hig_vbox)
{
	/* defaults to an INNER VBox */
	gtk_box_set_spacing (GTK_BOX(hig_vbox), HIG_VBOX_INNER_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(hig_vbox), 0);
}

static void
gl_hig_vbox_finalize (GObject *object)
{
	glHigVBox *hig_vbox;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_HIG_VBOX (object));

	hig_vbox = GL_HIG_VBOX (object);

	G_OBJECT_CLASS (hig_vbox_parent_class)->finalize (object);
}


/****************************************************************************/
/* Create a vbox wrapper that attempts to be HIG compliant.                 */
/****************************************************************************/
GtkWidget* gl_hig_vbox_new (glHigVBoxType type)
{
	GtkWidget    *hig_vbox;

	hig_vbox = g_object_new (gl_hig_vbox_get_type (), NULL);

	switch (type) {

	case GL_HIG_VBOX_OUTER:
		gtk_box_set_spacing (GTK_BOX(hig_vbox),
				     HIG_VBOX_OUTER_SPACING);
		gtk_container_set_border_width (GTK_CONTAINER(hig_vbox),
						HIG_VBOX_OUTER_BORDER);
		break;

	case GL_HIG_VBOX_INNER:
		gtk_box_set_spacing (GTK_BOX(hig_vbox),
				     HIG_VBOX_INNER_SPACING);
		gtk_container_set_border_width (GTK_CONTAINER(hig_vbox), 0);
		
	default:
		break;

	}

	return hig_vbox;
}

/****************************************************************************/
/* Add widget (from top) to vbox.                                           */
/****************************************************************************/
void
gl_hig_vbox_add_widget (glHigVBox     *hig_vbox,
			GtkWidget     *widget)
{
	gtk_box_pack_start (GTK_BOX (hig_vbox), widget, FALSE, FALSE, 0);
}


/****************************************************************************/
/****************************************************************************/
/* Boilerplate HBox Object stuff.                                           */
/****************************************************************************/
/****************************************************************************/
GType
gl_hig_hbox_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glHigHBoxClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_hig_hbox_class_init,
			NULL,
			NULL,
			sizeof (glHigHBox),
			0,
			(GInstanceInitFunc) gl_hig_hbox_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_HBOX,
					       "glHigHBox", &info, 0);
	}

	return type;
}

static void
gl_hig_hbox_class_init (glHigHBoxClass *class)
{
	GObjectClass *object_class = (GObjectClass *) class;

	hig_hbox_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_hig_hbox_finalize;
}

static void
gl_hig_hbox_init (glHigHBox *hig_hbox)
{
	gtk_box_set_spacing (GTK_BOX(hig_hbox), HIG_HBOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(hig_hbox), 0);
}

static void
gl_hig_hbox_finalize (GObject *object)
{
	glHigHBox *hig_hbox;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_HIG_HBOX (object));

	hig_hbox = GL_HIG_HBOX (object);

	G_OBJECT_CLASS (hig_hbox_parent_class)->finalize (object);
}


/****************************************************************************/
/* Create a hbox wrapper that attempts to be HIG compliant.                 */
/****************************************************************************/
GtkWidget* gl_hig_hbox_new (void)
{
	GtkWidget    *hig_hbox;

	hig_hbox = g_object_new (gl_hig_hbox_get_type (), NULL);

	return hig_hbox;
}

/****************************************************************************/
/* Add widget (from left) to hbox.                                          */
/****************************************************************************/
void
gl_hig_hbox_add_widget (glHigHBox     *hig_hbox,
			GtkWidget     *widget)
{
	gtk_box_pack_start (GTK_BOX (hig_hbox), widget, FALSE, FALSE, 0);
}

/****************************************************************************/
/* Add widget (from left) to hbox with a right justification.               */
/****************************************************************************/
void
gl_hig_hbox_add_widget_justify (glHigHBox     *hig_hbox,
				GtkWidget     *widget)
{
	gtk_box_pack_start (GTK_BOX (hig_hbox), widget, TRUE, TRUE, 0);
}


