/* Modified version of gnome-propertybox from gnome-libs-1.4 */
/* Primarily removed the "help" button and changed the names.*/
/*  -Jim Evins 11/25/2001 */

/* gnome-propertybox.h - Property dialog box.

   Copyright (C) 1998 Tom Tromey

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#ifndef __PROPERTY_BOX_H__
#define __PROPERTY_BOX_H__

#include <libgnomeui/gnome-dialog.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS
#define GL_TYPE_PROPERTY_BOX            (gl_property_box_get_type ())
#define GL_PROPERTY_BOX(obj)            (GTK_CHECK_CAST ((obj), GL_TYPE_PROPERTY_BOX, glPropertyBox))
#define GL_PROPERTY_BOX_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PROPERTY_BOX, glPropertyBoxClass))
#define GL_IS_PROPERTY_BOX(obj)         (GTK_CHECK_TYPE ((obj), GL_TYPE_PROPERTY_BOX))
#define GL_IS_PROPERTY_BOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PROPERTY_BOX))
/*the flag used on the notebook pages to see if a change happened on a certain page or not*/
#define GL_PROPERTY_BOX_DIRTY	"gl_property_box_dirty"
typedef struct _glPropertyBox glPropertyBox;
typedef struct _glPropertyBoxClass glPropertyBoxClass;

struct _glPropertyBox {
	GnomeDialog dialog;

	GtkWidget *notebook;	/* The notebook widget.  */
	GtkWidget *ok_button;	/* OK button.  */
	GtkWidget *apply_button;	/* Apply button.  */
	GtkWidget *cancel_button;	/* Cancel/Close button.  */
};

struct _glPropertyBoxClass {
	GnomeDialogClass parent_class;

	void (*apply) (glPropertyBox * propertybox,
		       gint page_num);
};

guint gl_property_box_get_type (void);
GtkWidget *gl_property_box_new (void);

/*
 * Call this when the user changes something in the current page of
 * the notebook.
 */
void gl_property_box_changed (glPropertyBox * property_box);

void gl_property_box_set_modified (glPropertyBox * property_box,
				   gboolean state);

gint gl_property_box_append_page (glPropertyBox * property_box,
				  GtkWidget * child,
				  GtkWidget * tab_label);

END_GNOME_DECLS
#endif				/* __PROPERTY_BOX_H__ */
