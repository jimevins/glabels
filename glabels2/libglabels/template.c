/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  template.c:  template module
 *
 *  Copyright (C) 2001-2004  Jim Evins <evins@snaught.com>.
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

#include "libglabels-private.h"

#include <string.h>

#include "xml.h"
#include "template.h"
#include "xml-template.h"
#include "paper.h"

#define FULL_PAGE "Full-page"

/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GList *templates = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static glTemplate *template_full_page           (const gchar            *page_size);

static GList      *read_templates               (void);

static GList      *read_template_files_from_dir (GList                  *templates,
						 const gchar            *dirname);
static gint        compare_origins              (gconstpointer           a,
						 gconstpointer           b,
						 gpointer                user_data);

/*****************************************************************************/
/* Initialize module.                                                        */
/*****************************************************************************/
void
gl_template_init (void)
{
	GList *page_sizes, *p;

	if (templates) {
		return; /* Already initialized */
	}

	templates = read_templates ();

	page_sizes = gl_paper_get_id_list ();
	for ( p=page_sizes; p != NULL; p=p->next ) {
		if ( !gl_paper_is_id_other (p->data) ) {
			templates = g_list_append (templates,
						   template_full_page (p->data));
		}
	}
	gl_paper_free_id_list (page_sizes);
}

/*****************************************************************************/
/* Register template: if not in current list, add it.                        */
/*****************************************************************************/
void
gl_template_register (const glTemplate  *template)
{
	GList      *p_tmplt, *pa1;
	glTemplate *template1;

	if (!templates) {
		gl_template_init ();
	}

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next) {
		template1 = (glTemplate *) p_tmplt->data;

		for (pa1=template1->aliases; pa1!=NULL; pa1=pa1->next) {
			
			if (g_strcasecmp (template->name, pa1->data) == 0) {

				/* FIXME: make sure templates are really identical */
				/*        if not, apply hash to name to make unique. */
				return;
			}
				
		}

	}

	if (gl_paper_is_id_known (template->page_size)) {

		gchar *dir, *filename, *abs_filename;

		templates = g_list_append (templates,
					   gl_template_dup (template));

		/* FIXME: make sure filename is unique */
		dir = GL_USER_DATA_DIR;
		mkdir (dir, 0775); /* Try to make sure directory exists. */
		filename = g_strconcat (template->name, ".template", NULL);
		abs_filename = g_build_filename (dir, filename, NULL);
		gl_xml_template_write_template_to_file (template, abs_filename);
		g_free (dir);
		g_free (filename);
		g_free (abs_filename);

	} else {
		g_warning ("Cannot register new template with unknown page size.");
	}

}

/*****************************************************************************/
/* Get a list of valid template names for given page size                    */
/*****************************************************************************/
GList *
gl_template_get_name_list (const gchar *page_size)
{
	GList      *p_tmplt, *p_alias;
	glTemplate *template;
	gchar      *str;
	GList      *names = NULL;

	if (!templates) {
		gl_template_init ();
	}

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next) {
		template = (glTemplate *) p_tmplt->data;
		if (g_strcasecmp (page_size, template->page_size) == 0) {
			for (p_alias = template->aliases; p_alias != NULL;
			     p_alias = p_alias->next) {
				str = g_strdup_printf("%s: %s",
						      (gchar *) p_alias->data,
						      template->description);
				names = g_list_insert_sorted (names, str,
							     (GCompareFunc)g_strcasecmp);
			}
		}
	}

	return names;
}

/*****************************************************************************/
/* Free a list of template names.                                            */
/*****************************************************************************/
void
gl_template_free_name_list (GList *names)
{
	GList *p_name;

	for (p_name = names; p_name != NULL; p_name = p_name->next) {
		g_free (p_name->data);
		p_name->data = NULL;
	}

	g_list_free (names);
}

