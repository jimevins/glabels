/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  file.h:  FILE menu dialog module header file
 *
 *  Copyright (C) 2000  Jim Evins <evins@snaught.com>.
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

#ifndef __FILE_H__
#define __FILE_H__

#include "mdi-child.h"
#include "gnome-recent-view.h"

extern void     gl_file_new         (void);

extern void     gl_file_open        (glMDIChild *active_child);
extern gboolean gl_file_open_recent (GnomeRecentView *view,
				     const gchar *filename,
				     BonoboWindow *win);
extern gboolean gl_file_open_real   (const gchar *filename,
				     GtkWindow *win);

extern gboolean gl_file_save        (glMDIChild *child);
extern gboolean gl_file_save_as     (glMDIChild *child);

extern void     gl_file_close       (GtkWidget *view);
extern gboolean gl_file_close_all   (void);
extern void     gl_file_exit        (void);

#endif				/* __FILE_H__ */
