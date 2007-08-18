/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs.h:  Application preferences module header file
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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
#ifndef __PREFS_H__
#define __PREFS_H__

#include "prefs-model.h"

G_BEGIN_DECLS



extern glPrefsModel *gl_prefs;



void          gl_prefs_init                (void);

const gchar  *gl_prefs_get_page_size       (void);

glUnitsType   gl_prefs_get_units           (void);

const gchar  *gl_prefs_get_units_string    (void);

gdouble       gl_prefs_get_units_per_point (void);

gdouble       gl_prefs_get_units_step_size (void);

gint          gl_prefs_get_units_precision (void);

G_END_DECLS

#endif /* __PREFS_H__ */

