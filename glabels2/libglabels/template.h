/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  template.h:  template module header file
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of the LIBGLABELS library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA
 */

#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

#include <glib/gtypes.h>
#include <glib/glist.h>

G_BEGIN_DECLS

typedef struct _glTemplate          glTemplate;
typedef struct _glTemplateLabelType glTemplateLabelType;
typedef struct _glTemplateLayout    glTemplateLayout;
typedef struct _glTemplateMarkup    glTemplateMarkup;
typedef struct _glTemplateOrigin    glTemplateOrigin;

/*
 *   Top-level Template Structure
 */
struct _glTemplate {

	gchar               *name;
	gchar               *description;
	gchar               *page_size;
	gdouble              page_width;
	gdouble              page_height;

        /* List of (gchar *) category ids. */
	GList               *categories;

	/* List of (glTemplateLabelType *) label type structures.
	 * Currently glabels only supports a single label type per
	 * template. */
	GList               *label_types;

	/* List of (gchar *) aliases. */
	GList               *aliases;

};


/*
 *   Possible Label Shapes
 */
typedef enum {
	GL_TEMPLATE_SHAPE_RECT,
	GL_TEMPLATE_SHAPE_ROUND,
	GL_TEMPLATE_SHAPE_CD,
} glTemplateLabelShape;


/*
 *   Label Type Structure
 */
struct _glTemplateLabelType{

	gchar                *id;       /* Id, currently always "0" */
	GList                *layouts;  /* List of glTemplateLayouts */
	GList                *markups;  /* List of glTemplateMarkups */

	glTemplateLabelShape  shape;

	union {

		struct {
			gdouble        w;        /* Width */
			gdouble        h;        /* Height */
			gdouble        r;        /* Corner radius */
			gdouble        x_waste;  /* Amount of horiz overprint allowed. */
			gdouble        y_waste;  /* Amount of vert overprint allowed. */
		} rect;

		struct {
			gdouble        r;      /* Radius */
			gdouble        waste;  /* Amount of overprint allowed. */

		} round;

		struct {
			gdouble        r1;     /* Outer radius */
			gdouble        r2;     /* Inner radius (hole) */
			gdouble        w;      /* Clip width, business card CDs */
			gdouble        h;      /* Clip height, business card CDs */
			gdouble        waste;  /* Amount of overprint allowed. */
		} cd;

	} size;

};


/*
 *   Label Layout Structure
 */
struct _glTemplateLayout {

	gint                  nx;  /* Number of labels across */
	gint                  ny;  /* Number of labels up and down */

	gdouble               x0;  /* Left of grid from left edge of paper */
	gdouble               y0;  /* Top of grid from top edge of paper */

	gdouble               dx;  /* Horizontal pitch of grid */
	gdouble               dy;  /* Vertical pitch of grid */

};


/*
 * Possible Markup Types
 */
typedef enum {
	GL_TEMPLATE_MARKUP_MARGIN,
	GL_TEMPLATE_MARKUP_LINE,
	GL_TEMPLATE_MARKUP_CIRCLE,
} glTemplateMarkupType;


/*
 *   Label Markup Structure (Helpful lines drawn in glabels to help locate objects)
 */
struct _glTemplateMarkup {

	glTemplateMarkupType   type;

	union {

		struct {
			gdouble                size;  /* Margin size */
		} margin;

		struct {
			gdouble                x1, y1; /* 1st endpoint */
			gdouble                x2, y2; /* 2nd endpoint */
		} line;

		struct {
			gdouble                x0, y0; /* Center of circle */
			gdouble                r;      /* Radius of circle */
		} circle;
		
	} data;

};


/*
 *  Origin coordinates
 */
struct _glTemplateOrigin {

	gdouble               x, y; /* Label origin relative to upper 
				     * upper left hand corner of paper */

};



/*
 * Module Initialization
 */
void                 gl_template_init                 (void);

