/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_ui.h:  document merge user interface module header file
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
#ifndef __MERGE_UI_H__
#define __MERGE_UI_H__

#include <gnome.h>

#include "merge.h"

extern void
 gl_merge_ui_init (void);

/*======================================================*/
/* Merge source selection widget                        */
/*======================================================*/
#define GL_TYPE_MERGE_UI_SRC (gl_merge_ui_src_get_type ())
#define GL_MERGE_UI_SRC(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_MERGE_UI_SRC, glMergeUISrc ))
#define GL_MERGE_UI_SRC_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE_UI_SRC, glMergeUISrcClass))
#define GL_IS_MERGE_UI_SRC(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_MERGE_UI_SRC))
#define GL_IS_MERGE_UI_SRC_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE_UI_SRC))

typedef struct _glMergeUISrc glMergeUISrc;
typedef struct _glMergeUISrcClass glMergeUISrcClass;

struct _glMergeUISrc {
	GtkVBox parent_widget;

	glMergeType type;
	GtkWidget *backend_widget;
};

struct _glMergeUISrcClass {
	GtkVBoxClass parent_class;

	void (*changed) (glMergeUISrc * src, gpointer user_data);
};

extern guint gl_merge_ui_src_get_type (void);
extern GtkWidget *gl_merge_ui_src_new (void);
extern void gl_merge_ui_src_set_type (glMergeUISrc * src, glMergeType type);
extern void gl_merge_ui_src_set_value (glMergeUISrc * src, gchar * text);
extern gchar *gl_merge_ui_src_get_value (glMergeUISrc * src);

/*======================================================*/
/* Merge field selection/definition widget.             */
/*======================================================*/
#define GL_TYPE_MERGE_UI_FIELD_WS (gl_merge_ui_field_ws_get_type ())
#define GL_MERGE_UI_FIELD_WS(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_MERGE_UI_FIELD_WS, glMergeUIFieldWS ))
#define GL_MERGE_UI_FIELD_WS_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE_UI_FIELD_WS, glMergeUIFieldWSClass))
#define GL_IS_MERGE_UI_FIELD_WS(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_MERGE_UI_FIELD_WS))
#define GL_IS_MERGE_UI_FIELD_WS_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE_UI_FIELD_WS))

typedef struct _glMergeUIFieldWS glMergeUIFieldWS;
typedef struct _glMergeUIFieldWSClass glMergeUIFieldWSClass;

struct _glMergeUIFieldWS {
	GtkVBox parent_widget;

	glMergeType type;
	gchar *src;
	GtkWidget *backend_widget;
};

struct _glMergeUIFieldWSClass {
	GtkVBoxClass parent_class;

	void (*changed) (glMergeUIFieldWS * field_ws, gpointer user_data);
};

extern guint gl_merge_ui_field_ws_get_type (void);
extern GtkWidget *gl_merge_ui_field_ws_new (void);
extern void gl_merge_ui_field_ws_set_type_src (glMergeUIFieldWS * field_ws,
					       glMergeType type,
					       gchar * src);
extern void gl_merge_ui_field_ws_set_field_defs (glMergeUIFieldWS * field_ws,
						 GList * field_defs);
extern GList *gl_merge_ui_field_ws_get_field_defs (glMergeUIFieldWS * field_ws);

#endif
