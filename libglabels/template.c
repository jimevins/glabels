/*
 *  template.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
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

#include <config.h>

#include "template.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libglabels-private.h"

#include "db.h"
#include "paper.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static gint         compare_origins              (gconstpointer           a,
                                                  gconstpointer           b,
                                                  gpointer                user_data);

/*===========================================*/
/* Functions.                                */
/*===========================================*/

/**
 * lgl_template_new:
 *   @brand:        Template brand
 *   @part:         Template part name/number
 *   @description:  Template descriptions
 *   @paper_id:     Page size id
 *   @page_width:   Page width in points, set to zero unless paper_id="Other"
 *   @page_height:  Page height in points, set to zero unless paper_id="Other"
 *
 * Create a new template structure, with the given top-level attributes.  The
 * created template will have no initial aliases, categories, or frames
 * associated with it.  See lgl_template_add_alias(), lgl_template_add_category(),
 * and lgl_template_add_frame() to add these.
 *
 * Returns: pointer to a newly allocated #lglTemplate structure.
 *
 */
lglTemplate *
lgl_template_new (const gchar         *brand,
                  const gchar         *part,
                  const gchar         *description,
                  const gchar         *paper_id,
                  gdouble              page_width,
                  gdouble              page_height)
{
	lglTemplate      *template;
	lglTemplateAlias *alias;

	template = g_new0 (lglTemplate,1);

	template->brand       = g_strdup (brand);
	template->part        = g_strdup (part);
	template->description = g_strdup (description);
	template->paper_id    = g_strdup (paper_id);
	template->page_width  = page_width;
	template->page_height = page_height;

	/* Always include primary name in alias list. */
	template->aliases = NULL;
        alias = lgl_template_alias_new (brand, part);
        lgl_template_add_alias (template, alias);

	return template;
}


/**
 * lgl_template_new_from_equiv:
 *   @brand:        Template brand
 *   @part:         Template part name/number
 *   @equiv_part:   Name of equivalent part to base template on
 *
 * Create a new template structure based on an existing template.  The
 * created template will be a duplicate of the original template, except with
 * the new part name/number.
 *
 * Returns: pointer to a newly allocated #lglTemplate structure.
 *
 */
lglTemplate *
lgl_template_new_from_equiv (const gchar          *brand,
                             const gchar          *part,
                             const gchar          *equiv_part)
{
        lglTemplate      *template;
        GList            *p_alias;
        lglTemplateAlias *alias;

        template = lgl_db_lookup_template_from_brand_part (brand, equiv_part);
        if (template)
        {
                g_free (template->part);
                g_free (template->equiv_part);

                template->part       = g_strdup (part);
                template->equiv_part = g_strdup (equiv_part);

                for ( p_alias = template->aliases; p_alias != NULL; p_alias = p_alias->next )
                {
                        alias = (lglTemplateAlias *)p_alias->data;
                        lgl_template_alias_free (alias);
                }
                g_list_free (template->aliases);
                template->aliases = NULL;

                alias = lgl_template_alias_new (brand, part);
                lgl_template_add_alias (template, alias);
        }
        else
        {
                g_message (_("Equivalent part (\"%s\") for \"%s\", not previously defined."),
                           equiv_part, part);
        }

        return template;
}


/**
 * lgl_template_get_name:
 *   @template:  Pointer to template structure to test
 *
 * This function returns the name of the given template.  The name is the concetenation
 * of the brand and part name/number.
 *
 * Returns:  A pointer to a newly allocated name string.  Should be freed with g_free().
 *
 */
gchar *
lgl_template_get_name (const lglTemplate  *template)
{
	g_return_val_if_fail (template, NULL);

        return g_strdup_printf ("%s %s", template->brand, template->part);
}


/**
 * lgl_template_do_templates_match:
 *   @template1:  Pointer to 1st template structure to test
 *   @template2:  Pointer to 2nd template structure to test
 *
 * This function tests if the given templates match.  This is a simple test that only tests
 * the brand and part name/number. It does not test if they are actually identical.
 *
 * Returns:  TRUE if the two template matche.
 *
 */
