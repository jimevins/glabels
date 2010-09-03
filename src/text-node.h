/*
 *  text-node.h
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TEXT_NODE_H__
#define __TEXT_NODE_H__

#include <glib.h>
#include "merge.h"

G_BEGIN_DECLS

typedef struct {
	gboolean field_flag;
	gchar *data;
} glTextNode;

gchar      *gl_text_node_expand              (const glTextNode    *text_node,
					      const glMergeRecord *record);
glTextNode *gl_text_node_new_from_text       (const gchar         *text);
glTextNode *gl_text_node_dup                 (const glTextNode    *text_node);
void        gl_text_node_free                (glTextNode         **text_node);

gboolean    gl_text_node_equal               (const glTextNode    *text_node1,
					      const glTextNode    *text_node2);

gchar      *gl_text_node_lines_expand        (GList               *lines,
					      const glMergeRecord *record);
GList      *gl_text_node_lines_new_from_text (const gchar         *text);
GList      *gl_text_node_lines_dup           (GList               *lines);
void        gl_text_node_lines_free          (GList              **lines);

/* debug function */
void        gl_text_node_lines_print         (GList               *lines);

G_END_DECLS

#endif



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
