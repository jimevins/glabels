/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  template.h:  template module header file
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

#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

#include <libgnome/libgnome.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

typedef enum {
	GL_TEMPLATE_STYLE_RECT,
	GL_TEMPLATE_STYLE_ROUND,
	GL_TEMPLATE_STYLE_CD,
} glTemplateStyle;

typedef struct {
	GList *name;
	gchar *description;
	gchar *page_size;
	glTemplateStyle style;

	/* Suggested margin */
	gdouble label_margin;

	/* Simple and rounded rectangles. */
	gdouble label_width, label_height, label_round;

	/* CD/DVD labels */
	gdouble label_radius, label_hole;

	/* Layout */
	gint nx, ny;
	gdouble x0, y0, dx, dy;
} glTemplate;

extern void       gl_template_init                (void);

extern GList      *gl_template_get_page_size_list (void);
extern void       gl_template_free_page_size_list (GList **sizes);

extern GList      *gl_template_get_name_list      (const gchar *page_size);
extern void       gl_template_free_name_list      (GList **names);

extern glTemplate *gl_template_from_name          (const gchar * name);

extern glTemplate *gl_template_dup                (const glTemplate *orig);
extern void       gl_template_free                (glTemplate **template);

extern glTemplate *gl_template_xml_parse_sheet    (xmlNodePtr sheet_node);

extern void       gl_template_xml_add_sheet       (glTemplate * template,
						   xmlNodePtr root,
						   xmlNsPtr ns);
#endif
