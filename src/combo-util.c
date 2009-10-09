/*
 *  combo-util.c
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

#include <config.h>

#include "combo-util.h"

#include <string.h>

#include <libglabels/libglabels.h>


/****************************************************************************/
/* Convienience function to set strings in a text combo_box from a GList    */
/****************************************************************************/
void
gl_combo_util_set_strings (GtkComboBox       *combo,
                           GList             *list)
{
	GtkTreeModel *model;
	GList        *p;

	g_return_if_fail (list);

	model = gtk_combo_box_get_model(combo);
	gtk_list_store_clear (GTK_LIST_STORE (model));

	for (p=list; p!=NULL; p=p->next) {
		if (p->data) {
			gtk_combo_box_append_text (combo, p->data);
		}
	}
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  gl_combo_util_set_active_text support.                          */
/*---------------------------------------------------------------------------*/

typedef struct {
  const gchar *text;
  GtkTreeIter  iter;
  gboolean     found;
} TextSearchData;

static gboolean
search_text_func (GtkTreeModel *model,
		  GtkTreePath  *path,
		  GtkTreeIter  *iter,
		  gpointer      data)
{
  TextSearchData *search_data = (TextSearchData *)data;
  gchar          *text = NULL;

  gtk_tree_model_get (model, iter, 0, &text, -1);

  if (strcmp (text,search_data->text) == 0) {
    search_data->found = TRUE;
    search_data->iter  = *iter;
  }

  g_free (text);
  
  return FALSE;
}


/****************************************************************************/
/* Convienience function to set active text in a text combo_box from text   */
/****************************************************************************/
void
gl_combo_util_set_active_text (GtkComboBox       *combo,
                               const gchar       *text)
{
	GtkTreeModel   *model = gtk_combo_box_get_model(combo);

	g_return_if_fail (GTK_IS_LIST_STORE (model));

	if (!text) {

		gtk_combo_box_set_active (combo, -1);

	} else {
		TextSearchData  search_data;

		search_data.text        = text;
		search_data.found       = FALSE;

		gtk_tree_model_foreach (model, search_text_func, &search_data);
		if (search_data.found) {
			gtk_combo_box_set_active_iter (combo,
						       &search_data.iter);
		} else {
			gtk_combo_box_set_active (combo, -1);
		}    

	}

}


/****************************************************************************/
/* Convienience function to add a simple text model to an existing          */
/* combo_box.  This is needed since combo_boxes created with glade do not   */
/* use the gtk_combo_box_new_text() constructor.                            */
/****************************************************************************/
void
gl_combo_util_add_text_model (GtkComboBox       *combo)
{
	GtkCellRenderer *cell;
	GtkListStore *store;

	store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store));
	g_object_unref (store);

	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
					"text", 0,
					NULL);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
