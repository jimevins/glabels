/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  template.c:  template module
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

#include <config.h>

#include <string.h>

#include "prefs.h"
#include "util.h"
#include "xml.h"
#include "template.h"
#include "xml-template.h"
#include "paper.h"

#include "debug.h"

#define GL_DATA_DIR gnome_program_locate_file (NULL,\
					 GNOME_FILE_DOMAIN_APP_DATADIR,\
					 "glabels",\
					 FALSE, NULL)

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

static gchar      *get_home_data_dir            (void);
static GList      *read_template_files_from_dir (GList                  *templates,
						 const gchar            *dirname);
static gint        compare_origins              (gconstpointer           a,
						 gconstpointer           b,
						 gpointer                user_data);

static glTemplateLayout *layout_dup             (glTemplateLayout       *orig_layout);
static void              layout_free            (glTemplateLayout      **layout);

static glTemplateMarkup *markup_dup             (glTemplateMarkup       *orig_markup);
static void              markup_free            (glTemplateMarkup      **markup);

/*****************************************************************************/
/* Initialize module.                                                        */
/*****************************************************************************/
void
gl_template_init (void)
{
	GList *page_sizes, *p;

	gl_debug (DEBUG_TEMPLATE, "START");

	templates = read_templates ();

	page_sizes = gl_paper_get_id_list ();
	for ( p=page_sizes; p != NULL; p=p->next ) {
		if ( g_strcasecmp(p->data, "Other") != 0 ) {
			templates = g_list_append (templates,
						   template_full_page (p->data));
		}
	}
	gl_paper_free_id_list (&page_sizes);

	gl_debug (DEBUG_TEMPLATE, "END");
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

	gl_debug (DEBUG_TEMPLATE, "START");

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next) {
		template = (glTemplate *) p_tmplt->data;
		if (g_strcasecmp (page_size, template->page_size) == 0) {
			for (p_alias = template->alias; p_alias != NULL;
			     p_alias = p_alias->next) {
				str = g_strdup_printf("%s: %s",
						      (gchar *) p_alias->data,
						      template->description);
				names = g_list_insert_sorted (names, str,
							     (GCompareFunc)g_strcasecmp);
			}
		}
	}

	gl_debug (DEBUG_TEMPLATE, "templates = %p", templates);
	gl_debug (DEBUG_TEMPLATE, "names = %p", names);

	gl_debug (DEBUG_TEMPLATE, "END");
	return names;
}

/*****************************************************************************/
/* Free a list of template names.                                            */
/*****************************************************************************/
void
gl_template_free_name_list (GList **names)
{
	GList *p_name;

	gl_debug (DEBUG_TEMPLATE, "START");

	for (p_name = *names; p_name != NULL; p_name = p_name->next) {
		g_free (p_name->data);
		p_name->data = NULL;
	}

	g_list_free (*names);
	*names = NULL;

	gl_debug (DEBUG_TEMPLATE, "END");
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

	gl_debug (DEBUG_TEMPLATE, "START");

	if (name == NULL) {
		/* If no name, return first template as a default */
		return gl_template_dup ((glTemplate *) templates->data);
	}

	split_name = g_strsplit (name, ":", 2);

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next) {
		template = (glTemplate *) p_tmplt->data;
		for (p_alias = template->alias; p_alias != NULL;
		     p_alias = p_alias->next) {
			if (g_strcasecmp (p_alias->data, split_name[0]) == 0) {

				new_template = gl_template_dup (template);

				/* Use the real name */
				g_free (new_template->name);
				new_template->name = g_strdup (split_name[0]);

				g_strfreev (split_name);
				gl_debug (DEBUG_TEMPLATE, "END");

				return new_template;
			}
		}
	}

	g_strfreev (split_name);

	gl_debug (DEBUG_TEMPLATE, "END");
	return NULL;
}

/*****************************************************************************/
/* Get name and format with description.                                     */
/*****************************************************************************/
gchar *
gl_template_get_name_with_desc (const glTemplate  *template)
{
	return g_strdup_printf("%s: %s", template->name, template->description);
}