gboolean
lgl_template_do_templates_match (const lglTemplate  *template1,
                                 const lglTemplate  *template2)
{
	g_return_val_if_fail (template1, FALSE);
	g_return_val_if_fail (template2, FALSE);

        return (UTF8_EQUAL (template1->brand, template2->brand) &&
                UTF8_EQUAL (template1->part, template2->part));
}


/**
 * lgl_template_does_brand_match:
 *   @template:  Pointer to template structure to test
 *   @brand:     Brand string
 *
 * This function tests if the brand of the template matches the given brand.
 *
 * Returns:  TRUE if the template matches the given brand.
 *
 */
gboolean
lgl_template_does_brand_match (const lglTemplate  *template,
                               const gchar        *brand)
{
	g_return_val_if_fail (template, FALSE);

        /* NULL matches everything. */
        if (brand == NULL)
        {
                return TRUE;
        }

        return UTF8_EQUAL (template->brand, brand);
}


/**
 * lgl_template_does_page_size_match:
 *   @template:  Pointer to template structure to test
 *   @paper_id:  Page size ID string
 *
 * This function tests if the page size of the template matches the given ID.
 *
 * Returns:  TRUE if the template matches the given page size ID.
 *
 */
gboolean
lgl_template_does_page_size_match (const lglTemplate  *template,
                                   const gchar        *paper_id)
{
	g_return_val_if_fail (template, FALSE);

        /* NULL matches everything. */
        if (paper_id == NULL)
        {
                return TRUE;
        }

        return ASCII_EQUAL(paper_id, template->paper_id);
}


/**
 * lgl_template_does_category_match:
 *   @template:     Pointer to template structure to test
 *   @category_id:  Category ID string
 *
 * This function tests if the given template belongs to the given category ID.
 *
 * Returns:  TRUE if the template matches the given category ID.
 *
 */
gboolean
lgl_template_does_category_match  (const lglTemplate  *template,
                                   const gchar        *category_id)
{
        GList *p;

	g_return_val_if_fail (template, FALSE);

        /* NULL matches everything. */
        if (category_id == NULL)
        {
                return TRUE;
        }

        for ( p=template->category_ids; p != NULL; p=p->next )
        {
                if (ASCII_EQUAL(category_id, p->data))
                {
                        return TRUE;
                }
        }

        return FALSE;
}


/**
 * lgl_template_alias_new:
 *   @brand:        Alias brand
 *   @part:         Alias part name/number
 *
 * Create a new template alias structure, with the given brand and part number.
 *
 * Returns: pointer to a newly allocated #lglTemplateAlias structure.
 *
 */
lglTemplateAlias *
lgl_template_alias_new (const gchar         *brand,
                        const gchar         *part)
{
	lglTemplateAlias *alias;

	alias = g_new0 (lglTemplateAlias,1);

	alias->brand       = g_strdup (brand);
	alias->part        = g_strdup (part);

	return alias;
}


/**
 * lgl_template_add_alias:
 *   @template:  Pointer to template structure
 *   @alias:     Alias string
 *
 * This function adds the given alias to a templates list of aliases.
 *
 */
void
lgl_template_add_alias (lglTemplate         *template,
                        lglTemplateAlias    *alias)
{
	g_return_if_fail (template);
	g_return_if_fail (alias);

	template->aliases = g_list_append (template->aliases, alias);
}
 

/**
 * lgl_template_add_frame:
 *   @template:  Pointer to template structure
 *   @frame:     Pointer to frame structure
 *
 * This function adds the given frame structure to the template.  Once added,
 * the frame structure belongs to the given template; do not attempt to free
 * it.
 *
 * Note: Currently glabels only supports a single frame per template.
 *
 */
void
lgl_template_add_frame (lglTemplate      *template,
                        lglTemplateFrame *frame)
{
	g_return_if_fail (template);
	g_return_if_fail (frame);

	template->frames = g_list_append (template->frames, frame);
}

 
/**
 * lgl_template_add_category:
 *   @template:     Pointer to template structure
 *   @category_id:  Category ID string
 *
 * This function adds the given category ID to a templates category list.
 *
 */
