/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  glabels.h:  GLabels main module header file
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

/*
 * This file is based on gedit.h from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */
#ifndef __GLABELS_H__
#define __GLABELS_H__

#include <gmodule.h>
#include <glib/glist.h>
#include <bonobo/bonobo-window.h>

#include "mdi.h"
#include "label.h"
#include "view.h" 

extern glMDI*   glabels_mdi;
extern gboolean glabels_close_x_button_pressed;
extern gboolean glabels_exit_button_pressed;

BonoboWindow* 		glabels_get_active_window 		(void);
glLabel* 		glabels_get_active_label 		(void);
glView* 		glabels_get_active_view 		(void);
GList* 			glabels_get_top_windows			(void);


#endif /* __GLABELS_H__ */



