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

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum {
	GL_MERGE_SRC_IS_FIXED,
	GL_MERGE_SRC_IS_FILE,
} glMergeSrcType;

typedef struct {
	gchar *key;
	gchar *value;
} glMergeField;

typedef struct {
	gboolean select_flag;
	GList    *field_list;  /* List of glMergeFields */
} glMergeRecord;


#define GL_TYPE_MERGE              (gl_merge_get_type ())
#define GL_MERGE(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MERGE, glMerge))
#define GL_MERGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE, glMergeClass))
#define GL_IS_MERGE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MERGE))
#define GL_IS_MERGE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE))
#define GL_MERGE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_MERGE, glMergeClass))


typedef struct _glMerge          glMerge;
typedef struct _glMergeClass     glMergeClass;

typedef struct _glMergePrivate   glMergePrivate;


struct _glMerge {
	GObject          object;

	glMergePrivate  *private;
};

struct _glMergeClass {
	GObjectClass     parent_class;

        GList         *(*get_key_list)   (glMerge *merge);

	void           (*open)           (glMerge *merge);

	void           (*close)          (glMerge *merge);

	glMergeRecord *(*get_record)     (glMerge *merge);

	void           (*copy)           (glMerge *dst_merge,
					  glMerge *src_merge);
};


void              gl_merge_register_backend    (GType              type,
						gchar             *name,
						gchar             *description,
						glMergeSrcType     src_type,
						const gchar       *first_arg_name,
						...);

GList            *gl_merge_get_descriptions    (void);

void              gl_merge_free_descriptions   (GList **descriptions);

gchar            *gl_merge_description_to_name (gchar *description);

GType             gl_merge_get_type            (void);

glMerge          *gl_merge_new                 (gchar             *name);

glMerge          *gl_merge_dup                 (glMerge           *orig);

gchar            *gl_merge_get_name            (glMerge           *merge);

gchar            *gl_merge_get_description     (glMerge           *merge);

glMergeSrcType    gl_merge_get_src_type        (glMerge           *merge);

void              gl_merge_set_src             (glMerge           *merge,
						gchar             *src);

gchar            *gl_merge_get_src             (glMerge           *merge);

GList            *gl_merge_get_key_list        (glMerge           *merge);

void              gl_merge_free_key_list       (GList            **keys);

void              gl_merge_open                (glMerge           *merge);

void              gl_merge_close               (glMerge           *merge);

glMergeRecord    *gl_merge_get_record          (glMerge           *merge);

void              gl_merge_free_record         (glMergeRecord    **record);

gchar            *gl_merge_eval_key            (glMergeRecord     *record,
						gchar             *key);

GList            *gl_merge_read_record_list    (glMerge           *merge);

void              gl_merge_free_record_list    (GList            **record_list);

gint              gl_merge_count_records       (GList             *record_list);

G_END_DECLS

#endif
