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

G_BEGIN_DECLS

/*
 *   Template Label Structure
 */
typedef enum {
	GL_TEMPLATE_STYLE_RECT,
	GL_TEMPLATE_STYLE_ROUND,
	GL_TEMPLATE_STYLE_CD,
} glTemplateLabelStyle;

typedef struct {
	glTemplateLabelStyle  style;
	GList                *layouts;  /* List of glTemplateLayouts */
	GList                *markups;  /* List of glTemplateMarkups */
} glTemplateLabelParent;

typedef struct {
	glTemplateLabelParent parent;

	gdouble               w, h, r;  /* Dimensions */
	gdouble               waste;
} glTemplateLabelRect;

typedef struct {
	glTemplateLabelParent parent;

	gdouble               r;        /* Dimensions */
	gdouble               waste;
} glTemplateLabelRound;

typedef struct {
	glTemplateLabelParent parent;

	gdouble               r1, r2, w, h; /* Dimensions, w&h are for business card CDs */
	gdouble               waste;
} glTemplateLabelCD;

typedef union {
	glTemplateLabelStyle  style;
	glTemplateLabelParent any;
	glTemplateLabelRect   rect;
	glTemplateLabelRound  round;
	glTemplateLabelCD     cd;
} glTemplateLabel;


/*
 *   Label Markup
 */
typedef enum {
	GL_TEMPLATE_MARKUP_MARGIN,
	GL_TEMPLATE_MARKUP_LINE,
	GL_TEMPLATE_MARKUP_CIRCLE,
} glTemplateMarkupType;

typedef struct {
	/* NOTE: These fields are common to all union members. */
	glTemplateMarkupType   type;
} glTemplateMarkupParent;

typedef struct {
	glTemplateMarkupParent parent;

	gdouble                size;
} glTemplateMarkupMargin;

typedef struct {
	glTemplateMarkupParent parent;

	gdouble                x1, y1, x2, y2;
} glTemplateMarkupLine;

typedef struct {
	glTemplateMarkupParent parent;

	gdouble                x0, y0, r;
} glTemplateMarkupCircle;

typedef union {
	glTemplateMarkupType   type;
	glTemplateMarkupParent any;
	glTemplateMarkupMargin margin;
	glTemplateMarkupLine   line;
	glTemplateMarkupCircle circle;
} glTemplateMarkup;


/*
 *   Label layout
 */
typedef struct {
	gint nx, ny;
	gdouble x0, y0, dx, dy;
} glTemplateLayout;


/*
 *   Template
 */
typedef struct {
	gchar               *name;
	gchar               *description;
	gchar               *page_size;
	gdouble              page_width;
	gdouble              page_height;

	glTemplateLabel      label;

	GList               *alias;
} glTemplate;


/*
 *  Origin coordinates
 */
typedef struct {
	gdouble x, y;
} glTemplateOrigin;


void                 gl_template_init                (void);

GList               *gl_template_get_name_list       (const gchar       *page_size);
void                 gl_template_free_name_list      (GList            **names);

void                 gl_template_register            (const glTemplate  *template);

glTemplate          *gl_template_from_name           (const gchar       *name);

gchar               *gl_template_get_name_with_desc  (const glTemplate  *template);

glTemplate          *gl_template_dup                 (const glTemplate  *orig);
void                 gl_template_free                (glTemplate       **template);

gchar               *gl_template_get_label_size_desc (const glTemplate  *template);
void                 gl_template_get_label_size      (const glTemplate  *template,
						      gdouble           *w,
						      gdouble           *h);

gint                 gl_template_get_n_labels        (const glTemplate  *template);
glTemplateOrigin    *gl_template_get_origins         (const glTemplate  *template);
gchar               *gl_template_get_layout_desc     (const glTemplate  *template);

glTemplateLayout    *gl_template_layout_new          (gdouble            nx,
						      gdouble            ny,
						      gdouble            x0,
						      gdouble            y0,
						      gdouble            dx,
						      gdouble            dy);

glTemplateMarkup    *gl_template_markup_margin_new   (gdouble            size);
glTemplateMarkup    *gl_template_markup_line_new     (gdouble            x1,
						      gdouble            y1,
						      gdouble            x2,
						      gdouble            y2);
glTemplateMarkup    *gl_template_markup_circle_new   (gdouble            x0,
						      gdouble            y0,
						      gdouble            r);


G_END_DECLS

#endif
