/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  recent.c:  gLabels recent files module
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
#include <config.h>


#include "recent.h"
#include "prefs.h"

#include "debug.h"

static GnomeRecentModel *model;


GnomeRecentModel *
gl_recent_get_model (void)
{
	return model;
}

void
gl_recent_init (void)
{
	gl_debug (DEBUG_RECENT, "max_recents = %d", gl_prefs->max_recents);

	model = gnome_recent_model_new ("glabels", gl_prefs->max_recents);
}
