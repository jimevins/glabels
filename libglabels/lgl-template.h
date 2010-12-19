/*
 *  lgl-template.h
 *  Copyright (C) 2001-2010  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of libglabels.
 *
 *  libglabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LGL_TEMPLATE_H__
#define __LGL_TEMPLATE_H__

#include <glib.h>
#include "lgl-units.h"

G_BEGIN_DECLS

typedef struct _lglTemplate                lglTemplate;

typedef union  _lglTemplateFrame           lglTemplateFrame;
typedef struct _lglTemplateFrameAll        lglTemplateFrameAll;
typedef struct _lglTemplateFrameRect       lglTemplateFrameRect;
typedef struct _lglTemplateFrameEllipse    lglTemplateFrameEllipse;
typedef struct _lglTemplateFrameRound      lglTemplateFrameRound;
typedef struct _lglTemplateFrameCD         lglTemplateFrameCD;

typedef struct _lglTemplateLayout          lglTemplateLayout;

typedef union  _lglTemplateMarkup          lglTemplateMarkup;
typedef struct _lglTemplateMarkupMargin    lglTemplateMarkupMargin;
typedef struct _lglTemplateMarkupLine      lglTemplateMarkupLine;
typedef struct _lglTemplateMarkupCircle    lglTemplateMarkupCircle;
typedef struct _lglTemplateMarkupRect      lglTemplateMarkupRect;
typedef struct _lglTemplateMarkupEllipse   lglTemplateMarkupEllipse;

typedef struct _lglTemplateOrigin          lglTemplateOrigin;

/*
 *   Top-level Template Structure
 */
struct _lglTemplate {

        gchar               *brand;
        gchar               *part;
        gchar               *equiv_part;

        gchar               *description;
        gchar               *paper_id;
        gdouble              page_width;
        gdouble              page_height;

        /* Meta information. */
        gchar               *product_url;   /* URL to manufacturer's product website. */
        GList               *category_ids;  /* List of (gchar *) category ids. */

        /* List of (lglTemplateFrame *) label frame structures.
         * Currently glabels only supports a single label frame per
         * template. */
        GList               *frames;

};


/*
 *   Possible Frame Shapes
 */
typedef enum {
        LGL_TEMPLATE_FRAME_SHAPE_RECT,
        LGL_TEMPLATE_FRAME_SHAPE_ELLIPSE,
        LGL_TEMPLATE_FRAME_SHAPE_ROUND,
        LGL_TEMPLATE_FRAME_SHAPE_CD,
} lglTemplateFrameShape;


/*
 *   Frame Structure
 */
struct _lglTemplateFrameAll {

        /* Begin Common Fields */
        lglTemplateFrameShape shape;

        gchar                *id;       /* Id, currently always "0" */
        GList                *layouts;  /* List of lglTemplateLayouts */
        GList                *markups;  /* List of lglTemplateMarkups */
        /* End Common Fields */
};

struct _lglTemplateFrameRect {

        /* Begin Common Fields */
        lglTemplateFrameShape shape;    /* Always LGL_TEMPLATE_FRAME_SHAPE_RECT. */

        gchar                *id;       /* Id, currently always "0" */
        GList                *layouts;  /* List of lglTemplateLayouts */
        GList                *markups;  /* List of lglTemplateMarkups */
        /* End Common Fields */

        gdouble               w;        /* Width */
        gdouble               h;        /* Height */
        gdouble               r;        /* Corner radius */
        gdouble               x_waste;  /* Amount of horiz overprint allowed. */
        gdouble               y_waste;  /* Amount of vert overprint allowed. */
};

struct _lglTemplateFrameEllipse {

        /* Begin Common Fields */
        lglTemplateFrameShape shape;    /* Always LGL_TEMPLATE_FRAME_SHAPE_ELLIPSE. */

        gchar                *id;       /* Id, currently always "0" */
        GList                *layouts;  /* List of lglTemplateLayouts */
        GList                *markups;  /* List of lglTemplateMarkups */
        /* End Common Fields */

        gdouble               w;        /* Width */
        gdouble               h;        /* Height */
        gdouble               waste;    /* Amount of overprint allowed. */
};

struct _lglTemplateFrameRound {

        /* Begin Common Fields */
        lglTemplateFrameShape shape;    /* Always LGL_TEMPLATE_FRAME_SHAPE_ROUND. */