void
lgl_template_add_category (lglTemplate         *template,
                           const gchar         *category_id)
{
	g_return_if_fail (template);
	g_return_if_fail (category_id);

	template->category_ids = g_list_append (template->category_ids,
                                                g_strdup (category_id));
}

 
/**
 * lgl_template_frame_rect_new:
 *   @id:      ID of frame.  (This should currently always be "0").
 *   @w:       width of frame in points.
 *   @h:       height of frame in points.
 *   @r:       radius of rounded corners in points.  (Should be 0 for square corners.)
 *   @x_waste: Amount of overprint to allow in the horizontal direction.
 *   @y_waste: Amount of overprint to allow in the vertical direction.
 *
 * This function creates a new template frame for a rectangular label or card.
 *
 * Returns: Pointer to newly allocated #lglTemplateFrame structure.
 *
 */
lglTemplateFrame *
lgl_template_frame_rect_new  (const gchar         *id,
                              gdouble              w,
                              gdouble              h,
                              gdouble              r,
                              gdouble              x_waste,
                              gdouble              y_waste)
{
	lglTemplateFrame *frame;

	frame = g_new0 (lglTemplateFrame, 1);

	frame->shape = LGL_TEMPLATE_FRAME_SHAPE_RECT;
	frame->rect.id = g_strdup (id);

	frame->rect.w = w;
	frame->rect.h = h;
	frame->rect.r = r;
	frame->rect.x_waste = x_waste;
	frame->rect.y_waste = y_waste;

	return frame;
}


/**
 * lgl_template_frame_ellipse_new:
 *   @id:      ID of frame.  (This should currently always be "0").
 *   @w:       width of frame in points.
 *   @h:       height of frame in points.
 *   @r:       radius of rounded corners in points.  (Should be 0 for square corners.)
 *   @x_waste: Amount of overprint to allow in the horizontal direction.
 *   @y_waste: Amount of overprint to allow in the vertical direction.
 *
 * This function creates a new template frame for an elliptical label or card.
 *
 * Returns: Pointer to newly allocated #lglTemplateFrame structure.
 *
 */
lglTemplateFrame *
lgl_template_frame_ellipse_new  (const gchar         *id,
                                 gdouble              w,
                                 gdouble              h,
                                 gdouble              waste)
{
	lglTemplateFrame *frame;

	frame = g_new0 (lglTemplateFrame, 1);

	frame->shape = LGL_TEMPLATE_FRAME_SHAPE_ELLIPSE;
	frame->ellipse.id = g_strdup (id);

	frame->ellipse.w = w;
	frame->ellipse.h = h;
	frame->ellipse.waste = waste;

	return frame;
}


/**
 * lgl_template_frame_round_new:
 *   @id:      ID of frame.  (This should currently always be "0").
 *   @r:       radius of label in points.
 *   @waste:   Amount of overprint to allow.
 *
 * This function creates a new template frame for a round label.
 *
 * Returns: Pointer to newly allocated #lglTemplateFrame structure.
 *
 */
lglTemplateFrame *
lgl_template_frame_round_new (const gchar         *id,
                              gdouble              r,
                              gdouble              waste)
{
	lglTemplateFrame *frame;

	frame = g_new0 (lglTemplateFrame, 1);

	frame->shape = LGL_TEMPLATE_FRAME_SHAPE_ROUND;
	frame->round.id = g_strdup (id);

	frame->round.r = r;
	frame->round.waste = waste;

	return frame;
}

                                                                               
/**
 * lgl_template_frame_cd_new:
 *   @id:      ID of frame.  (This should currently always be "0").
 *   @r1:      outer radius of label in points.
 *   @r2:      radius of center hole in points.
 *   @w:       clip width of frame in points for business card CDs.  Should be 0 for no clipping.
 *   @h:       clip height of frame in points for business card CDs.  Should be 0 for no clipping.
 *   @waste:   Amount of overprint to allow.
 *
 * This function creates a new template frame for a CD/DVD label.
 *
 * Returns: Pointer to newly allocated #lglTemplateFrame structure.
 *
 */