void                 gl_template_register             (const glTemplate    *template);

/*
 * Known templates query functions
 */
GList               *gl_template_get_name_list_unique (const gchar         *page_size,
                                                       const gchar         *category);

GList               *gl_template_get_name_list_all    (const gchar         *page_size,
                                                       const gchar         *category);

void                 gl_template_free_name_list       (GList               *names);

glTemplate          *gl_template_from_name            (const gchar         *name);


/* 
 * Template query functions
 */
const glTemplateLabelType *gl_template_get_first_label_type (const glTemplate   *template);

gboolean                   gl_template_does_page_size_match (const glTemplate   *template,
                                                             const gchar        *page_size);

gboolean                   gl_template_does_category_match  (const glTemplate   *template,
                                                             const gchar        *category);


/*
 * Label Type query functions
 */
void                 gl_template_get_label_size  (const glTemplateLabelType *label_type,
						  gdouble                   *w,
						  gdouble                   *h);

gint                 gl_template_get_n_labels    (const glTemplateLabelType *label_type);

glTemplateOrigin    *gl_template_get_origins     (const glTemplateLabelType *label_type);


/*
 * Template Construction
 */
glTemplate          *gl_template_new                  (const gchar         *name,
						       const gchar         *description,
						       const gchar         *page_size,
						       gdouble              page_width,
						       gdouble              page_height);

void                 gl_template_add_category         (glTemplate          *template,
						       const gchar         *category);

void                 gl_template_add_label_type       (glTemplate          *template,
						       glTemplateLabelType *label_type);

void                 gl_template_add_alias            (glTemplate          *template,
						       const gchar         *alias);

glTemplateLabelType *gl_template_rect_label_type_new  (const gchar         *id,
						       gdouble              w,
						       gdouble              h,
						       gdouble              r,
						       gdouble              x_waste,
						       gdouble              y_waste);

glTemplateLabelType *gl_template_round_label_type_new (const gchar         *id,
						       gdouble              r,
						       gdouble              waste);

glTemplateLabelType *gl_template_cd_label_type_new    (const gchar         *id,
						       gdouble              r1,
						       gdouble              r2,
						       gdouble              w,
						       gdouble              h,
						       gdouble              waste);

void                 gl_template_add_layout           (glTemplateLabelType *label_type,
						       glTemplateLayout    *layout);

void                 gl_template_add_markup           (glTemplateLabelType *label_type,
						       glTemplateMarkup    *markup);

glTemplateLayout    *gl_template_layout_new           (gint                 nx,
						       gint                 ny,
						       gdouble              x0,
						       gdouble              y0,
						       gdouble              dx,
						       gdouble              dy);

glTemplateMarkup    *gl_template_markup_margin_new    (gdouble              size);

glTemplateMarkup    *gl_template_markup_line_new      (gdouble              x1,
						       gdouble              y1,
						       gdouble              x2,
						       gdouble              y2);

glTemplateMarkup    *gl_template_markup_circle_new    (gdouble              x0,
						       gdouble              y0,
						       gdouble              r);

glTemplate          *gl_template_dup                  (const glTemplate    *orig);

void                 gl_template_free                 (glTemplate          *template);

glTemplateLabelType *gl_template_label_type_dup       (const glTemplateLabelType *orig_ltype);
void                 gl_template_label_type_free      (glTemplateLabelType       *ltype);

glTemplateLayout    *gl_template_layout_dup           (const glTemplateLayout    *orig_layout);
void                 gl_template_layout_free          (glTemplateLayout          *layout);

glTemplateMarkup    *gl_template_markup_dup           (const glTemplateMarkup    *orig_markup);
void                 gl_template_markup_free          (glTemplateMarkup          *markup);


/*
 * Debugging functions
 */
void                 gl_template_print_known_templates (void);
void                 gl_template_print_aliases         (const glTemplate    *template);


G_END_DECLS

#endif
