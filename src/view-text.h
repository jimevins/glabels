/*
 *  view-text.h
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

#ifndef __VIEW_TEXT_H__
#define __VIEW_TEXT_H__

#include "view.h"
#include <gdk/gdk.h>

#include "view.h"
#include <gdk/gdk.h>

G_BEGIN_DECLS


/* cursor for creating text objects */
GdkCursor *gl_view_text_get_create_cursor           (void);

/* Object creation handlers. */
void       gl_view_text_create_button_press_event   (glView *view,
                                                     gdouble x,
                                                     gdouble y);

void       gl_view_text_create_motion_event         (glView *view,
                                                     gdouble x,
                                                     gdouble y);

void       gl_view_text_create_button_release_event (glView *view,
                                                     gdouble x,
                                                     gdouble y);


G_END_DECLS

#endif /* __VIEW_TEXT_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