lglTemplateFrame *
lgl_template_frame_cd_new (const gchar         *id,
                           gdouble              r1,
                           gdouble              r2,
                           gdouble              w,
                           gdouble              h,
                           gdouble              waste)
{
	lglTemplateFrame *frame;

	frame = g_new0 (lglTemplateFrame, 1);

	frame->shape = LGL_TEMPLATE_FRAME_SHAPE_CD;
	frame->cd.id = g_strdup (id);

	frame->cd.r1 = r1;
	frame->cd.r2 = r2;
	frame->cd.w  = w;
	frame->cd.h  = h;
	frame->cd.waste = waste;

	return frame;
}


/**
 * lgl_template_frame_get_size:
 * @frame: #lglTemplateFrame structure to query
 * @w: pointer to location to receive width of frame
 * @h: pointer to location to receive height of frame
 *
 * Get size (width and height) of given #lglTemplateFrame in points.
 *
 */
void
lgl_template_frame_get_size (const lglTemplateFrame *frame,
                             gdouble                *w,
                             gdouble                *h)
{
	g_return_if_fail (frame);

	switch (frame->shape) {
	case LGL_TEMPLATE_FRAME_SHAPE_RECT:
		*w = frame->rect.w;
		*h = frame->rect.h;
		break;
	case LGL_TEMPLATE_FRAME_SHAPE_ELLIPSE:
		*w = frame->ellipse.w;
		*h = frame->ellipse.h;
		break;
	case LGL_TEMPLATE_FRAME_SHAPE_ROUND:
		*w = 2.0 * frame->round.r;
		*h = 2.0 * frame->round.r;
		break;
	case LGL_TEMPLATE_FRAME_SHAPE_CD:
		if (frame->cd.w == 0.0) {
			*w = 2.0 * frame->cd.r1;
		} else {
			*w = frame->cd.w;
		}
		if (frame->cd.h == 0.0) {
			*h = 2.0 * frame->cd.r1;
		} else {
			*h = frame->cd.h;
		}
		break;
	default:
		*w = 0.0;
		*h = 0.0;
		break;
	}
}


/**
 * lgl_template_frame_get_n_labels:
 * @frame: #lglTemplateFrame structure to query
 *
 * Get total number of labels per sheet corresponding to the given frame.
 *
 * Returns: number of labels per sheet.
 *
 */
gint
lgl_template_frame_get_n_labels (const lglTemplateFrame *frame)
{
	gint               n_labels = 0;
	GList             *p;
	lglTemplateLayout *layout;

	g_return_val_if_fail (frame, 0);

	for ( p=frame->all.layouts; p != NULL; p=p->next ) {
		layout = (lglTemplateLayout *)p->data;

		n_labels += layout->nx * layout->ny;
	}

	return n_labels;
}


/**
 * lgl_template_frame_get_origins:
 * @frame: #lglTemplateFrame structure to query
 *
 * Get an array of label origins for the given frame.  These origins represent the
 * upper left hand corner of each label on a page corresponding to the given frame.
 * The origins will be ordered geometrically left to right and then top to bottom.
 * The array should be freed using g_free().
 *
 * Returns: A newly allocated array of #lglTemplateOrigin structures.
 *
 */
lglTemplateOrigin *
lgl_template_frame_get_origins (const lglTemplateFrame *frame)
{
	gint               i_label, n_labels, ix, iy;
	lglTemplateOrigin *origins;
	GList             *p;
	lglTemplateLayout *layout;

	g_return_val_if_fail (frame, NULL);

	n_labels = lgl_template_frame_get_n_labels (frame);
	origins = g_new0 (lglTemplateOrigin, n_labels);

	i_label = 0;
	for ( p=frame->all.layouts; p != NULL; p=p->next ) {
		layout = (lglTemplateLayout *)p->data;

		for (iy = 0; iy < layout->ny; iy++) {
			for (ix = 0; ix < layout->nx; ix++, i_label++) {
				origins[i_label].x = ix*layout->dx + layout->x0;
				origins[i_label].y = iy*layout->dy + layout->y0;
			}
		}
	}

	g_qsort_with_data (origins, n_labels, sizeof(lglTemplateOrigin),
			   compare_origins, NULL);

	return origins;
}