/*****************************************************************************/
/* Copy a template.                                                          */
/*****************************************************************************/
glTemplate *gl_template_dup (const glTemplate *orig_template)
{
	glTemplate       *template;
	GList            *p;
	glTemplateLayout *layout;
	glTemplateMarkup *markup;

	gl_debug (DEBUG_TEMPLATE, "START");

	template = g_new0 (glTemplate,1);

	template->name = g_strdup (orig_template->name);

	template->alias = NULL;
	for ( p=orig_template->alias; p != NULL; p=p->next ) {
		template->alias = g_list_append (template->alias,
						g_strdup (p->data));
	}
	template->description = g_strdup (orig_template->description);
	template->page_size   = g_strdup (orig_template->page_size);
	template->page_width  = orig_template->page_width;
	template->page_height = orig_template->page_height;

	template->label       = orig_template->label;

	template->label.any.layouts = NULL;
	for ( p=orig_template->label.any.layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;
		template->label.any.layouts =
			g_list_append (template->label.any.layouts,
				       layout_dup (layout));
	}

	template->label.any.markups = NULL;
	for ( p=orig_template->label.any.markups; p != NULL; p=p->next ) {
		markup = (glTemplateMarkup *)p->data;
		template->label.any.markups =
			g_list_append (template->label.any.markups,
				       markup_dup (markup));
	}

	gl_debug (DEBUG_TEMPLATE, "END");
	return template;
}

