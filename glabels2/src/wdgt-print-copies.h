/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_print_copies.h:  custom print copies widget module header file
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

#ifndef __WDGT_PRINT_COPIES_H__
#define __WDGT_PRINT_COPIES_H__

#include <gnome.h>
#include "label.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_PRINT_COPIES (gl_wdgt_print_copies_get_type ())
#define GL_WDGT_PRINT_COPIES(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_PRINT_COPIES, glWdgtPrintCopies ))
#define GL_WDGT_PRINT_COPIES_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_PRINT_COPIES, glWdgtPrintCopiesClass))
#define GL_IS_WDGT_PRINT_COPIES(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_PRINT_COPIES))
#define GL_IS_WDGT_PRINT_COPIES_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_PRINT_COPIES))

typedef struct _glWdgtPrintCopies      glWdgtPrintCopies;
typedef struct _glWdgtPrintCopiesClass glWdgtPrintCopiesClass;

struct _glWdgtPrintCopies {
	glHigHBox parent_widget;

	gint      labels_per_sheet;

	GtkWidget *mini_preview;

	GtkWidget *sheets_radio;
	GtkWidget *sheets_spin;

	GtkWidget *labels_radio;
	GtkWidget *first_spin;
	GtkWidget *last_spin;
};

struct _glWdgtPrintCopiesClass {
	glHigHBoxClass parent_class;
};

GType      gl_wdgt_print_copies_get_type  (void) G_GNUC_CONST;

GtkWidget *gl_wdgt_print_copies_new       (glLabel           *label);

void       gl_wdgt_print_copies_get_range (glWdgtPrintCopies *copies,
					   gint              *n_sheets,
					   gint              *first_label,
					   gint              *last_label);

void       gl_wdgt_print_copies_set_range (glWdgtPrintCopies *copies,
					   gint               n_sheets,
					   gint               first_label,
					   gint               last_label);

G_END_DECLS

#endif