/**
 * lgl_template_frame_add_layout:
 *   @frame:  Pointer to template frame to add layout to.
 *   @layout: Pointer to layout structure to add to frame.
 *
 * This function adds a layout structure to the given template frame.
 *
 */
void
lgl_template_frame_add_layout (lglTemplateFrame   *frame,
                               lglTemplateLayout  *layout)
{
	g_return_if_fail (frame);
	g_return_if_fail (layout);

	frame->all.layouts = g_list_append (frame->all.layouts, layout);
}
 

/**
 * lgl_template_frame_add_markup:
 *   @frame:  Pointer to template frame to add markup to.
 *   @markup: Pointer to markup structure to add to frame.
 *
 * This function adds a markup structure to the given template frame.
 *
 */
void
lgl_template_frame_add_markup (lglTemplateFrame   *frame,
                               lglTemplateMarkup  *markup)
{
	g_return_if_fail (frame);
	g_return_if_fail (markup);

	frame->all.markups = g_list_append (frame->all.markups, markup);
}
 

/**
 * lgl_template_layout_new:
 *   @nx:  Number of labels across.
 *   @ny:  Number of labels down.
 *   @x0:  X coordinate of the top-left corner of the top-left label in the layout in points.
 *   @y0:  Y coordinate of the top-left corner of the top-left label in the layout in points.
 *   @dx:  Horizontal pitch in points.  This is the distance from left-edge to left-edge.
 *   @dy:  Vertical pitch in points.  This is the distance from top-edge to top-edge.
 *
 * This function creates a new layout structure with the given parameters.
 *
 * Returns: a newly allocated #lglTemplateLayout structure.
 *
 */
lglTemplateLayout *
lgl_template_layout_new (gint    nx,
                         gint    ny,
                         gdouble x0,
                         gdouble y0,
                         gdouble dx,
                         gdouble dy)
{
	lglTemplateLayout *layout;

	layout = g_new0 (lglTemplateLayout, 1);

	layout->nx = nx;
	layout->ny = ny;
	layout->x0 = x0;
	layout->y0 = y0;
	layout->dx = dx;
	layout->dy = dy;

	return layout;
}


/**
 * lgl_template_markup_margin_new:
 *   @size: margin size in points.
 *
 * This function creates a new margin markup structure.
 *
 * Returns: a newly allocated #lglTemplateMarkup structure.
 *
 */
lglTemplateMarkup *
lgl_template_markup_margin_new (gdouble size)
{
	lglTemplateMarkup *markup;

	markup = g_new0 (lglTemplateMarkup, 1);

	markup->type        = LGL_TEMPLATE_MARKUP_MARGIN;
	markup->margin.size = size;

	return markup;
}


/**
 * lgl_template_markup_line_new:
 *   @x1: x coordinate of first endpoint.
 *   @y1: y coordinate of first endpoint.
 *   @x2: x coordinate of second endpoint.
 *   @y2: y coordinate of second endpoint.
 *
 * This function creates a new line markup structure.
 *
 * Returns: a newly allocated #lglTemplateMarkup structure.
 *
 */
lglTemplateMarkup *
lgl_template_markup_line_new (gdouble x1,
                              gdouble y1,
                              gdouble x2,
                              gdouble y2)
{
	lglTemplateMarkup *markup;

	markup = g_new0 (lglTemplateMarkup, 1);

	markup->type        = LGL_TEMPLATE_MARKUP_LINE;
	markup->line.x1     = x1;
	markup->line.y1     = y1;
	markup->line.x2     = x2;
	markup->line.y2     = y2;

	return markup;
}


