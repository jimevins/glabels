/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mdi-child.h:  gLabels MDI child module header file
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
 * This file is based on gedit-mdi-child.h from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 * Copyright (C) 2002  Paolo Maggi 
 *
 */
#ifndef __MDI_CHILD_H__
#define __MDI_CHILD_H__

#include <bonobo-mdi.h>

#include "label.h"

#define GL_TYPE_MDI_CHILD	(gl_mdi_child_get_type ())
#define GL_MDI_CHILD(o)		(GTK_CHECK_CAST ((o), GL_TYPE_MDI_CHILD, glMDIChild))
#define GL_MDI_CHILD_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_MDI_CHILD, \
					glMDIChildClass))
#define GL_IS_MDI_CHILD(o)		(GTK_CHECK_TYPE ((o), GL_TYPE_MDI_CHILD))
#define GL_IS_MDI_CHILD_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_MDI_CHILD))
#define GL_MDI_CHILD_GET_CLASS(o)    (GTK_CHECK_GET_CLASS ((o), GL_TYPE_MDI_CHILD, glMdiChildClass))


typedef struct _glMDIChild	  glMDIChild;
typedef struct _glMDIChildClass   glMDIChildClass;

typedef struct _glMDIChildPrivate glMDIChildPrivate;

struct _glMDIChild
{
	BonoboMDIChild child;
	
	glLabel* label;
	glMDIChildPrivate *priv;
};

struct _glMDIChildClass
{
	BonoboMDIChildClass parent_class;

	/* MDI child state changed */
	void (*state_changed)		(glMDIChild *child);

	void (*undo_redo_state_changed)	(glMDIChild *child);
};


GtkType        	gl_mdi_child_get_type 	(void) G_GNUC_CONST;

glMDIChild*	gl_mdi_child_new	(const gchar *tmplt_name,
					 gboolean rotate_flag);

glMDIChild*	gl_mdi_child_new_with_uri (const gchar *uri, GError **error);

#endif /* __MDI_CHILD_H__ */

