/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  recent.h:  gLabels recent files module header file
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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

#ifndef __RECENT_H__
#define __RECENT_H__

#include <gtk/gtkrecentmanager.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

void       gl_recent_init              (void);

gchar     *gl_recent_get_utf8_filename (GtkRecentInfo *item);

void       gl_recent_add_utf8_filename (gchar         *utf8_filename);

GtkWidget *gl_recent_create_menu       (void);


G_END_DECLS

#endif /*__RECENT_H__*/