/**
 * lgl_template_markup_circle_new:
 *   @x0: x coordinate of center of circle.
 *   @y0: y coordinate of center of circle.
 *   @r:  radius of circle.
 *
 * This function creates a new circle markup structure.
 *
 * Returns: a newly allocated #lglTemplateMarkup structure.
 *
 */
lglTemplateMarkup *
lgl_template_markup_circle_new (gdouble x0,
                                gdouble y0,
                                gdouble r)
{
	lglTemplateMarkup *markup;

	markup = g_new0 (lglTemplateMarkup, 1);

	markup->type        = LGL_TEMPLATE_MARKUP_CIRCLE;
	markup->circle.x0   = x0;
	markup->circle.y0   = y0;
	markup->circle.r    = r;

	return markup;
}


/**
 * lgl_template_markup_rect_new:
 *   @x1: x coordinate of top-left corner of rectangle.
 *   @y1: y coordinate of top-left corner of rectangle.
 *   @w:  width of rectangle.
 *   @h:  height of rectangle.
 *   @r:  radius of rounded corner.
 *
 * This function creates a new rectangle markup structure.
 *
 * Returns: a newly allocated #lglTemplateMarkup structure.
 *
 */
lglTemplateMarkup *
lgl_template_markup_rect_new (gdouble x1,
                              gdouble y1,
                              gdouble w,
                              gdouble h,
                              gdouble r)
{
	lglTemplateMarkup *markup;

	markup = g_new0 (lglTemplateMarkup, 1);

	markup->type        = LGL_TEMPLATE_MARKUP_RECT;
	markup->rect.x1     = x1;
	markup->rect.y1     = y1;
	markup->rect.w      = w;
	markup->rect.h      = h;
	markup->rect.r      = r;

	return markup;
}


/**
 * lgl_template_markup_ellipse_new:
 *   @x1: x coordinate of top-left corner of ellipse.
 *   @y1: y coordinate of top-left corner of ellipse.
 *   @w:  width of ellipse.
 *   @h:  height of ellipse.
 *
 * This function creates a new ellipse markup structure.
 *
 * Returns: a newly allocated #lglTemplateMarkup structure.
 *
 */
lglTemplateMarkup *
lgl_template_markup_ellipse_new (gdouble x1,
                                 gdouble y1,
                                 gdouble w,
                                 gdouble h)
{
	lglTemplateMarkup *markup;

	markup = g_new0 (lglTemplateMarkup, 1);

	markup->type        = LGL_TEMPLATE_MARKUP_ELLIPSE;
	markup->ellipse.x1     = x1;
	markup->ellipse.y1     = y1;
	markup->ellipse.w      = w;
	markup->ellipse.h      = h;

	return markup;
}


/**
 * lgl_template_dup:
 *   @orig_template: Template to duplicate.
 *
 * This function duplicates a template structure.
 *
 * Returns:  a newly allocated #lglTemplate structure.
 *
 */
lglTemplate *
lgl_template_dup (const lglTemplate *orig_template)
{
	lglTemplate         *template;
	lglTemplateAlias    *alias;
	GList               *p;
	lglTemplateFrame    *frame;

	g_return_val_if_fail (orig_template, NULL);

	template = lgl_template_new (orig_template->brand,
                                     orig_template->part,
                                     orig_template->description,
                                     orig_template->paper_id,
                                     orig_template->page_width,
                                     orig_template->page_height);

        template->equiv_part  = g_strdup (orig_template->equiv_part);
        template->product_url = g_strdup (orig_template->product_url);

	for ( p=orig_template->aliases; p != NULL; p=p->next )
        {
                alias = (lglTemplateAlias *)p->data;

		if ( !(UTF8_EQUAL (template->brand, alias->brand) &&
                       UTF8_EQUAL (template->part, alias->part)) )
                {
			lgl_template_add_alias (template, lgl_template_alias_dup (alias));
		}

	}

	for ( p=orig_template->category_ids; p != NULL; p=p->next )
        {
                lgl_template_add_category (template, p->data);
	}

	for ( p=orig_template->frames; p != NULL; p=p->next )
        {
		frame = (lglTemplateFrame *)p->data;

		lgl_template_add_frame (template, lgl_template_frame_dup (frame));
	}

	return template;
}


