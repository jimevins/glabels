/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  hack.h:  various small hacks to deal with I18N
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#ifndef __HACK_H__
#define __HACK_H__

#include <libgnomeprint/gnome-font.h>

extern gchar *gl_hack_text_to_utf8 (gchar * text);

extern double gl_hack_get_width_string (GnomeFont * font, gchar * text);

#endif				/* __HACK_H__ */
