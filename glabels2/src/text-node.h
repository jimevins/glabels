/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  text_node.h:  text node module header file
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
#ifndef __TEXT_NODE_H__
#define __TEXT_NODE_H__

#include <gnome.h>
#include "merge.h"

typedef struct {
	gboolean field_flag;
	gchar *data;
} glTextNode;

extern gchar *gl_text_node_expand              (glTextNode * text_node,
						glMergeRecord * record);
extern glTextNode *gl_text_node_new_from_text  (gchar * text);
extern glTextNode *gl_text_node_dup            (glTextNode * text_node);
extern void gl_text_node_free                  (glTextNode ** text_node);

extern gchar *gl_text_node_lines_expand        (GList * lines,
						glMergeRecord * record);
extern GList *gl_text_node_lines_new_from_text (gchar * text);
extern GList *gl_text_node_lines_dup           (GList *lines);
extern void   gl_text_node_lines_free          (GList ** lines);

/* debug function */
extern void gl_text_node_lines_print (GList * lines );

#endif