/**
 * lgl_template_free:
 *   @template: Template to free.
 *
 * This function frees all memory associated with given template structure.
 *
 */
void
lgl_template_free (lglTemplate *template)
{
	GList            *p;
	lglTemplateFrame *frame;

	if ( template != NULL ) {

		g_free (template->brand);
		template->brand = NULL;

		g_free (template->part);
		template->part = NULL;

		g_free (template->description);
		template->description = NULL;

		g_free (template->paper_id);
		template->paper_id = NULL;

		for ( p=template->aliases; p != NULL; p=p->next ) {

			lgl_template_alias_free (p->data);
			p->data = NULL;

		}
		g_list_free (template->aliases);
		template->aliases = NULL;

		for ( p=template->category_ids; p != NULL; p=p->next ) {

			g_free (p->data);
			p->data = NULL;

		}
		g_list_free (template->category_ids);
		template->category_ids = NULL;

		for ( p=template->frames; p != NULL; p=p->next ) {

			frame = (lglTemplateFrame *)p->data;

			lgl_template_frame_free (frame);
			p->data = NULL;
		}
		g_list_free (template->frames);
		template->frames = NULL;

		g_free (template);

	}

}


/**
 * lgl_template_alias_dup:
 *   @orig_alias: Alias to duplicate.
 *
 * This function duplicates a template alias structure.
 *
 * Returns:  a newly allocated #lglTemplateAlias structure.
 *
 */
lglTemplateAlias *
lgl_template_alias_dup (const lglTemplateAlias *orig_alias)
{
	g_return_val_if_fail (orig_alias, NULL);

	return lgl_template_alias_new (orig_alias->brand, orig_alias->part);
}


/**
 * lgl_template_alias_free:
 *   @alias: Alias to free.
 *
 * This function frees all memory associated with given template alias structure.
 *
 */
void
lgl_template_alias_free (lglTemplateAlias *alias)
{

	if ( alias != NULL )
        {
		g_free (alias->brand);
		alias->brand = NULL;

		g_free (alias->part);
		alias->part = NULL;

		g_free (alias);
	}
}


/**
 * lgl_template_frame_dup:
 *   @orig_frame: Frame to duplicate.
 *
 * This function duplicates a template frame structure.
 *
 * Returns:  a newly allocated #lglTemplateFrame structure.
 *
 */
lglTemplateFrame *
lgl_template_frame_dup (const lglTemplateFrame *orig_frame)
{
	lglTemplateFrame    *frame;
	GList               *p;
	lglTemplateLayout   *layout;
	lglTemplateMarkup   *markup;

	g_return_val_if_fail (orig_frame, NULL);

	switch (orig_frame->shape) {

	case LGL_TEMPLATE_FRAME_SHAPE_RECT:
		frame =
			lgl_template_frame_rect_new (orig_frame->all.id,
                                                     orig_frame->rect.w,
                                                     orig_frame->rect.h,
                                                     orig_frame->rect.r,
                                                     orig_frame->rect.x_waste,
                                                     orig_frame->rect.y_waste);
		break;

	case LGL_TEMPLATE_FRAME_SHAPE_ELLIPSE:
		frame =
			lgl_template_frame_ellipse_new (orig_frame->all.id,
                                                        orig_frame->ellipse.w,
                                                        orig_frame->ellipse.h,
                                                        orig_frame->ellipse.waste);
		break;

	case LGL_TEMPLATE_FRAME_SHAPE_ROUND:
		frame =
			lgl_template_frame_round_new (orig_frame->all.id,
                                                      orig_frame->round.r,
                                                      orig_frame->round.waste);
		break;

	case LGL_TEMPLATE_FRAME_SHAPE_CD:
		frame =
			lgl_template_frame_cd_new (orig_frame->all.id,
                                                   orig_frame->cd.r1,
                                                   orig_frame->cd.r2,
                                                   orig_frame->cd.w,
                                                   orig_frame->cd.h,
                                                   orig_frame->cd.waste);
		break;

	default:
		return NULL;
		break;
	}

	for ( p=orig_frame->all.layouts; p != NULL; p=p->next ) {

		layout = (lglTemplateLayout *)p->data;

		lgl_template_frame_add_layout (frame, lgl_template_layout_dup (layout));
	}

	for ( p=orig_frame->all.markups; p != NULL; p=p->next ) {

		markup = (lglTemplateMarkup *)p->data;

		lgl_template_frame_add_markup (frame, lgl_template_markup_dup (markup));
	}

	return frame;
}


