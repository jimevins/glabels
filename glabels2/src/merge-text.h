/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_text.h:  text-file merge backend module header file
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
#ifndef __MERGE_TEXT_H__
#define __MERGE_TEXT_H__

#include <gtk/gtk.h>

#include "merge.h"

extern glMergeInput *gl_merge_text_open (glMergeType type, GList * field_defs,
					 gchar * src);
extern void gl_merge_text_close (glMergeInput * input);
extern glMergeRecord *gl_merge_text_get_record (glMergeInput * input);
extern GList *gl_merge_text_get_raw_record (glMergeInput * input);

#endif
