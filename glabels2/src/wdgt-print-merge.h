/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_print_merge.h:  print merge widget module header file
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

#ifndef __WDGT_PRINT_MERGE_H__
#define __WDGT_PRINT_MERGE_H__

#include "label.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_PRINT_MERGE (gl_wdgt_print_merge_get_type ())
#define GL_WDGT_PRINT_MERGE(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_PRINT_MERGE, glWdgtPrintMerge ))
#define GL_WDGT_PRINT_MERGE_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_PRINT_MERGE, glWdgtPrintMergeClass))
#define GL_IS_WDGT_PRINT_MERGE(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_PRINT_MERGE))
#define GL_IS_WDGT_PRINT_MERGE_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_PRINT_MERGE))

typedef struct _glWdgtPrintMerge      glWdgtPrintMerge;
typedef struct _glWdgtPrintMergeClass glWdgtPrintMergeClass;

struct _glWdgtPrintMerge {
	glHigHBox parent_widget;

	gint      labels_per_sheet;
	GtkWidget *mini_preview;

	gint      n_records;

	GtkWidget *copies_spin;
	GtkWidget *first_spin;
	GtkWidget *collate_image;
	GtkWidget *collate_check;
};

struct _glWdgtPrintMergeClass {
	glHigHBoxClass parent_class;
};

GType      gl_wdgt_print_merge_get_type   (void) G_GNUC_CONST;

GtkWidget *gl_wdgt_print_merge_new        (glLabel          *label);

void       gl_wdgt_print_merge_get_copies (glWdgtPrintMerge *merge,
					   gint             *n_copies,
					   gint             *first_label,
					   gboolean         *collate_flag);

void       gl_wdgt_print_merge_set_copies (glWdgtPrintMerge *merge,
					   gint              n_copies,
					   gint              first_label,
					   gint              n_records,
					   gboolean          collate_flag);

G_END_DECLS

#endif
