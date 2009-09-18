/*
 *  file.h
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __FILE_H__
#define __FILE_H__

#include <gtk/gtk.h>

#include "label.h"
#include "window.h"

G_BEGIN_DECLS

void     gl_file_new         (glWindow        *window);


void     gl_file_properties  (glLabel         *label,
			      glWindow        *window);


void     gl_file_open        (glWindow        *window);


void     gl_file_open_recent (const gchar     *filename,
			      glWindow        *window);

gboolean gl_file_open_real   (const gchar     *filename,
			      glWindow        *window);


gboolean gl_file_save        (glLabel         *label,
			      glWindow        *window);

gboolean gl_file_save_as     (glLabel         *label,
			      glWindow        *window);


gboolean gl_file_close       (glWindow        *window);

void     gl_file_exit        (void);

G_END_DECLS

#endif /* __FILE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
