/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  libglabels-private.h:  libglabels private header file
 *
 *  Copyright (C) 2004  Jim Evins <evins@snaught.com>.
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

#ifndef __LIBGLABELS_PRIVATE_H__
#define __LIBGLABELS_PRIVATE_H__

#include <config.h>

#if ENABLE_NLS

#       include <libintl.h>
#       define _(x) dgettext (GETTEXT_PACKAGE, x)
#       define N_(x) x

#else

#       define _(x) x
#       define N_(x) x

#endif

/* Data system and user data directories.  (must free w/ g_free()) */
#define GL_SYSTEM_DATA_DIR g_build_filename (LIBGLABELS_TEMPLATE_DIR, NULL)
#define GL_USER_DATA_DIR   g_build_filename (g_get_home_dir (), ".glabels", NULL)

#endif /* __LIBGLABELS_PRIVATE_H__ */
