/*
 *  db.h
 *  Copyright (C) 2006-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __LGL_DB_H__
#define __LGL_DB_H__

#include <glib.h>

#include "paper.h"
#include "category.h"
#include "vendor.h"
#include "template.h"

G_BEGIN_DECLS

typedef enum
{
        LGL_DB_REG_OK                =  0,
        LGL_DB_REG_BAD_PAPER_ID      = -1,
        LGL_DB_REG_BRAND_PART_EXISTS = -2,
        LGL_DB_REG_FILE_WRITE_ERROR  = -3
} lglDbRegStatus;

typedef enum
{
        LGL_DB_DELETE_OK                =  0,
        LGL_DB_DELETE_DOES_NOT_EXIST    = -1,
        LGL_DB_DELETE_NOT_USER_DEFINED  = -2,
        LGL_DB_DELETE_FILE_ERROR        = -3
} lglDbDeleteStatus;


typedef void  (*lglDbNotifyFunc) (gpointer user_data);




/*
 * Module initialization
 */
void           lgl_db_init                           (void);



/*
 * Notification
 */
gulong         lgl_db_notify_add                     (lglDbNotifyFunc      func,
                                                      gpointer             user_data);

void           lgl_db_notify_remove                  (gulong               id);



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
 * Vendor
 */
GList         *lgl_db_get_vendor_name_list           (void);

void           lgl_db_free_vendor_name_list          (GList               *names);

lglVendor     *lgl_db_lookup_vendor_from_name        (const gchar         *name);

gboolean       lgl_db_is_vendor_name_known           (const gchar         *name);


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

lglDbDeleteStatus lgl_db_delete_template_by_name        (const gchar         *name);

lglDbDeleteStatus lgl_db_delete_template_by_brand_part  (const gchar         *brand,
                                                         const gchar         *part);

gboolean       lgl_db_does_template_exist            (const gchar         *brand,
                                                      const gchar         *part);

gboolean       lgl_db_does_template_name_exist       (const gchar         *name);

GList         *lgl_db_get_template_name_list_all     (const gchar         *brand,
                                                      const gchar         *paper_id,
                                                      const gchar         *category_id);

GList         *lgl_db_get_similar_template_name_list (const gchar         *name);

void           lgl_db_free_template_name_list        (GList               *names);

lglTemplate   *lgl_db_lookup_template_from_name      (const gchar         *name);

lglTemplate   *lgl_db_lookup_template_from_brand_part(const gchar         *brand,
                                                      const gchar         *part);


/*
 * Debugging functions
 */
void           lgl_db_print_known_papers             (void);

void           lgl_db_print_known_categories         (void);

void           lgl_db_print_known_templates          (void);

void           lgl_db_print_aliases                  (const lglTemplate    *template);

void           lgl_db_print_known_vendors            (void);



G_END_DECLS

#endif /* __LGL_DB_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
