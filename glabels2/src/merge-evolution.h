/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_evolution.h:  evolution merge backend module header file
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
 *  and
 *  Copyright (C) 2005  Austin Henry <ahenry@users.sourceforge.net>
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
#ifndef __MERGE_EVOLUTION_H__
#define __MERGE_EVOLUTION_H__

#include "merge.h"

G_BEGIN_DECLS

/* The following object arguments are available:
 *
 * name               type             description
 * --------------------------------------------------------------------------------
 * query              gchar*           The query used to select records
 *
 */

#define GL_TYPE_MERGE_EVOLUTION              (gl_merge_evolution_get_type ())
#define GL_MERGE_EVOLUTION(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MERGE_EVOLUTION, glMergeEvolution))
#define GL_MERGE_EVOLUTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE_EVOLUTION, glMergeEvolutionClass))
#define GL_IS_MERGE_EVOLUTION(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MERGE_EVOLUTION))
#define GL_IS_MERGE_EVOLUTION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE_EVOLUTION))
#define GL_MERGE_EVOLUTION_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_MERGE_EVOLUTION, glMergeEvolutionClass))


typedef struct _glMergeEvolution          glMergeEvolution;
typedef struct _glMergeEvolutionClass     glMergeEvolutionClass;

typedef struct _glMergeEvolutionPrivate   glMergeEvolutionPrivate;


struct _glMergeEvolution {
	glMerge              object;

	glMergeEvolutionPrivate  *private;
};

struct _glMergeEvolutionClass {
	glMergeClass         parent_class;
};


GType             gl_merge_evolution_get_type            (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __MERGE_EVOLUTION_H__ */