/*****************************************************************************/
/* Return a template structure from a name.                                  */
/*****************************************************************************/
glTemplate *
gl_template_from_name (const gchar *name)
{
	GList       *p_tmplt, *p_alias;
	glTemplate  *template, *new_template;
	gchar      **split_name;

	if (!templates) {
		gl_template_init ();
	}

	if (name == NULL) {
		/* If no name, return first template as a default */
		return gl_template_dup ((glTemplate *) templates->data);
	}

	/* Strip off any descriptions */
	split_name = g_strsplit (name, ":", 2);

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next) {
		template = (glTemplate *) p_tmplt->data;
		for (p_alias = template->aliases; p_alias != NULL;
		     p_alias = p_alias->next) {
			if (g_strcasecmp (p_alias->data, split_name[0]) == 0) {

				new_template = gl_template_dup (template);

				/* Use the real name */
				g_free (new_template->name);
				new_template->name = g_strdup (split_name[0]);

				g_strfreev (split_name);

				return new_template;
			}
		}
	}

	g_strfreev (split_name);

	/* No matching template has been found so return the first template */
	return gl_template_dup ((glTemplate *) templates->data);
}

/*****************************************************************************/
/* Get name and format with description.                                     */
/*****************************************************************************/
gchar *
gl_template_get_name_with_desc (const glTemplate  *template)
{
	g_return_val_if_fail (template, NULL);

	return g_strdup_printf("%s: %s", template->name, template->description);
}

/*****************************************************************************/
/* Get first label type in template.                                         */
/*****************************************************************************/
const glTemplateLabelType *
gl_template_get_first_label_type (const glTemplate *template)
{
	g_return_val_if_fail (template, NULL);

	return (glTemplateLabelType *)template->label_types->data;
}

/****************************************************************************/
/* Get raw label size (width and height).                                   */
/****************************************************************************/
void
gl_template_get_label_size (const glTemplateLabelType *label_type,
			    gdouble                   *w,
			    gdouble                   *h)
{
	g_return_if_fail (label_type);

	switch (label_type->shape) {
	case GL_TEMPLATE_SHAPE_RECT:
		*w = label_type->size.rect.w;
		*h = label_type->size.rect.h;
		break;
	case GL_TEMPLATE_SHAPE_ROUND:
		*w = 2.0 * label_type->size.round.r;
		*h = 2.0 * label_type->size.round.r;
		break;
	case GL_TEMPLATE_SHAPE_CD:
		if (label_type->size.cd.w == 0.0) {
			*w = 2.0 * label_type->size.cd.r1;
		} else {
			*w = label_type->size.cd.w;
		}
		if (label_type->size.cd.h == 0.0) {
			*h = 2.0 * label_type->size.cd.r1;
		} else {
			*h = label_type->size.cd.h;
		}
		break;
	default:
		*w = 0.0;
		*h = 0.0;
		break;
	}
}

/****************************************************************************/
/* Get total number of labels per sheet of a label type.                    */
/****************************************************************************/
gint
gl_template_get_n_labels (const glTemplateLabelType *label_type)
{
	gint              n_labels = 0;
	GList            *p;
	glTemplateLayout *layout;

	g_return_val_if_fail (label_type, 0);

	for ( p=label_type->layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;

		n_labels += layout->nx * layout->ny;
	}

	return n_labels;
}

/****************************************************************************/
/* Get array of origins of individual labels.                               */
/****************************************************************************/
glTemplateOrigin *
gl_template_get_origins (const glTemplateLabelType *label_type)
{
	gint              i_label, n_labels, ix, iy;
	glTemplateOrigin *origins;
	GList            *p;
	glTemplateLayout *layout;

	g_return_val_if_fail (label_type, NULL);

	n_labels = gl_template_get_n_labels (label_type);
	origins = g_new0 (glTemplateOrigin, n_labels);

	i_label = 0;
	for ( p=label_type->layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;

		for (iy = 0; iy < layout->ny; iy++) {
			for (ix = 0; ix < layout->nx; ix++, i_label++) {
				origins[i_label].x = ix*layout->dx + layout->x0;
				origins[i_label].y = iy*layout->dy + layout->y0;
			}
		}
	}

	g_qsort_with_data (origins, n_labels, sizeof(glTemplateOrigin),
			   compare_origins, NULL);

	return origins;
}

