/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_text_entry.h:  text entry widget module header file
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

#ifndef __WDGT_TEXT_ENTRY_H__
#define __WDGT_TEXT_ENTRY_H__

#include <gnome.h>
#include "merge.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_TEXT_ENTRY (gl_wdgt_text_entry_get_type ())
#define GL_WDGT_TEXT_ENTRY(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_TEXT_ENTRY, glWdgtTextEntry ))
#define GL_WDGT_TEXT_ENTRY_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_TEXT_ENTRY, glWdgtTextEntryClass))
#define GL_IS_WDGT_TEXT_ENTRY(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_TEXT_ENTRY))
#define GL_IS_WDGT_TEXT_ENTRY_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_TEXT_ENTRY))

typedef struct _glWdgtTextEntry glWdgtTextEntry;
typedef struct _glWdgtTextEntryClass glWdgtTextEntryClass;

struct _glWdgtTextEntry {
	glHigVBox      parent_widget;

	GtkWidget     *edit_label;
	GtkWidget     *text_entry;

	GtkWidget     *key_label;
	GtkWidget     *key_combo;
	GtkWidget     *key_entry;
	GtkWidget     *insert_button;

	GtkTextBuffer *text_buffer;
};

struct _glWdgtTextEntryClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtTextEntry * text_entry, gpointer user_data);
};

guint      gl_wdgt_text_entry_get_type       (void);

GtkWidget *gl_wdgt_text_entry_new            (GList           *field_defs);

void       gl_wdgt_text_entry_set_field_defs (glWdgtTextEntry *text_entry,
					      GList           *field_defs);

GList     *gl_wdgt_text_entry_get_text       (glWdgtTextEntry *text_entry);

void       gl_wdgt_text_entry_set_text       (glWdgtTextEntry *text_entry,
					      gboolean         merge_flag,
					      GList           *lines);

void       gl_wdgt_text_entry_set_label_size_group (glWdgtTextEntry *text_entry,
						    GtkSizeGroup    *label_size_group);

G_END_DECLS

#endif
