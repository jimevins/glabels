/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge.h:  document merge module header file
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
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
#ifndef __MERGE_H__
#define __MERGE_H__

#include <glib.h>

typedef enum {

	GL_MERGE_NONE,
	GL_MERGE_TEXT_TAB,
	GL_MERGE_TEXT_COMMA,
	GL_MERGE_TEXT_COLON,

	GL_MERGE_N_TYPES
} glMergeType;

typedef struct {
	glMergeType type;
	GList *field_defs;
	gpointer handle;
} glMergeInput;

typedef struct {
	gchar *key;
	gchar *loc;
} glMergeFieldDefinition;

typedef struct {
	gchar *loc;
	gchar *value;
} glMergeRawField;

typedef struct {
	gchar *key;
	gchar *value;
} glMergeField;

typedef struct {
	gboolean select_flag;
	GList    *field_list;  /* List of glMergeFields */
} glMergeRecord;

extern void
 gl_merge_init (void);

extern glMergeType gl_merge_text_to_type (gchar * text);
extern gchar *gl_merge_type_to_text (glMergeType type);
extern glMergeType gl_merge_long_text_to_type (gchar * text);
extern gchar *gl_merge_type_to_long_text (glMergeType type);
extern GList *gl_merge_get_long_texts_list (void);
extern void gl_merge_free_long_texts_list (GList ** list);

extern void gl_merge_free_field_def_list (GList ** field_defs);

extern GList *gl_merge_get_key_list (GList * field_defs);
extern void gl_merge_free_key_list (GList ** keys);

extern gchar *gl_merge_find_key (GList * field_defs, gchar * loc);

extern glMergeInput *gl_merge_open (glMergeType type, GList * field_defs,
				    gchar * src);
extern void gl_merge_close (glMergeInput * input);
extern glMergeRecord *gl_merge_get_record (glMergeInput * input);
extern GList *gl_merge_get_raw_record (glMergeInput * input);
extern void gl_merge_free_record (glMergeRecord ** record);
extern void gl_merge_free_raw_record (GList ** record);
extern gchar *gl_merge_eval_key (gchar * key, glMergeRecord * record);

extern GList *gl_merge_read_data (glMergeType type, GList * field_defs,
				  gchar * src);
extern void gl_merge_free_data (GList **record_list);
extern gint gl_merge_count_records (GList *record_list);

#endif