/*****************************************************************************/
/* Create a new template with given properties.                              */
/*****************************************************************************/
glTemplate *
gl_template_new (const gchar         *name,
		 const gchar         *description,
		 const gchar         *page_size,
		 gdouble              page_width,
		 gdouble              page_height)
{
	glTemplate *template;

	template = g_new0 (glTemplate,1);

	template->name        = g_strdup (name);
	template->description = g_strdup (description);
	template->page_size   = g_strdup (page_size);
	template->page_width  = page_width;
	template->page_height = page_height;

	/* Always include primary name in alias list. */
	template->aliases = NULL;
	template->aliases = g_list_append (template->aliases, g_strdup (name));

	return template;
}

/*****************************************************************************/
/* Add label type structure to label type list of template.                  */
/*****************************************************************************/
void
gl_template_add_label_type (glTemplate          *template,
			    glTemplateLabelType *label_type)
{
	g_return_if_fail (template);
	g_return_if_fail (label_type);

	template->label_types = g_list_append (template->label_types,
					       label_type);
}
 
/*****************************************************************************/
/* Add alias to alias list of template.                                      */
/*****************************************************************************/
void
gl_template_add_alias (glTemplate          *template,
		       const gchar         *alias)
{
	g_return_if_fail (template);
	g_return_if_fail (alias);

	template->aliases = g_list_append (template->aliases,
					   g_strdup (alias));
}
 
/*****************************************************************************/
/* Create a new label type structure for a rectangular label.                */
/*****************************************************************************/
glTemplateLabelType *
gl_template_rect_label_type_new  (const gchar         *id,
				  gdouble              w,
				  gdouble              h,
				  gdouble              r,
				  gdouble              waste)
{
	glTemplateLabelType *label_type;

	label_type = g_new0 (glTemplateLabelType, 1);

	label_type->id    = g_strdup (id);
	label_type->waste = waste;
	label_type->shape = GL_TEMPLATE_SHAPE_RECT;

	label_type->size.rect.w = w;
	label_type->size.rect.h = h;
	label_type->size.rect.r = r;

	return label_type;
}
                                                                               
/*****************************************************************************/
/* Create a new label type structure for a round label.                      */
/*****************************************************************************/
glTemplateLabelType *
gl_template_round_label_type_new (const gchar         *id,
				  gdouble              r,
				  gdouble              waste)
{
	glTemplateLabelType *label_type;

	label_type = g_new0 (glTemplateLabelType, 1);

	label_type->id    = g_strdup (id);
	label_type->waste = waste;
	label_type->shape = GL_TEMPLATE_SHAPE_ROUND;

	label_type->size.round.r = r;

	return label_type;
}
                                                                               
/*****************************************************************************/
/* Create a new label type structure for a CD/DVD label.                     */
/*****************************************************************************/
glTemplateLabelType *
gl_template_cd_label_type_new (const gchar         *id,
			       gdouble              r1,
			       gdouble              r2,
			       gdouble              w,
			       gdouble              h,
			       gdouble              waste)
{
	glTemplateLabelType *label_type;

	label_type = g_new0 (glTemplateLabelType, 1);

	label_type->id    = g_strdup (id);
	label_type->waste = waste;
	label_type->shape = GL_TEMPLATE_SHAPE_CD;

	label_type->size.cd.r1 = r1;
	label_type->size.cd.r2 = r2;
	label_type->size.cd.w  = w;
	label_type->size.cd.h  = h;

	return label_type;
}

/*****************************************************************************/
/* Add a layout to a label type.                                             */
/*****************************************************************************/
void
gl_template_add_layout (glTemplateLabelType *label_type,
			glTemplateLayout    *layout)
{
	g_return_if_fail (label_type);
	g_return_if_fail (layout);

	label_type->layouts = g_list_append (label_type->layouts,
					     layout);
}
 
/*****************************************************************************/
/* Add a markup item to a label type.                                        */
/*****************************************************************************/
void
gl_template_add_markup (glTemplateLabelType *label_type,
			glTemplateMarkup    *markup)
{
	g_return_if_fail (label_type);
	g_return_if_fail (markup);

	label_type->markups = g_list_append (label_type->markups,
					     markup);
}
 