        gchar                *id;       /* Id, currently always "0" */
        GList                *layouts;  /* List of lglTemplateLayouts */
        GList                *markups;  /* List of lglTemplateMarkups */
        /* End Common Fields */

        gdouble               r;      /* Radius */
        gdouble               waste;  /* Amount of overprint allowed. */
};

struct _lglTemplateFrameCD {

        /* Begin Common Fields */
        lglTemplateFrameShape shape;    /* Always LGL_TEMPLATE_FRAME_SHAPE_CD. */

        gchar                *id;       /* Id, currently always "0" */
        GList                *layouts;  /* List of lglTemplateLayouts */
        GList                *markups;  /* List of lglTemplateMarkups */
        /* End Common Fields */

        gdouble               r1;     /* Outer radius */
        gdouble               r2;     /* Inner radius (hole) */
        gdouble               w;      /* Clip width, business card CDs */
        gdouble               h;      /* Clip height, business card CDs */
        gdouble               waste;  /* Amount of overprint allowed. */
};

union _lglTemplateFrame{

        lglTemplateFrameShape   shape;

        lglTemplateFrameAll     all;
        lglTemplateFrameRect    rect;
        lglTemplateFrameEllipse ellipse;
        lglTemplateFrameRound   round;
        lglTemplateFrameCD      cd;
};


/*
 *   Label Layout Structure
 */
struct _lglTemplateLayout {

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
        LGL_TEMPLATE_MARKUP_MARGIN,
        LGL_TEMPLATE_MARKUP_LINE,
        LGL_TEMPLATE_MARKUP_CIRCLE,
        LGL_TEMPLATE_MARKUP_RECT,
        LGL_TEMPLATE_MARKUP_ELLIPSE,
} lglTemplateMarkupType;


/*
 *   Label Markup Structure (Helpful lines drawn in glabels to help locate objects)
 */
struct _lglTemplateMarkupMargin {

        lglTemplateMarkupType  type;  /* Always LGL_TEMPLATE_MARKUP_MARGIN */

        gdouble                size;  /* Margin size */
};

struct _lglTemplateMarkupLine {

        lglTemplateMarkupType  type;   /* Always LGL_TEMPLATE_MARKUP_LINE */

        gdouble                x1, y1; /* 1st endpoint */
        gdouble                x2, y2; /* 2nd endpoint */
};

struct _lglTemplateMarkupCircle {

        lglTemplateMarkupType  type;   /* Always LGL_TEMPLATE_MARKUP_CIRCLE */

        gdouble                x0, y0; /* Center of circle */
        gdouble                r;      /* Radius of circle */
};

struct _lglTemplateMarkupRect {

        lglTemplateMarkupType  type;   /* Always LGL_TEMPLATE_MARKUP_RECT */

        gdouble                x1, y1; /* Upper left corner */
        gdouble                w, h;   /* Width and height. */
        gdouble                r;      /* Radius of corners. */
};

struct _lglTemplateMarkupEllipse {

        lglTemplateMarkupType  type;   /* Always LGL_TEMPLATE_MARKUP_ELLIPSE */

        gdouble                x1, y1; /* Upper left corner */
        gdouble                w, h;   /* Width and height. */
};

union _lglTemplateMarkup {

        lglTemplateMarkupType    type;

        lglTemplateMarkupMargin  margin;
        lglTemplateMarkupLine    line;
        lglTemplateMarkupCircle  circle;
        lglTemplateMarkupRect    rect;
        lglTemplateMarkupEllipse ellipse;
};


/*
 *  Origin coordinates
 */
struct _lglTemplateOrigin {

        gdouble               x, y; /* Label origin relative to upper 
                                     * upper left hand corner of paper */

};



/* 
 * Template query functions
 */
gchar                     *lgl_template_get_name                (const lglTemplate   *template);

gboolean                   lgl_template_do_templates_match      (const lglTemplate   *template1,
                                                                 const lglTemplate   *template2);

gboolean                   lgl_template_does_brand_match        (const lglTemplate   *template,
                                                                 const gchar         *brand);

gboolean                   lgl_template_does_page_size_match    (const lglTemplate   *template,
                                                                 const gchar         *paper_id);

gboolean                   lgl_template_does_category_match     (const lglTemplate   *template,
                                                                 const gchar         *category_id);

gboolean                   lgl_template_are_templates_identical (const lglTemplate   *template1,
                                                                 const lglTemplate   *template2);




