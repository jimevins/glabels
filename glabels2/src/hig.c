/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  hig.c:  HIG inspired dialog and layout tools
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

#include <glib.h>

#include "hig.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define HIG_DIALOG_BORDER        12
#define HIG_DIALOG_VBOX_SPACING  18
#define HIG_DIALOG_ACTION_BORDER  0

#define HIG_GENERAL_SPACING       6

/*===========================================*/
/* Private globals                           */
/*===========================================*/
static GtkDialogClass *hig_dialog_parent_class;
static GtkVBoxClass   *hig_category_parent_class;


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_hig_dialog_class_init   (glHigDialogClass *class);
static void       gl_hig_dialog_init         (glHigDialog *hig_dialog);
static void       gl_hig_dialog_finalize     (GObject *object);

static void       add_buttons_valist         (glHigDialog *dialog,
					      const gchar *first_button_text,
					      va_list      args);

static void       gl_hig_category_class_init (glHigCategoryClass *class);
static void       gl_hig_category_init       (glHigCategory *hig_category);
static void       gl_hig_category_finalize   (GObject *object);


/****************************************************************************/
/* Boilerplate Dialog Object stuff.                                         */
/****************************************************************************/
guint
gl_hig_dialog_get_type (void)
{
	static guint hig_dialog_type = 0;

	if (!hig_dialog_type) {
		GTypeInfo hig_dialog_info = {
			sizeof (glHigDialogClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_hig_dialog_class_init,
			NULL,
			NULL,
			sizeof (glHigDialog),
			0,
			(GInstanceInitFunc) gl_hig_dialog_init,
		};

		hig_dialog_type =
		    g_type_register_static (gtk_dialog_get_type (),
					    "glHigDialog",
					    &hig_dialog_info, 0);
	}

	return hig_dialog_type;
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
			     HIG_DIALOG_VBOX_SPACING);
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
/* Boilerplate Category Object stuff.                                       */
/****************************************************************************/
guint
gl_hig_category_get_type (void)
{
	static guint hig_category_type = 0;

	if (!hig_category_type) {
		GTypeInfo hig_category_info = {
			sizeof (glHigCategoryClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_hig_category_class_init,
			NULL,
			NULL,
			sizeof (glHigCategory),
			0,
			(GInstanceInitFunc) gl_hig_category_init,
		};

		hig_category_type =
		    g_type_register_static (gtk_vbox_get_type (),
					    "glHigCategory",
					    &hig_category_info, 0);
	}

	return hig_category_type;
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

	gtk_box_set_spacing (GTK_BOX(hig_category), HIG_GENERAL_SPACING);

	/* 1st row: Label */
	hig_category->label = gtk_label_new ("");
	gtk_label_set_use_markup (GTK_LABEL(hig_category->label), TRUE);
	gtk_misc_set_alignment (GTK_MISC(hig_category->label), 0.0, 0.0);
	gtk_box_pack_start (GTK_BOX(hig_category),
			    hig_category->label, FALSE, FALSE, 0);
	
	/* 2nd row: HBOX */
	hbox = gtk_hbox_new (FALSE, HIG_GENERAL_SPACING);
	gtk_box_pack_start (GTK_BOX(hig_category), hbox, FALSE, FALSE, 0);

	/* 2nd row, Column 1: Indentation spacing */
	gtk_box_pack_start (GTK_BOX(hbox),
			    gtk_label_new ("    "), FALSE, FALSE, 0);

	/* 2nd row, Column 2: User area (inner vbox) */
	hig_category->vbox = gtk_vbox_new (FALSE, HIG_GENERAL_SPACING);
	gtk_box_pack_start (GTK_BOX(hbox),
			    hig_category->vbox, FALSE, FALSE, 0);
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