/*****************************************************************************/
/* Create new layout structure.                                              */
/*****************************************************************************/
glTemplateLayout *
gl_template_layout_new (gdouble nx,
			gdouble ny,
			gdouble x0,
			gdouble y0,
			gdouble dx,
			gdouble dy)
{
	glTemplateLayout *layout;

	layout = g_new0 (glTemplateLayout, 1);

	layout->nx = nx;
	layout->ny = ny;
	layout->x0 = x0;
	layout->y0 = y0;
	layout->dx = dx;
	layout->dy = dy;

	return layout;
}

/*****************************************************************************/
/* Create new margin markup structure.                                       */
/*****************************************************************************/
glTemplateMarkup *
gl_template_markup_margin_new (gdouble size)
{
	glTemplateMarkup *markup;

	markup = g_new0 (glTemplateMarkup, 1);

	markup->type             = GL_TEMPLATE_MARKUP_MARGIN;
	markup->data.margin.size = size;

	return markup;
}

/*****************************************************************************/
/* Create new markup line structure.                                         */
/*****************************************************************************/
glTemplateMarkup *
gl_template_markup_line_new (gdouble x1,
			     gdouble y1,
			     gdouble x2,
			     gdouble y2)
{
	glTemplateMarkup *markup;

	markup = g_new0 (glTemplateMarkup, 1);

	markup->type             = GL_TEMPLATE_MARKUP_LINE;
	markup->data.line.x1     = x1;
	markup->data.line.y1     = y1;
	markup->data.line.x2     = x2;
	markup->data.line.y2     = y2;

	return markup;
}

/*****************************************************************************/
/* Create new markup circle structure.                                       */
/*****************************************************************************/
glTemplateMarkup *
gl_template_markup_circle_new (gdouble x0,
			       gdouble y0,
			       gdouble r)
{
	glTemplateMarkup *markup;

	markup = g_new0 (glTemplateMarkup, 1);

	markup->type             = GL_TEMPLATE_MARKUP_CIRCLE;
	markup->data.circle.x0   = x0;
	markup->data.circle.y0   = y0;
	markup->data.circle.r    = r;

	return markup;
}


/*****************************************************************************/
/* Copy a template.                                                          */
/*****************************************************************************/
glTemplate *
gl_template_dup (const glTemplate *orig_template)
{
	glTemplate          *template;
	GList               *p;
	glTemplateLabelType *label_type;

	g_return_val_if_fail (orig_template, NULL);

	template = gl_template_new (orig_template->name,
				    orig_template->description,
				    orig_template->page_size,
				    orig_template->page_width,
				    orig_template->page_height);

	for ( p=orig_template->label_types; p != NULL; p=p->next ) {

		label_type = (glTemplateLabelType *)p->data;

		gl_template_add_label_type (template,
					    gl_template_label_type_dup (label_type));
	}

	for ( p=orig_template->aliases; p != NULL; p=p->next ) {

		if (g_strcasecmp (template->name, p->data) != 0) {
			gl_template_add_alias (template, p->data);
		}

	}

	return template;
}

/*****************************************************************************/
/* Free up a template.                                                       */
/*****************************************************************************/
void
gl_template_free (glTemplate *template)
{
	GList               *p;
	glTemplateLabelType *label_type;

	if ( template != NULL ) {

		g_free (template->name);
		template->name = NULL;

		g_free (template->description);
		template->description = NULL;

		g_free (template->page_size);
		template->page_size = NULL;

		for ( p=template->label_types; p != NULL; p=p->next ) {

			label_type = (glTemplateLabelType *)p->data;

			gl_template_label_type_free (label_type);
			p->data = NULL;
		}
		g_list_free (template->label_types);
		template->label_types = NULL;

		for ( p=template->aliases; p != NULL; p=p->next ) {

			g_free (p->data);
			p->data = NULL;

		}
		g_list_free (template->aliases);
		template->aliases = NULL;

		g_free (template);

	}

}

