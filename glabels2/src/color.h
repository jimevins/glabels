/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  color.h:  various small utilities for dealing with canvas colors
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

#ifndef __COLOR_H__
#define __COLOR_H__

#include <libgnomecanvas/gnome-canvas.h>

G_BEGIN_DECLS

#define GL_COLOR(r,g,b)     GNOME_CANVAS_COLOR(r,g,b)
#define GL_COLOR_A(r,g,b,a) GNOME_CANVAS_COLOR_A(r,g,b,a)

#define GL_COLOR_I_RED(x)   (((x)>>24) & 0xff)
#define GL_COLOR_I_GREEN(x) (((x)>>16) & 0xff)
#define GL_COLOR_I_BLUE(x)  (((x)>>8)  & 0xff)
#define GL_COLOR_I_ALPHA(x) ( (x)      & 0xff)

#define GL_COLOR_F_RED(x)   ( (((x)>>24) & 0xff) / 255.0 )
#define GL_COLOR_F_GREEN(x) ( (((x)>>16) & 0xff) / 255.0 )
#define GL_COLOR_F_BLUE(x)  ( (((x)>>8)  & 0xff) / 255.0 )
#define GL_COLOR_F_ALPHA(x) ( ( (x)      & 0xff) / 255.0 )

guint     gl_color_set_opacity (guint color, gdouble opacity);

GdkColor *gl_color_to_gdk_color (guint color);

G_END_DECLS

#endif /* __COLOR_H__ */
