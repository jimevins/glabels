/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  color.c:  various small utilities for dealing with canvas colors
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
#include <config.h>

#include "color.h"

/*****************************************************************************/
/* Apply given opacity to given color.                                       */
/*****************************************************************************/
guint
gl_color_set_opacity (guint   color,
		      gdouble opacity)
{
	guint new_color;

	new_color = (color & 0xFFFFFF00) | (((guint)(255.0*opacity)) & 0xFF);

	return new_color;
}

/*****************************************************************************/
/* Convert canvas color into a GdkColor                                      */
/*****************************************************************************/
GdkColor *
gl_color_to_gdk_color (guint color)
{
	GdkColor *gdk_color;

	gdk_color = g_new0 (GdkColor, 1);

	gdk_color->red   = GL_COLOR_F_RED   (color) * 65535;
	gdk_color->green = GL_COLOR_F_GREEN (color) * 65535;
	gdk_color->blue  = GL_COLOR_F_BLUE  (color) * 65535;

	return gdk_color;
}

/*****************************************************************************/
/* Convert GdkColor into a canvas color                                      */
/*****************************************************************************/
guint
gl_color_from_gdk_color (GdkColor *gdk_color)
{
	guint color;

	color = GL_COLOR ((gdk_color->red   >>8),
			  (gdk_color->green >>8),
			  (gdk_color->blue  >>8));

	return color;
}