/*****************************************************************************/
/* Copy a template label type.                                               */
/*****************************************************************************/
glTemplateLabelType *
gl_template_label_type_dup (const glTemplateLabelType *orig_label_type)
{
	glTemplateLabelType *label_type;
	GList               *p;
	glTemplateLayout    *layout;
	glTemplateMarkup    *markup;

	g_return_val_if_fail (orig_label_type, NULL);

	switch (orig_label_type->shape) {

	case GL_TEMPLATE_SHAPE_RECT:
		label_type =
			gl_template_rect_label_type_new (orig_label_type->id,
							 orig_label_type->size.rect.w,
							 orig_label_type->size.rect.h,
							 orig_label_type->size.rect.r,
							 orig_label_type->waste);
		break;

	case GL_TEMPLATE_SHAPE_ROUND:
		label_type =
			gl_template_round_label_type_new (orig_label_type->id,
							  orig_label_type->size.round.r,
							  orig_label_type->waste);
		break;

	case GL_TEMPLATE_SHAPE_CD:
		label_type =
			gl_template_cd_label_type_new (orig_label_type->id,
						       orig_label_type->size.cd.r1,
						       orig_label_type->size.cd.r2,
						       orig_label_type->size.cd.w,
						       orig_label_type->size.cd.h,
						       orig_label_type->waste);
		break;

	default:
		return NULL;
		break;
	}

	for ( p=orig_label_type->layouts; p != NULL; p=p->next ) {

		layout = (glTemplateLayout *)p->data;

		gl_template_add_layout (label_type,
					gl_template_layout_dup (layout));
	}

	for ( p=orig_label_type->markups; p != NULL; p=p->next ) {

		markup = (glTemplateMarkup *)p->data;

		gl_template_add_markup (label_type,
					gl_template_markup_dup (markup));
	}

	return label_type;
}

/*****************************************************************************/
/* Free up a template label type.                                            */
/*****************************************************************************/
void
gl_template_label_type_free (glTemplateLabelType *label_type)
{
	GList               *p;
	glTemplateLayout    *layout;
	glTemplateMarkup    *markup;

	if ( label_type != NULL ) {

		g_free (label_type->id);
		label_type->id = NULL;

		for ( p=label_type->layouts; p != NULL; p=p->next ) {

			layout = (glTemplateLayout *)p->data;

			gl_template_layout_free (layout);
			p->data = NULL;
		}
		g_list_free (label_type->layouts);
		label_type->layouts = NULL;

		for ( p=label_type->markups; p != NULL; p=p->next ) {

			markup = (glTemplateMarkup *)p->data;

			gl_template_markup_free (markup);
			p->data = NULL;
		}
		g_list_free (label_type->markups);
		label_type->markups = NULL;

		g_free (label_type);

	}

}

/*****************************************************************************/
/* Duplicate layout structure.                                               */
/*****************************************************************************/
glTemplateLayout *
gl_template_layout_dup (const glTemplateLayout *orig_layout)
{
	glTemplateLayout *layout;

	g_return_val_if_fail (orig_layout, NULL);

	layout = g_new0 (glTemplateLayout, 1);

	/* copy contents */
	*layout = *orig_layout;

	return layout;
}

/*****************************************************************************/
/* Free layout structure.                                                    */
/*****************************************************************************/
void
gl_template_layout_free (glTemplateLayout *layout)
{
	g_free (layout);
}

/*****************************************************************************/
/* Duplicate markup structure.                                               */
/*****************************************************************************/
glTemplateMarkup *
gl_template_markup_dup (const glTemplateMarkup *orig_markup)
{
	glTemplateMarkup *markup;

	g_return_val_if_fail (orig_markup, NULL);

	markup = g_new0 (glTemplateMarkup, 1);

	*markup = *orig_markup;

	return markup;
}

