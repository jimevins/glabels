/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  db.h:  template db module header file
 *
 *  Copyright (C) 2006-2007  Jim Evins <evins@snaught.com>.
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

#ifndef __DB_H__
#define __DB_H__

#include <glib/gtypes.h>
#include <glib/glist.h>

#include "paper.h"
#include "category.h"
#include "template.h"

G_BEGIN_DECLS

typedef enum
{
        LGL_DB_REG_OK                =  0,
        LGL_DB_REG_BAD_PAPER_ID      = -1,
        LGL_DB_REG_BRAND_PART_EXISTS = -2,
        LGL_DB_REG_FILE_WRITE_ERROR  = -3
} lglDbRegStatus;

/*
 * Module initialization
 */
void           lgl_db_init                           (void);


/*
 * Paper
 */
GList         *lgl_db_get_paper_id_list              (void);

void           lgl_db_free_paper_id_list             (GList               *ids);

GList         *lgl_db_get_paper_name_list            (void);

void           lgl_db_free_paper_name_list           (GList               *names);

lglPaper      *lgl_db_lookup_paper_from_name         (const gchar         *name);

lglPaper      *lgl_db_lookup_paper_from_id           (const gchar         *id);

gchar         *lgl_db_lookup_paper_id_from_name      (const gchar         *name);

gchar         *lgl_db_lookup_paper_name_from_id      (const gchar         *id);

gboolean       lgl_db_is_paper_id_known              (const gchar         *id);

gboolean       lgl_db_is_paper_id_other              (const gchar         *id);



/*
 * Template categories
 */
GList         *lgl_db_get_category_id_list           (void);

void           lgl_db_free_category_id_list          (GList              *ids);

GList         *lgl_db_get_category_name_list         (void);

void           lgl_db_free_category_name_list        (GList               *names);

lglCategory   *lgl_db_lookup_category_from_name      (const gchar         *name);

lglCategory   *lgl_db_lookup_category_from_id        (const gchar         *id);

gchar         *lgl_db_lookup_category_id_from_name   (const gchar         *name);

gchar         *lgl_db_lookup_category_name_from_id   (const gchar         *id);

gboolean       lgl_db_is_category_id_known           (const gchar         *id);


/*
 * Template brands
 */
GList         *lgl_db_get_brand_list                 (const gchar         *paper_id,
                                                      const gchar         *category_id);

void           lgl_db_free_brand_list                (GList               *brands);


/*
 * Templates
 */
lglDbRegStatus lgl_db_register_template              (const lglTemplate   *template);

gboolean       lgl_db_does_template_exist            (const gchar         *brand,
                                                      const gchar         *part);

gboolean       lgl_db_does_template_name_exist       (const gchar         *name);

GList         *lgl_db_get_template_name_list_unique  (const gchar         *brand,
                                                      const gchar         *paper_id,
                                                      const gchar         *category_id);

GList         *lgl_db_get_template_name_list_all     (const gchar         *brand,
                                                      const gchar         *paper_id,
                                                      const gchar         *category_id);

void           lgl_db_free_template_name_list        (GList               *names);

lglTemplate   *lgl_db_lookup_template_from_name      (const gchar         *name);


/*
 * Debugging functions
 */
void           lgl_db_print_known_papers             (void);

void           lgl_db_print_known_categories         (void);

void           lgl_db_print_known_templates          (void);

void           lgl_db_print_aliases                  (const lglTemplate    *template);



G_END_DECLS

#endif
