/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label.h:  GLabels label module header file
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
#ifndef __XML_LABEL_H__
#define __XML_LABEL_H__

#include "label.h"

G_BEGIN_DECLS


typedef enum {
	XML_LABEL_UNKNOWN_MEDIA = 1,
	XML_LABEL_OK = 0,
	XML_LABEL_ERROR_OPEN_PARSE = -10,
	XML_LABEL_ERROR_SAVE_FILE = -20,
} glXMLLabelStatus;


extern glLabel      *gl_xml_label_open          (const gchar * filename,
						 glXMLLabelStatus *status);
extern glLabel      *gl_xml_label_open_buffer   (const gchar * buffer,
						 glXMLLabelStatus *status);

extern void          gl_xml_label_save          (glLabel * label,
						 const gchar * filename,
						 glXMLLabelStatus *status);
extern gchar        *gl_xml_label_save_buffer   (glLabel * label,
						 glXMLLabelStatus *status);


G_END_DECLS


#endif /* __XML_LABEL_H__ */