/*****************************************************************************/
/* Free up a template.                                                       */
/*****************************************************************************/
void gl_template_free (glTemplate **template)
{
	GList *p;

	gl_debug (DEBUG_TEMPLATE, "START");

	if ( *template != NULL ) {

		g_free ((*template)->name);
		(*template)->name = NULL;

		for ( p=(*template)->alias; p != NULL; p=p->next ) {
			g_free (p->data);
			p->data = NULL;
		}
		g_list_free ((*template)->alias);
		(*template)->alias = NULL;

		g_free ((*template)->description);
		(*template)->description = NULL;

		g_free ((*template)->page_size);
		(*template)->page_size = NULL;

		for ( p=(*template)->label.any.layouts; p != NULL; p=p->next ) {
			layout_free ((glTemplateLayout **)&p->data);
		}
		g_list_free ((*template)->label.any.layouts);
		(*template)->label.any.layouts = NULL;

		for ( p=(*template)->label.any.markups; p != NULL; p=p->next ) {
			markup_free ((glTemplateMarkup **)&p->data);
		}
		g_list_free ((*template)->label.any.markups);
		(*template)->label.any.markups = NULL;

		g_free (*template);
		*template = NULL;

	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Make a template for a full page of the given page size.        */
/*--------------------------------------------------------------------------*/
static glTemplate *
template_full_page (const gchar *page_size)
{
	glPaper               *paper;
	glTemplate            *template;

	paper = gl_paper_from_id (page_size);
	if ( paper == NULL ) {
		return NULL;
	}

	template = g_new0 (glTemplate, 1);

	template->name         = g_strdup_printf (_("Generic %s full page"), page_size);

	template->alias        = g_list_append (template->alias, template->name);

	template->page_size    = g_strdup(page_size);
	template->page_width   = paper->width;
	template->page_height  = paper->height;
	template->description  = g_strdup(FULL_PAGE);

	template->label.style  = GL_TEMPLATE_STYLE_RECT;
	template->label.rect.w = paper->width;
	template->label.rect.h = paper->height;
	template->label.rect.r = 0.0;

	template->label.any.layouts =
		g_list_append (template->label.any.layouts,
			       gl_template_layout_new (1, 1, 0., 0., 0., 0.));

	template->label.any.markups =
		g_list_append (template->label.any.markups,
			       gl_template_markup_margin_new (5.0));

	gl_paper_free (&paper);

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read templates from various  files.                            */
/*--------------------------------------------------------------------------*/
static GList *
read_templates (void)
{
	gchar *home_data_dir = get_home_data_dir ();
	GList *templates = NULL;

	gl_debug (DEBUG_TEMPLATE, "START");

	templates = read_template_files_from_dir (templates, GL_DATA_DIR);
	templates = read_template_files_from_dir (templates, home_data_dir);

	g_free (home_data_dir);

	if (templates == NULL) {
		g_warning (_("No template files found!"));
	}

	gl_debug (DEBUG_TEMPLATE, "END");
	return templates;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  get '~/.glabels' directory path.                               */
/*--------------------------------------------------------------------------*/
static gchar *
get_home_data_dir (void)
{
	gchar *dir = gnome_util_prepend_user_home (".glabels");

	gl_debug (DEBUG_TEMPLATE, "START");

	/* Try to create ~/.glabels directory.  If it exists, no problem. */
	mkdir (dir, 0775);

	gl_debug (DEBUG_TEMPLATE, "END");
	return dir;
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

	gl_debug (DEBUG_TEMPLATE, "START");

	if (dirname == NULL)
		return templates;

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL) {
	        g_warning ("cannot open data directory: %s", gerror->message );
		gl_debug (DEBUG_TEMPLATE, "END");
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
				templates =
				    gl_xml_template_read_templates_from_file (templates,
									      full_filename);
				g_free (full_filename);

			}

		}

	}

	g_dir_close (dp);

	gl_debug (DEBUG_TEMPLATE, "END");
	return templates;
}

/****************************************************************************/
/* Get label size description.                                              */ 
/****************************************************************************/
gchar *
gl_template_get_label_size_desc (const glTemplate *template)
{
	glPrefsUnits  units;
	const gchar  *units_string;
	gdouble       units_per_point;
	gchar        *string = NULL;

	units           = gl_prefs_get_units ();
	units_string    = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();

	switch (template->label.style) {
	case GL_TEMPLATE_STYLE_RECT:
		if ( units == GL_UNITS_INCHES ) {
			gchar *xstr, *ystr;

			xstr = gl_util_fraction (template->label.rect.w * units_per_point);
			ystr = gl_util_fraction (template->label.rect.h * units_per_point);
			string = g_strdup_printf (_("%s x %s %s"),
						  xstr, ystr, units_string);
			g_free (xstr);
			g_free (ystr);
		} else {
			string = g_strdup_printf (_("%.5g x %.5g %s"),
						  template->label.rect.w * units_per_point,
						  template->label.rect.h * units_per_point,
						  units_string);
		}
		break;
	case GL_TEMPLATE_STYLE_ROUND:
		if ( units == GL_UNITS_INCHES ) {
			gchar *dstr;

			dstr = gl_util_fraction (2.0 * template->label.round.r * units_per_point);
			string = g_strdup_printf (_("%s %s diameter"),
						  dstr, units_string);
			g_free (dstr);
		} else {
			string = g_strdup_printf (_("%.5g %s diameter"),
						  2.0 * template->label.round.r * units_per_point,
						  units_string);
		}
		break;
	case GL_TEMPLATE_STYLE_CD:
		if ( units == GL_UNITS_INCHES ) {
			gchar *dstr;

			dstr = gl_util_fraction (2.0 * template->label.cd.r1 * units_per_point);
			string = g_strdup_printf (_("%s %s diameter"),
						  dstr, units_string);
			g_free (dstr);
		} else {
			string = g_strdup_printf (_("%.5g %s diameter"),
						  2.0 * template->label.cd.r1 * units_per_point,
						  units_string);
		}
		break;
	default:
		break;
	}

	return string;
}

/****************************************************************************/
/* Get raw label size (width and height).                                   */
/****************************************************************************/
void
gl_template_get_label_size (const glTemplate *template,
			    gdouble          *w,
			    gdouble          *h)
{
	switch (template->label.style) {
	case GL_TEMPLATE_STYLE_RECT:
		*w = template->label.rect.w;
		*h = template->label.rect.h;
		break;
	case GL_TEMPLATE_STYLE_ROUND:
		*w = 2.0 * template->label.round.r;
		*h = 2.0 * template->label.round.r;
		break;
	case GL_TEMPLATE_STYLE_CD:
		if (template->label.cd.w == 0.0) {
			*w = 2.0 * template->label.cd.r1;
		} else {
			*w = template->label.cd.w;
		}
		if (template->label.cd.h == 0.0) {
			*h = 2.0 * template->label.cd.r1;
		} else {
			*h = template->label.cd.h;
		}
		break;
	default:
		*w = 0.0;
		*h = 0.0;
		break;
	}
}

/****************************************************************************/
/* Get total number of labels per sheet.                                    */
/****************************************************************************/
gint
gl_template_get_n_labels (const glTemplate *template)
{
	gint              n_labels = 0;
	GList            *p;
	glTemplateLayout *layout;

	for ( p=template->label.any.layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;

		n_labels += layout->nx * layout->ny;
	}

	return n_labels;
}

/****************************************************************************/
/* Get array of origins of individual labels.                               */
/****************************************************************************/
glTemplateOrigin *
gl_template_get_origins (const glTemplate *template)
{
	gint              i_label, n_labels, ix, iy;
	glTemplateOrigin *origins;
	GList            *p;
	glTemplateLayout *layout;

	n_labels = gl_template_get_n_labels (template);
	origins = g_new0 (glTemplateOrigin, n_labels);

	i_label = 0;
	for ( p=template->label.any.layouts; p != NULL; p=p->next ) {
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

/****************************************************************************/
/* Get a description of the layout and number of labels.                    */
/****************************************************************************/
gchar *
gl_template_get_layout_desc (const glTemplate *template)
{
	gint              n_labels;
	glTemplateLayout *layout;
	gchar            *string;

	n_labels = gl_template_get_n_labels (template);

	if ( template->label.any.layouts->next == NULL ) {
		layout = (glTemplateLayout *)template->label.any.layouts->data;
		string = g_strdup_printf (_("%d x %d  (%d per sheet)"),
					  layout->nx, layout->ny,
					  n_labels);
	} else {
		string = g_strdup_printf (_("%d per sheet"),
					  n_labels);
	}

	return string;
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

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Duplicate layout structure.                                    */
/*--------------------------------------------------------------------------*/
static glTemplateLayout *
layout_dup (glTemplateLayout *orig_layout)
{
	glTemplateLayout *layout;

	layout = g_new0 (glTemplateLayout, 1);

	/* copy contents */
	*layout = *orig_layout;

	return layout;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Free layout structure.                                         */
/*--------------------------------------------------------------------------*/
static void
layout_free (glTemplateLayout **layout)
{
	g_free (*layout);
	*layout = NULL;
}

/*****************************************************************************/
/* Create new margin markup structure.                                       */
/*****************************************************************************/
glTemplateMarkup *
gl_template_markup_margin_new (gdouble size)
{
	glTemplateMarkup *markup;

	markup = g_new0 (glTemplateMarkup, 1);

	markup->type        = GL_TEMPLATE_MARKUP_MARGIN;
	markup->margin.size = size;

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

	markup->type        = GL_TEMPLATE_MARKUP_LINE;
	markup->line.x1     = x1;
	markup->line.y1     = y1;
	markup->line.x2     = x2;
	markup->line.y2     = y2;

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

	markup->type        = GL_TEMPLATE_MARKUP_CIRCLE;
	markup->circle.x0   = x0;
	markup->circle.y0   = y0;
	markup->circle.r    = r;

	return markup;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Duplicate markup structure.                                    */
/*--------------------------------------------------------------------------*/
static glTemplateMarkup *
markup_dup (glTemplateMarkup *orig_markup)
{
	glTemplateMarkup *markup;

	markup = g_new0 (glTemplateMarkup, 1);

	*markup = *orig_markup;

	return markup;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Free markup structure.                                         */
/*--------------------------------------------------------------------------*/
static void
markup_free (glTemplateMarkup **markup)
{
	g_free (*markup);
	*markup = NULL;
}
