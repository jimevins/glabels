/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  item_text.h:  GLabels Text Object (canvas item) module header file
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
#ifndef __ITEM_TEXT_H__
#define __ITEM_TEXT_H__

#include <gnome.h>

#include "label.h"

extern GnomeCanvasItem *gl_item_text_new (glLabelObject * object,
					  glDisplay * display);

extern void gl_item_text_highlight (GnomeCanvasItem * item);

extern void gl_item_text_get_position_size (GnomeCanvasItem * item,
					    gdouble * x,
					    gdouble * y,
					    gdouble * w,
					    gdouble * h);

extern void gl_item_text_set_position_size (GnomeCanvasItem * item,
					    gdouble x,
					    gdouble y,
					    gdouble w,
					    gdouble h);

extern void gl_item_text_get_bounds (GnomeCanvasItem * item,
				     gdouble * x1,
				     gdouble * y1,
				     gdouble * x2,
				     gdouble * y2);

extern void gl_item_text_edit_dialog (GnomeCanvasItem * item);

extern GdkCursor *gl_item_text_get_create_cursor (void);

extern gint gl_item_text_create_event_handler (GnomeCanvas * canvas,
					       GdkEvent * event,
					       gpointer data);

#endif