/**
 * lgl_template_frame_free:
 *   @frame: Frame to free.
 *
 * This function frees all memory associated with given template frame structure.
 *
 */
void
lgl_template_frame_free (lglTemplateFrame *frame)
{
	GList                *p;
	lglTemplateLayout    *layout;
	lglTemplateMarkup    *markup;

	if ( frame != NULL ) {

		g_free (frame->all.id);
		frame->all.id = NULL;

		for ( p=frame->all.layouts; p != NULL; p=p->next ) {

			layout = (lglTemplateLayout *)p->data;

			lgl_template_layout_free (layout);
			p->data = NULL;
		}
		g_list_free (frame->all.layouts);
		frame->all.layouts = NULL;

		for ( p=frame->all.markups; p != NULL; p=p->next ) {

			markup = (lglTemplateMarkup *)p->data;

			lgl_template_markup_free (markup);
			p->data = NULL;
		}
		g_list_free (frame->all.markups);
		frame->all.markups = NULL;

		g_free (frame);

	}

}


/**
 * lgl_template_layout_dup:
 *   @orig_layout: Layout to duplicate.
 *
 * This function duplicates a template layout structure.
 *
 * Returns:  a newly allocated #lglTemplateLayout structure.
 *
 */
lglTemplateLayout *
lgl_template_layout_dup (const lglTemplateLayout *orig_layout)
{
	lglTemplateLayout *layout;

	g_return_val_if_fail (orig_layout, NULL);

	layout = g_new0 (lglTemplateLayout, 1);

	/* copy contents */
	*layout = *orig_layout;

	return layout;
}


/**
 * lgl_template_layout_free:
 *   @layout: Layout to free.
 *
 * This function frees all memory associated with given template layout structure.
 *
 */
void
lgl_template_layout_free (lglTemplateLayout *layout)
{
	g_free (layout);
}


/**
 * lgl_template_markup_dup:
 *   @orig_markup: Markup to duplicate.
 *
 * This function duplicates a template markup structure.
 *
 * Returns:  a newly allocated #lglTemplateMarkup structure.
 *
 */
lglTemplateMarkup *
lgl_template_markup_dup (const lglTemplateMarkup *orig_markup)
{
	lglTemplateMarkup *markup;

	g_return_val_if_fail (orig_markup, NULL);

	markup = g_new0 (lglTemplateMarkup, 1);

	*markup = *orig_markup;

	return markup;
}


/**
 * lgl_template_markup_free:
 *   @markup: Markup to free.
 *
 * This function frees all memory associated with given template markup structure.
 *
 */
void
lgl_template_markup_free (lglTemplateMarkup *markup)
{
	g_free (markup);
}


static gint
compare_origins (gconstpointer a,
		 gconstpointer b,
		 gpointer      user_data)
{
	const lglTemplateOrigin *a_origin = a, *b_origin = b;

	if ( a_origin->y < b_origin->y ) {
		return -1;
	} else if ( a_origin->y > b_origin->y ) {
		return +1;
	} else {
		if ( a_origin->x < b_origin->x ) {
			return -1;
		} else if ( a_origin->x > b_origin->x ) {
			return +1;
		} else {
			return 0; /* hopefully 2 labels won't have the same origin */
		}
	}
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
