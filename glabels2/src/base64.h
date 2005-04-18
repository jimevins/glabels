/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  base64.h:  GLabels base64 encode/decode module
 *
 *  Copyright (C)  2003  Jim Evins <evins@snaught.com>
 *
 *  This module is based on base64.c from fetchmail:
 *
 *  Copyright (C)2002 by Eric S. Raymond.
 *  Portions are copyrighted by Carl E. Harris and George M. Sipe.
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
#ifndef __BASE64_H__
#define __BASE64_H__

#include <glib/gtypes.h>

G_BEGIN_DECLS

gchar  *gl_base64_encode (const guchar *in,
			  guint         inlen);

guchar *gl_base64_decode (const gchar  *in,
			  guint        *outlen);

G_END_DECLS

#endif