/*
 * Frame query functions
 */
void                 lgl_template_frame_get_size       (const lglTemplateFrame    *frame,
                                                        gdouble                   *w,
                                                        gdouble                   *h);

gint                 lgl_template_frame_get_n_labels   (const lglTemplateFrame    *frame);

lglTemplateOrigin   *lgl_template_frame_get_origins    (const lglTemplateFrame    *frame);

gchar               *lgl_template_frame_get_layout_description (const lglTemplateFrame *frame);

gchar               *lgl_template_frame_get_size_description   (const lglTemplateFrame *frame,
                                                                lglUnits                units);




/*
 * Template Construction
 */
lglTemplate         *lgl_template_new                  (const gchar          *brand,
                                                        const gchar          *part,
                                                        const gchar          *description,
                                                        const gchar          *paper_id,
                                                        gdouble               page_width,
                                                        gdouble               page_height);

lglTemplate         *lgl_template_new_from_equiv       (const gchar          *brand,
                                                        const gchar          *part,
                                                        const gchar          *equiv_part);

void                 lgl_template_add_category         (lglTemplate          *template,
                                                        const gchar          *category_id);

void                 lgl_template_add_frame            (lglTemplate          *template,
                                                        lglTemplateFrame     *frame);

lglTemplateFrame    *lgl_template_frame_rect_new       (const gchar          *id,
                                                        gdouble               w,
                                                        gdouble               h,
                                                        gdouble               r,
                                                        gdouble               x_waste,
                                                        gdouble               y_waste);

lglTemplateFrame    *lgl_template_frame_ellipse_new    (const gchar          *id,
                                                        gdouble               w,
                                                        gdouble               h,
                                                        gdouble               waste);

lglTemplateFrame    *lgl_template_frame_round_new      (const gchar          *id,
                                                        gdouble               r,
                                                        gdouble               waste);

lglTemplateFrame    *lgl_template_frame_cd_new         (const gchar          *id,
                                                        gdouble               r1,
                                                        gdouble               r2,
                                                        gdouble               w,
                                                        gdouble               h,
                                                        gdouble               waste);

void                 lgl_template_frame_add_layout     (lglTemplateFrame     *frame,
                                                        lglTemplateLayout    *layout);

void                 lgl_template_frame_add_markup     (lglTemplateFrame     *frame,
                                                        lglTemplateMarkup    *markup);

lglTemplateLayout   *lgl_template_layout_new           (gint                  nx,
                                                        gint                  ny,
                                                        gdouble               x0,
                                                        gdouble               y0,
                                                        gdouble               dx,
                                                        gdouble               dy);

lglTemplateMarkup   *lgl_template_markup_margin_new    (gdouble               size);

lglTemplateMarkup   *lgl_template_markup_line_new      (gdouble               x1,
                                                        gdouble               y1,
                                                        gdouble               x2,
                                                        gdouble               y2);

lglTemplateMarkup   *lgl_template_markup_circle_new    (gdouble               x0,
                                                        gdouble               y0,
                                                        gdouble               r);

lglTemplateMarkup   *lgl_template_markup_rect_new      (gdouble               x1,
                                                        gdouble               y1,
                                                        gdouble               w,
                                                        gdouble               h,
                                                        gdouble               r);

lglTemplateMarkup   *lgl_template_markup_ellipse_new   (gdouble               x1,
                                                        gdouble               y1,
                                                        gdouble               w,
                                                        gdouble               h);

lglTemplate         *lgl_template_dup                  (const lglTemplate    *orig_template);

void                 lgl_template_free                 (lglTemplate          *template);

lglTemplateFrame    *lgl_template_frame_dup            (const lglTemplateFrame     *orig_frame);
void                 lgl_template_frame_free           (lglTemplateFrame           *frame);

lglTemplateLayout   *lgl_template_layout_dup           (const lglTemplateLayout    *orig_layout);
void                 lgl_template_layout_free          (lglTemplateLayout          *layout);

lglTemplateMarkup   *lgl_template_markup_dup           (const lglTemplateMarkup    *orig_markup);
void                 lgl_template_markup_free          (lglTemplateMarkup          *markup);


/*
 * Debugging functions.
 */
void                 lgl_template_print                (const lglTemplate          *template);


G_END_DECLS

#endif /* __LGL_TEMPLATE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
