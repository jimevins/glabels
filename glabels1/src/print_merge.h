/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print_merge.h:  print merge widget module header file
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

#ifndef __PRINT_MERGE_H__
#define __PRINT_MERGE_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_PRINT_MERGE (gl_print_merge_get_type ())
#define GL_PRINT_MERGE(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_PRINT_MERGE, glPrintMerge ))
#define GL_PRINT_MERGE_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PRINT_MERGE, glPrintMergeClass))
#define GL_IS_PRINT_MERGE(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_PRINT_MERGE))
#define GL_IS_PRINT_MERGE_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PRINT_MERGE))

typedef struct _glPrintMerge glPrintMerge;
typedef struct _glPrintMergeClass glPrintMergeClass;

struct _glPrintMerge {
	GtkHBox parent_widget;

	gint labels_per_sheet;
	GtkWidget *mini_preview;

	gint n_records;

	GtkWidget *copies_spin;
	GtkWidget *first_spin;
	GtkWidget *collate_image;
	GtkWidget *collate_check;
};

struct _glPrintMergeClass {
	GtkHBoxClass parent_class;
};

extern guint gl_print_merge_get_type (void);

extern GtkWidget *gl_print_merge_new (glLabel * label);

extern void gl_print_merge_get_copies (glPrintMerge * merge,
				       gint * n_copies,
				       gint * first_label,
				       gboolean * collate_flag);

extern void gl_print_merge_set_copies (glPrintMerge * merge,
				       gint n_copies,
				       gint first_label,
				       gint n_records,
				       gboolean collate_flag);

#endif
