/*
 *  xml-label.h
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

#ifndef __XML_LABEL_H__
#define __XML_LABEL_H__

#include "label.h"

G_BEGIN_DECLS


typedef enum {
	XML_LABEL_UNKNOWN_MEDIA = 1,
	XML_LABEL_OK = 0,
	XML_LABEL_ERROR_OPEN_PARSE = -10,
	XML_LABEL_ERROR_SAVE_FILE = -20
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



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
