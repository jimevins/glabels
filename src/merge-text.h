/*
 *  merge-text.h
 *  Copyright (C) 2002-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __MERGE_TEXT_H__
#define __MERGE_TEXT_H__

#include "merge.h"

G_BEGIN_DECLS

/* The following object arguments are available:
 *
 * name               type             description
 * ---------------------------------------------------------------------------
 * delim              gchar            Field delimiter.
 * line1_has_keys     gboolean         Do we harvest key names from line 1?
 *
 */

#define GL_TYPE_MERGE_TEXT              (gl_merge_text_get_type ())
#define GL_MERGE_TEXT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MERGE_TEXT, glMergeText))
#define GL_MERGE_TEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE_TEXT, glMergeTextClass))
#define GL_IS_MERGE_TEXT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MERGE_TEXT))
#define GL_IS_MERGE_TEXT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE_TEXT))
#define GL_MERGE_TEXT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_MERGE_TEXT, glMergeTextClass))


typedef struct _glMergeText          glMergeText;
typedef struct _glMergeTextClass     glMergeTextClass;

typedef struct _glMergeTextPrivate   glMergeTextPrivate;


struct _glMergeText {
	glMerge              object;

	glMergeTextPrivate  *priv;
};

struct _glMergeTextClass {
	glMergeClass         parent_class;
};


GType             gl_merge_text_get_type            (void) G_GNUC_CONST;

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
