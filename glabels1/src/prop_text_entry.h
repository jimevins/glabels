/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_text_entry.h:  text entry widget module header file
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

#ifndef __PROP_TEXT_ENTRY_H__
#define __PROP_TEXT_ENTRY_H__

#include <gnome.h>

#include "merge.h"

#define GL_TYPE_PROP_TEXT_ENTRY (gl_prop_text_entry_get_type ())
#define GL_PROP_TEXT_ENTRY(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_PROP_TEXT_ENTRY, glPropTextEntry ))
#define GL_PROP_TEXT_ENTRY_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PROP_TEXT_ENTRY, glPropTextEntryClass))
#define GL_IS_PROP_TEXT_ENTRY(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_PROP_TEXT_ENTRY))
#define GL_IS_PROP_TEXT_ENTRY_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PROP_TEXT_ENTRY))

typedef struct _glPropTextEntry glPropTextEntry;
typedef struct _glPropTextEntryClass glPropTextEntryClass;

struct _glPropTextEntry {
	GtkVBox parent_widget;

	GtkWidget *text_entry;
	GtkWidget *key_entry;
	GtkWidget *insert_button;
};

struct _glPropTextEntryClass {
	GtkVBoxClass parent_class;

	void (*changed) (glPropTextEntry * text_entry, gpointer user_data);
};

extern guint gl_prop_text_entry_get_type (void);

extern GtkWidget *gl_prop_text_entry_new (gchar * label, GList * field_defs);

extern GList *gl_prop_text_entry_get_text (glPropTextEntry * text_entry);

extern void gl_prop_text_entry_set_text (glPropTextEntry * text_entry,
					 gboolean merge_flag,
					 GList * lines);

#endif
