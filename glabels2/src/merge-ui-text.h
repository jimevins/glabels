/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_ui_text.h:  text-file merge user interface backend header
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
#ifndef __MERGE_UI_TEXT_H__
#define __MERGE_UI_TEXT_H__

#include <gnome.h>

#include "merge.h"

/*======================================================*/
/* Merge source selection widget                        */
/*======================================================*/
#define GL_TYPE_MERGE_UI_TEXT_SRC (gl_merge_ui_text_src_get_type ())
#define GL_MERGE_UI_TEXT_SRC(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_MERGE_UI_TEXT_SRC, glMergeUITextSrc ))
#define GL_MERGE_UI_TEXT_SRC_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE_UI_TEXT_SRC, glMergeUITextSrcClass))
#define GL_IS_MERGE_UI_TEXT_SRC(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_MERGE_UI_TEXT_SRC))
#define GL_IS_MERGE_UI_TEXT_SRC_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE_UI_TEXT_SRC))

typedef struct _glMergeUITextSrc glMergeUITextSrc;
typedef struct _glMergeUITextSrcClass glMergeUITextSrcClass;

struct _glMergeUITextSrc {
	GtkVBox parent_widget;

	glMergeType type;
	GtkWidget *entry;
};

struct _glMergeUITextSrcClass {
	GtkVBoxClass parent_class;

	void (*changed) (glMergeUITextSrc * src, gpointer user_data);
};

extern guint gl_merge_ui_text_src_get_type (void);
extern GtkWidget *gl_merge_ui_text_src_new (glMergeType type);
extern void gl_merge_ui_text_src_set_value (glMergeUITextSrc * src,
					    gchar * text);
extern gchar *gl_merge_ui_text_src_get_value (glMergeUITextSrc * src);

/*======================================================*/
/* Merge field selection/definition widget.             */
/*======================================================*/
#define GL_TYPE_MERGE_UI_TEXT_FIELD_WS (gl_merge_ui_text_field_ws_get_type ())
#define GL_MERGE_UI_TEXT_FIELD_WS(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_MERGE_UI_TEXT_FIELD_WS, glMergeUITextFieldWS ))
#define GL_MERGE_UI_TEXT_FIELD_WS_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE_UI_TEXT_FIELD_WS, glMergeUITextFieldWSClass))
#define GL_IS_MERGE_UI_TEXT_FIELD_WS(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_MERGE_UI_TEXT_FIELD_WS))
#define GL_IS_MERGE_UI_TEXT_FIELD_WS_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE_UI_TEXT_FIELD_WS))

typedef struct _glMergeUITextFieldWS glMergeUITextFieldWS;
typedef struct _glMergeUITextFieldWSClass glMergeUITextFieldWSClass;

struct _glMergeUITextFieldWS {
	GtkHBox parent_widget;

	glMergeType type;
	GList *entry_list;
};

struct _glMergeUITextFieldWSClass {
	GtkHBoxClass parent_class;

	void (*changed) (glMergeUITextFieldWS * field_ws, gpointer user_data);
};

extern guint gl_merge_ui_text_field_ws_get_type (void);
extern GtkWidget *gl_merge_ui_text_field_ws_new (glMergeType type,
						 gchar * src);
extern void gl_merge_ui_text_field_ws_set_field_defs (glMergeUITextFieldWS * field_ws,
						      GList * field_defs);
extern GList *gl_merge_ui_text_field_ws_get_field_defs (glMergeUITextFieldWS *
							field_ws);

#endif