/*****************************************************************************/
/* Free markup structure.                                                    */
/*****************************************************************************/
void
gl_template_markup_free (glTemplateMarkup *markup)
{
	g_free (markup);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Make a template for a full page of the given page size.        */
/*--------------------------------------------------------------------------*/
static glTemplate *
template_full_page (const gchar *page_size)
{
	glPaper               *paper = NULL;
	glTemplate            *template = NULL;
	glTemplateLabelType   *label_type = NULL;
	gchar                 *name;

	g_return_val_if_fail (page_size, NULL);

	paper = gl_paper_from_id (page_size);
	if ( paper == NULL ) {
		return NULL;
	}

	name         = g_strdup_printf (_("Generic %s full page"), page_size);

	template = gl_template_new (name,
				    FULL_PAGE,
				    page_size,
				    paper->width,
				    paper->height);


	label_type = gl_template_rect_label_type_new ("0",
						      paper->width,
						      paper->height,
						      0.0,
						      0.0);
	gl_template_add_label_type (template, label_type);

	gl_template_add_layout (label_type,
				gl_template_layout_new (1, 1, 0., 0., 0., 0.));

	gl_template_add_markup (label_type,
				gl_template_markup_margin_new (9.0));

	g_free (name);
	name = NULL;
	gl_paper_free (paper);
	paper = NULL;

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read templates from various  files.                            */
/*--------------------------------------------------------------------------*/
static GList *
read_templates (void)
{
	gchar *data_dir;
	GList *templates = NULL;

	data_dir = GL_SYSTEM_DATA_DIR;
	templates = read_template_files_from_dir (templates, data_dir);
	g_free (data_dir);

	data_dir = GL_USER_DATA_DIR;
	templates = read_template_files_from_dir (templates, data_dir);
	g_free (data_dir);

	if (templates == NULL) {
		g_warning (_("No template files found!"));
	}

	return templates;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read all template files from given directory.  Append to list. */
/*--------------------------------------------------------------------------*/
static GList *
read_template_files_from_dir (GList       *templates,
			      const gchar *dirname)
{
	GDir        *dp;
	const gchar *filename, *extension, *extension2;
	gchar       *full_filename = NULL;
	GError      *gerror = NULL;
	GList       *new_templates = NULL;

	if (dirname == NULL)
		return templates;

	if (!g_file_test (dirname, G_FILE_TEST_EXISTS)) {
		return templates;
	}

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL) {
	        g_warning ("cannot open data directory: %s", gerror->message );
		return templates;
	}

	while ((filename = g_dir_read_name (dp)) != NULL) {

		extension = strrchr (filename, '.');
		extension2 = strrchr (filename, '-');

		if (extension != NULL) {

			if ( (g_strcasecmp (extension, ".template") == 0)
			     || (g_strcasecmp (extension2, "-templates.xml") == 0) ) {

				full_filename =
				    g_build_filename (dirname, filename, NULL);
				new_templates =
				    gl_xml_template_read_templates_from_file (full_filename);
				g_free (full_filename);

				templates = g_list_concat (templates, new_templates);
				new_templates = NULL;
			}

		}

	}

	g_dir_close (dp);

	return templates;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Sort origins comparison function, first by y then by x.        */
/*--------------------------------------------------------------------------*/
static gint
compare_origins (gconstpointer a,
		 gconstpointer b,
		 gpointer      user_data)
{
	const glTemplateOrigin *a_origin = a, *b_origin = b;

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

/*****************************************************************************/
/* Print all known templates (for debugging purposes).                       */
/*****************************************************************************/
void
gl_template_print_known_templates (void)
{
	GList      *p;
	glTemplate *template;

	g_print ("%s():\n", __FUNCTION__);
	for (p=templates; p!=NULL; p=p->next) {
		template = (glTemplate *)p->data;

		g_print("TEMPLATE name=\"%s\", description=\"%s\"\n",
			template->name, template->description);

	}
	g_print ("\n");

}

/*****************************************************************************/
/* Print all aliases of a template (for debugging purposes).                 */
/*****************************************************************************/
void
gl_template_print_aliases (const glTemplate *template)
{
	GList *p;

	g_print ("%s():\n", __FUNCTION__);
	for (p=template->aliases; p!=NULL; p=p->next) {
		
		g_print("Alias = \"%s\"\n", p->data);

	}
	g_print ("\n");

}

