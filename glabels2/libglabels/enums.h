/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  enums.h:  common libGLabels enumerations header file
 *
 *  Copyright (C) 2003, 2004  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of the LIBGLABELS library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA
 */

#ifndef __ENUMS_H__
#define __ENUMS_H__

#include <glib.h>

G_BEGIN_DECLS

/* Units of distance */
typedef enum {
	GL_UNITS_POINT,
	GL_UNITS_INCH,
	GL_UNITS_MM,
	GL_UNITS_CM,
	GL_UNITS_PICA,

	GL_UNITS_FIRST = GL_UNITS_POINT,
	GL_UNITS_LAST  = GL_UNITS_PICA,
} glUnitsType;


G_END_DECLS


#endif /* __ENUMS_H__ */
