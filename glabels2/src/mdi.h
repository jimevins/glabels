/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mdi.h:  gLabels MDI module header file
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
 * This file is based on gedit-mdi.h from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 * Copyright (C) 2002  Paolo Maggi 
 *
 */
#ifndef __MDI_H__
#define __MDI_H__

#include <bonobo-mdi.h>

#define GL_TYPE_MDI			(gl_mdi_get_type ())
#define GL_MDI(obj)			(GTK_CHECK_CAST ((obj), GL_TYPE_MDI, glMDI))
#define GL_MDI_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_MDI, glMDIClass))
#define GL_IS_MDI(obj)		(GTK_CHECK_TYPE ((obj), GL_TYPE_MDI))
#define GL_IS_MDI_CLASS(klass)  	(GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_MDI))
#define GL_MDI_GET_CLASS(obj)  	(GTK_CHECK_GET_CLASS ((obj), GL_TYPE_MDI, glMdiClass))


typedef struct _glMDI		        glMDI;
typedef struct _glMDIClass		glMDIClass;

typedef struct _glMDIPrivate		glMDIPrivate;

struct _glMDI
{
	BonoboMDI mdi;
	
	glMDIPrivate *priv;
};

struct _glMDIClass
{
	BonoboMDIClass parent_class;
};


GtkType        	gl_mdi_get_type 	                   (void) G_GNUC_CONST;

glMDI*	        gl_mdi_new                                 (void);

void		gl_mdi_set_active_window_title             (BonoboMDI *mdi);

void		gl_mdi_update_ui_according_to_preferences  (glMDI *mdi);

void 		gl_mdi_set_active_window_verbs_sensitivity (BonoboMDI *mdi);

#endif /* __MDI_H__ */

