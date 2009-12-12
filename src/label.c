/*
 *  label.c
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

#include "label.h"

#include <glib/gi18n.h>
#include <math.h>

#include "template-history.h"
#include "file-util.h"
#include "xml-label.h"
#include "prefs.h"
#include "label-text.h"
#include "label-image.h"
#include "marshal.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelPrivate {

	gchar       *filename;
	gint         compression;
	gboolean     modified_flag;
	gint         untitled_instance;

        lglTemplate *template;
        gboolean     rotate_flag;

        GList       *object_list;

	glMerge     *merge;

	GHashTable  *pixbuf_cache;

        /* Delay changed signals while operating on selections of multiple objects. */
        gboolean     selection_op_flag;
        gboolean     delayed_change_flag;

	/* Default object text properties */
	gchar             *default_font_family;
	gdouble            default_font_size;
	PangoWeight        default_font_weight;
	gboolean           default_font_italic_flag;
	guint              default_text_color;
	PangoAlignment     default_text_alignment;
	gdouble            default_text_line_spacing;

	/* Default object line properties */
	gdouble            default_line_width;
	guint              default_line_color;
	
	/* Default object fill properties */
	guint              default_fill_color;
};

typedef struct {
        gchar             *xml_buffer;
        gchar             *text;
        GdkPixbuf         *pixbuf;
} ClipboardData;

enum {
	SELECTION_CHANGED,
	CHANGED,
	NAME_CHANGED,
	MODIFIED_CHANGED,
	MERGE_CHANGED,
	SIZE_CHANGED,
	LAST_SIGNAL
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static guint signals[LAST_SIGNAL] = {0};

static guint untitled = 0;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_finalize      (GObject *object);

static void object_changed_cb      (glLabelObject *object,
                                    glLabel       *label);

static void do_modify              (glLabel       *label);

static void begin_selection_op     (glLabel       *label);
static void end_selection_op       (glLabel       *label);

static void clipboard_get_cb       (GtkClipboard     *clipboard,
                                    GtkSelectionData *selection_data,
                                    guint             info,
                                    ClipboardData    *data);

static void clipboard_clear_cb     (GtkClipboard     *clipboard,
                                    ClipboardData    *data);

static void receive_targets_cb     (GtkClipboard     *clipboard,
                                    GdkAtom          *targets,
                                    gint              n_targets,
                                    glLabel          *label);

static void paste_xml_received_cb  (GtkClipboard     *clipboard,
                                    GtkSelectionData *selection_data,
                                    glLabel          *label);

static void paste_text_received_cb (GtkClipboard     *clipboard,
                                    const gchar      *text,
                                    glLabel          *label);

static void paste_image_received_cb(GtkClipboard     *clipboard,
                                    GdkPixbuf        *pixbuf,
                                    glLabel          *label);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabel, gl_label, G_TYPE_OBJECT);


static void
gl_label_class_init (glLabelClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_LABEL, "START");

	gl_label_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_label_finalize;

	signals[SELECTION_CHANGED] =
		g_signal_new ("selection_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelClass, selection_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelClass, changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[NAME_CHANGED] =
		g_signal_new ("name_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelClass, name_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[MODIFIED_CHANGED] =
		g_signal_new ("modified_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelClass, modified_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[MERGE_CHANGED] =
		g_signal_new ("merge_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelClass, merge_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[SIZE_CHANGED] =
		g_signal_new ("size_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glLabelClass, size_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	gl_debug (DEBUG_LABEL, "END");
}


static void
gl_label_init (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	label->priv = g_new0 (glLabelPrivate, 1);

	label->priv->template     = NULL;
	label->priv->rotate_flag  = FALSE;
        label->priv->object_list  = NULL;

	label->priv->filename      = NULL;
	label->priv->modified_flag = FALSE;
	label->priv->compression   = 9;

	label->priv->merge        = NULL;
	label->priv->pixbuf_cache = gl_pixbuf_cache_new ();

        /*
         * Defaults from preferences
         */
	label->priv->default_font_family       = gl_prefs_model_get_default_font_family (gl_prefs);
	label->priv->default_font_size         = gl_prefs_model_get_default_font_size (gl_prefs);
	label->priv->default_font_weight       = gl_prefs_model_get_default_font_weight (gl_prefs);
	label->priv->default_font_italic_flag  = gl_prefs_model_get_default_font_italic_flag (gl_prefs);
	label->priv->default_text_color        = gl_prefs_model_get_default_text_color (gl_prefs);
	label->priv->default_text_alignment    = gl_prefs_model_get_default_text_alignment (gl_prefs);
	label->priv->default_text_line_spacing = gl_prefs_model_get_default_text_line_spacing (gl_prefs);
	label->priv->default_line_width        = gl_prefs_model_get_default_line_width (gl_prefs);
	label->priv->default_line_color        = gl_prefs_model_get_default_line_color (gl_prefs);
	label->priv->default_fill_color        = gl_prefs_model_get_default_fill_color (gl_prefs);

	gl_debug (DEBUG_LABEL, "END");
}


static void
gl_label_finalize (GObject *object)
{
	glLabel *label = GL_LABEL (object);
	GList   *p;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL (object));

	for (p = label->priv->object_list; p != NULL; p = p->next)
        {
		g_object_unref (G_OBJECT(p->data));
	}
        g_list_free (label->priv->object_list);

	lgl_template_free (label->priv->template);
	g_free (label->priv->filename);
	if (label->priv->merge != NULL)
        {
		g_object_unref (G_OBJECT(label->priv->merge));
	}
	gl_pixbuf_cache_free (label->priv->pixbuf_cache);
        g_free (label->priv->default_font_family);

	g_free (label->priv);

	G_OBJECT_CLASS (gl_label_parent_class)->finalize (object);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* New label.                                                                */
/*****************************************************************************/
GObject *
gl_label_new (void)
{
	glLabel *label;

	gl_debug (DEBUG_LABEL, "START");

	label = g_object_new (gl_label_get_type(), NULL);

	gl_debug (DEBUG_LABEL, "END");

	return G_OBJECT (label);
}


/****************************************************************************/
/* Set filename.                                                            */
/****************************************************************************/
void
gl_label_set_filename (glLabel     *label,
		       const gchar *filename)
{
	label->priv->filename = g_strdup (filename);

	g_signal_emit (G_OBJECT(label), signals[NAME_CHANGED], 0);
}


/****************************************************************************/
/* return filename.                                                         */
/****************************************************************************/
gchar *
gl_label_get_filename (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "");

	return g_strdup ( label->priv->filename );
}


/****************************************************************************/
/* return short filename.                                                   */
/****************************************************************************/
gchar *
gl_label_get_short_name (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "");

	if ( label->priv->filename == NULL )
        {

		if ( label->priv->untitled_instance == 0 )
                {
			label->priv->untitled_instance = ++untitled;
		}

		return g_strdup_printf ( "%s %d", _("Untitled"),
					 label->priv->untitled_instance );

	}
        else
        {
		gchar *temp_name, *short_name;

		temp_name = g_path_get_basename ( label->priv->filename );
		short_name = gl_file_util_remove_extension (temp_name);
		g_free (temp_name);

		return short_name;
	}
}


/****************************************************************************/
/* Is label untitled?                                                       */
/****************************************************************************/
gboolean
gl_label_is_untitled (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "return %d",(label->priv->filename == NULL));
	return (label->priv->filename == NULL);
}


/****************************************************************************/
/* Set compression level.                                                   */
/****************************************************************************/
void
gl_label_set_compression (glLabel  *label,
			  gint      compression)
{
	gl_debug (DEBUG_LABEL, "set %d", compression);

	/* Older versions of libxml2 always return a -1 for documents "read in," so
	 * default to 9.  Also, default to 9 for anything else out of range. */
	if ((compression < 0) || (compression >9))
        {
		compression = 9;
	}

	gl_debug (DEBUG_LABEL, "actual set %d", compression);
	label->priv->compression = compression;
}


/****************************************************************************/
/* Get compression level.                                                   */
/****************************************************************************/
gint
gl_label_get_compression (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "return %d", label->priv->compression);
	return label->priv->compression;
}


/****************************************************************************/
/* Set modified flag.                                                       */
/****************************************************************************/
void
gl_label_set_modified (glLabel *label)
{

	if ( !label->priv->modified_flag )
        {

		label->priv->modified_flag = TRUE;

		g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	}

}


/****************************************************************************/
/* Clear modified flag.                                                     */
/****************************************************************************/
void
gl_label_clear_modified (glLabel *label)
{

	if ( label->priv->modified_flag )
        {
		label->priv->modified_flag = FALSE;

		g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	}

}


/****************************************************************************/
/* Is label modified?                                                       */
/****************************************************************************/
gboolean
gl_label_is_modified (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "return %d", label->priv->modified_flag);
	return label->priv->modified_flag;
}


/****************************************************************************/
/* Object "changed" callback.                                               */
/****************************************************************************/
static void
object_changed_cb (glLabelObject *object,
                   glLabel       *label)
{
        do_modify (label);
}


/****************************************************************************/
/* Do modify.                                                               */
/****************************************************************************/
static void
do_modify (glLabel  *label)
{
        if ( label->priv->selection_op_flag )
        {
                label->priv->delayed_change_flag = TRUE;
        }
        else
        {
                label->priv->modified_flag = TRUE;

                g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
                g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);
        }
}


/****************************************************************************/
/* Begin selection operation.                                               */
/****************************************************************************/
static void
begin_selection_op (glLabel  *label)
{
        label->priv->selection_op_flag = TRUE;
}


/****************************************************************************/
/* End selection operation.                                                 */
/****************************************************************************/
static void
end_selection_op (glLabel  *label)
{
        label->priv->selection_op_flag = FALSE;
        if ( label->priv->delayed_change_flag )
        {
                label->priv->delayed_change_flag = FALSE;
                do_modify (label);
        }
}


/****************************************************************************/
/* set template.                                                            */
/****************************************************************************/
void
gl_label_set_template (glLabel           *label,
		       const lglTemplate *template)
{
        gchar *name;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (template);

	if ((label->priv->template == NULL) ||
            !lgl_template_do_templates_match (template, label->priv->template))
        {

		lgl_template_free (label->priv->template);
		label->priv->template = lgl_template_dup (template);

                do_modify (label);
		g_signal_emit (G_OBJECT(label), signals[SIZE_CHANGED], 0);

                name = lgl_template_get_name (template);
                gl_template_history_model_add_name (gl_template_history, name);
                g_free (name);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* get template.                                                            */
/****************************************************************************/
const lglTemplate *
gl_label_get_template (glLabel            *label)
{
        return label->priv->template;
}


/****************************************************************************/
/* set rotate flag.                                                         */
/****************************************************************************/
void
gl_label_set_rotate_flag (glLabel *label,
			  gboolean rotate_flag)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	if (rotate_flag != label->priv->rotate_flag)
        {
		label->priv->rotate_flag = rotate_flag;

                do_modify (label);
		g_signal_emit (G_OBJECT(label), signals[SIZE_CHANGED], 0);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get rotate flag.                                                         */
/****************************************************************************/
gboolean
gl_label_get_rotate_flag (glLabel       *label)
{
        return label->priv->rotate_flag;
}


/****************************************************************************/
/* Get label size.                                                          */
/****************************************************************************/
void
gl_label_get_size (glLabel *label,
		   gdouble *w,
		   gdouble *h)
{
	lglTemplate            *template;
	const lglTemplateFrame *frame;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	template = label->priv->template;
	if ( !template )
        {
		gl_debug (DEBUG_LABEL, "END -- template NULL");
		*w = *h = 0;
		return;
	}
        frame = (lglTemplateFrame *)template->frames->data;

	if (!label->priv->rotate_flag)
        {
		lgl_template_frame_get_size (frame, w, h);
	}
        else
        {
		lgl_template_frame_get_size (frame, h, w);
	}

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* set merge information structure.                                         */
/****************************************************************************/
void
gl_label_set_merge (glLabel *label,
		    glMerge *merge)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	if ( label->priv->merge != NULL )
        {
		g_object_unref (G_OBJECT(label->priv->merge));
	}
	label->priv->merge = gl_merge_dup (merge);

        do_modify (label);
	g_signal_emit (G_OBJECT(label), signals[MERGE_CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get merge information structure.                                         */
/****************************************************************************/
glMerge *
gl_label_get_merge (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), NULL);

	gl_debug (DEBUG_LABEL, "END");

	return gl_merge_dup (label->priv->merge);
}


/****************************************************************************/
/* Get pixbuf cache.                                                        */
/****************************************************************************/
GHashTable *
gl_label_get_pixbuf_cache (glLabel       *label)
{
	return label->priv->pixbuf_cache;
}


/*****************************************************************************/
/* Add object to label.                                                      */
/*****************************************************************************/
void
gl_label_add_object (glLabel       *label,
		     glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	object->parent = label;
	label->priv->object_list = g_list_append (label->priv->object_list, object);

        g_signal_connect (G_OBJECT (object), "changed",
                          G_CALLBACK (object_changed_cb), label);

        do_modify (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Delete object from label.                                                 */
/*****************************************************************************/
void
gl_label_delete_object (glLabel       *label,
                        glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        label->priv->object_list = g_list_remove (label->priv->object_list, object);

        g_signal_handlers_disconnect_by_func (G_OBJECT (object),
                                              G_CALLBACK (object_changed_cb), label);
        g_object_unref (object);

        do_modify (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get object list.                                                          */
/*****************************************************************************/
const GList *
gl_label_get_object_list (glLabel       *label)
{
        return label->priv->object_list;
}


/*****************************************************************************/
/* Select object.                                                            */
/*****************************************************************************/
void
gl_label_select_object (glLabel       *label,
                        glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        gl_label_object_select (object);

	g_signal_emit (G_OBJECT(label), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Unselect object.                                                          */
/*****************************************************************************/
void
gl_label_unselect_object (glLabel       *label,
                          glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        gl_label_object_unselect (object);

	g_signal_emit (G_OBJECT(label), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Select all objects.                                                       */
/*****************************************************************************/
void
gl_label_select_all (glLabel       *label)
{
	GList         *p;
        glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        for ( p = label->priv->object_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                gl_label_object_select (object);
        }

	g_signal_emit (G_OBJECT(label), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Unselect all objects.                                                     */
/*****************************************************************************/
void
gl_label_unselect_all (glLabel       *label)
{
	GList         *p;
        glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        for ( p = label->priv->object_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                gl_label_object_unselect (object);
        }

	g_signal_emit (G_OBJECT(label), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Select all objects contained in region.                                   */
/*****************************************************************************/
void
gl_label_select_region (glLabel       *label,
                        glLabelRegion *region)
{
	GList         *p;
	glLabelObject *object;
        gdouble        r_x1, r_y1;
        gdouble        r_x2, r_y2;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        r_x1 = MIN (region->x1, region->x2);
        r_y1 = MIN (region->y1, region->y2);
        r_x2 = MAX (region->x1, region->x2);
        r_y2 = MAX (region->y1, region->y2);

	for (p = label->priv->object_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT(p->data);

                gl_label_object_get_extent (object, &obj_extent);
                if ((obj_extent.x1 >= r_x1) &&
                    (obj_extent.x2 <= r_x2) &&
                    (obj_extent.y1 >= r_y1) &&
                    (obj_extent.y2 <= r_y2))
                {
                        gl_label_object_select (object);
                }
	}

	g_signal_emit (G_OBJECT(label), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Is selection empty?                                                       */
/*****************************************************************************/
gboolean
gl_label_is_selection_empty (glLabel       *label)
{
        GList         *p;
        glLabelObject *object;

        for ( p = label->priv->object_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                if ( gl_label_object_is_selected (object) )
                {
                        return FALSE;
                }
        }

        return TRUE;
}


/*****************************************************************************/
/* Is selection atomic?                                                      */
/*****************************************************************************/
gboolean
gl_label_is_selection_atomic (glLabel       *label)
{
        GList         *p;
        glLabelObject *object;
        gint           n_selected = 0;

        for ( p = label->priv->object_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                if ( gl_label_object_is_selected (object) )
                {
                        n_selected++;

                        if (n_selected > 1)
                        {
                                return FALSE;
                        }
                }
        }

        return (n_selected == 1);
}


/*****************************************************************************/
/* Get first selected object.                                                */
/*****************************************************************************/
glLabelObject *
gl_label_get_1st_selected_object (glLabel  *label)
{
        GList         *p;
        glLabelObject *object;

        for ( p = label->priv->object_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                if ( gl_label_object_is_selected (object) )
                {
                        return object;
                }
        }

        return NULL;
}


/*****************************************************************************/
/* Get list of selected objects.                                             */
/*****************************************************************************/
GList *
gl_label_get_selection_list (glLabel *label)
{
        GList         *selection_list = NULL;
        GList         *p;
        glLabelObject *object;

        for ( p = label->priv->object_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                if ( gl_label_object_is_selected (object) )
                {
                        selection_list = g_list_append (selection_list, object);
                }
        }

        return (selection_list);
}


/*****************************************************************************/
/* Can text properties be set for selection?                                 */
/*****************************************************************************/
gboolean
gl_label_can_selection_text (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (label && GL_IS_LABEL (label), FALSE);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		if (gl_label_object_can_text (object))
                {
                        g_list_free (selection_list);
			return TRUE;
		}
	}

        g_list_free (selection_list);
	return FALSE;
}


/*****************************************************************************/
/* Can fill properties be set for selection?                                 */
/*****************************************************************************/
gboolean
gl_label_can_selection_fill (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (label && GL_IS_LABEL (label), FALSE);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		if (gl_label_object_can_fill (object))
                {
                        g_list_free (selection_list);
			return TRUE;
		}

	}

        g_list_free (selection_list);
	return FALSE;
}


/*****************************************************************************/
/* Can line color properties be set for selection?                           */
/*****************************************************************************/
gboolean
gl_label_can_selection_line_color (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (label && GL_IS_LABEL (label), FALSE);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		if (gl_label_object_can_line_color (object))
                {
                        g_list_free (selection_list);
			return TRUE;
		}
	}

        g_list_free (selection_list);
	return FALSE;
}


/*****************************************************************************/
/* Can line width properties be set for selection?                           */
/*****************************************************************************/
gboolean
gl_label_can_selection_line_width (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "");

	g_return_val_if_fail (label && GL_IS_LABEL (label), FALSE);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		if (gl_label_object_can_line_width (object))
                {
                        g_list_free (selection_list);
			return TRUE;
		}

	}

        g_list_free (selection_list);
	return FALSE;
}


/*****************************************************************************/
/* Delete selection from label.                                              */
/*****************************************************************************/
void
gl_label_delete_selection (glLabel       *label)
{
        GList         *selection_list;
        GList         *p;
        glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

        for ( p = selection_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                gl_label_delete_object (label, object);
        }

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Bring selection object to front/top.                                     */
/****************************************************************************/
void
gl_label_raise_selection_to_top (glLabel       *label)
{
        GList         *selection_list;
        GList         *p;
        glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

        selection_list = gl_label_get_selection_list (label);

        for ( p = selection_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                label->priv->object_list = g_list_remove (label->priv->object_list, object);
        }

	/* Move to end of list, representing front most object */
	label->priv->object_list = g_list_concat (label->priv->object_list, selection_list);

        do_modify (label);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Send selection to rear/bottom.                                           */
/****************************************************************************/
void
gl_label_lower_selection_to_bottom (glLabel       *label)
{
        GList         *selection_list;
        GList         *p;
        glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

        selection_list = gl_label_get_selection_list (label);

        for ( p = selection_list; p != NULL; p = p->next )
        {
                object = GL_LABEL_OBJECT (p->data);

                label->priv->object_list = g_list_remove (label->priv->object_list, object);
        }

	/* Move to front of list, representing rear most object */
	label->priv->object_list = g_list_concat (selection_list, label->priv->object_list);

        do_modify (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Rotate selected objects by given angle.                                   */
/*****************************************************************************/
void
gl_label_rotate_selection (glLabel *label,
                           gdouble  theta_degs)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

        for ( p = selection_list; p != NULL; p = p->next )
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_rotate (object, theta_degs);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Rotate selected objects 90 degrees left.                                  */
/*****************************************************************************/
void
gl_label_rotate_selection_left (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

        for ( p = selection_list; p != NULL; p = p->next )
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_rotate (object, -90.0);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Rotate selected objects 90 degrees right.                                 */
/*****************************************************************************/
void
gl_label_rotate_selection_right (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

        for ( p = selection_list; p != NULL; p = p->next )
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_rotate (object, 90.0);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Flip selected objects horizontally.                                       */
/*****************************************************************************/
void
gl_label_flip_selection_horiz (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

        for ( p = selection_list; p != NULL; p = p->next )
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_flip_horiz (object);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Flip selected objects vertically.                                         */
/*****************************************************************************/
void
gl_label_flip_selection_vert (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

        for ( p = selection_list; p != NULL; p = p->next )
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_flip_vert (object);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Align selected objects to left most edge.                                 */
/*****************************************************************************/
void
gl_label_align_selection_left (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;
	gdouble        dx, x1_min;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	g_return_if_fail (!gl_label_is_selection_empty (label) &&
			  !gl_label_is_selection_atomic (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	/* find left most edge */
	p = selection_list;
	object = GL_LABEL_OBJECT (p->data);

	gl_label_object_get_extent (object, &obj_extent);
        x1_min = obj_extent.x1;
	for (p = p->next; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		if ( obj_extent.x1 < x1_min ) x1_min = obj_extent.x1;
	}

	/* now adjust the object positions to line up the left edges */
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		dx = x1_min - obj_extent.x1;
		gl_label_object_set_position_relative (object, dx, 0.0);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Align selected objects to right most edge.                                */
/*****************************************************************************/
void
gl_label_align_selection_right (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;
	gdouble        dx, x2_max;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	g_return_if_fail (!gl_label_is_selection_empty (label) &&
			  !gl_label_is_selection_atomic (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	/* find left most edge */
	p = selection_list;
	object = GL_LABEL_OBJECT (p->data);

	gl_label_object_get_extent (object, &obj_extent);
        x2_max = obj_extent.x2;
	for (p = p->next; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		if ( obj_extent.x2 > x2_max ) x2_max = obj_extent.x2;
	}

	/* now adjust the object positions to line up the left edges */
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		dx = x2_max - obj_extent.x2;
		gl_label_object_set_position_relative (object, dx, 0.0);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Align selected objects to horizontal center of objects.                   */
/*****************************************************************************/
void
gl_label_align_selection_hcenter (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;
	gdouble        dx;
	gdouble        dxmin;
	gdouble        xsum, xavg;
        glLabelRegion  obj_extent;
	gdouble        xcenter;
	gint           n;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	g_return_if_fail (!gl_label_is_selection_empty (label) &&
			  !gl_label_is_selection_atomic (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	/* find average center of objects */
	xsum = 0.0;
	n = 0;
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		xsum += (obj_extent.x1 + obj_extent.x2) / 2.0;
		n++;
	}
	xavg = xsum / n;

	/* find center of object closest to average center */
	p = selection_list;
	object = GL_LABEL_OBJECT (p->data);

	gl_label_object_get_extent (object, &obj_extent);
	dxmin = fabs (xavg - (obj_extent.x1 + obj_extent.x2)/2.0);
	xcenter = (obj_extent.x1 + obj_extent.x2)/2.0;
	for (p = p->next; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		dx = fabs (xavg - (obj_extent.x1 + obj_extent.x2)/2.0);
		if ( dx < dxmin )
                {
			dxmin = dx;
			xcenter = (obj_extent.x1 + obj_extent.x2)/2.0;
		}
	}

	/* now adjust the object positions to line up this center */
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		dx = xcenter - (obj_extent.x1 + obj_extent.x2)/2.0;
		gl_label_object_set_position_relative (object, dx, 0.0);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Align selected objects to top most edge.                                  */
/*****************************************************************************/
void
gl_label_align_selection_top (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;
	gdouble        dy, y1_min;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	g_return_if_fail (!gl_label_is_selection_empty (label) &&
			  !gl_label_is_selection_atomic (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	/* find top most edge */
	p = selection_list;
	object = GL_LABEL_OBJECT (p->data);

	gl_label_object_get_extent (object, &obj_extent);
        y1_min = obj_extent.y1;
	for (p = p->next; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		if ( obj_extent.y1 < y1_min ) y1_min = obj_extent.y1;
	}

	/* now adjust the object positions to line up the top edges */
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		dy = y1_min - obj_extent.y1;
		gl_label_object_set_position_relative (object, 0.0, dy);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Align selected objects to bottom most edge.                               */
/*****************************************************************************/
void
gl_label_align_selection_bottom (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;
	gdouble        dy, y2_max;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	g_return_if_fail (!gl_label_is_selection_empty (label) &&
			  !gl_label_is_selection_atomic (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	/* find bottom most edge */
	p = selection_list;
	object = GL_LABEL_OBJECT (p->data);

	gl_label_object_get_extent (object, &obj_extent);
        y2_max = obj_extent.y2;
	for (p = p->next; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		if ( obj_extent.y2 > y2_max ) y2_max = obj_extent.y2;
	}

	/* now adjust the object positions to line up the bottom edges */
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		dy = y2_max - obj_extent.y2;
		gl_label_object_set_position_relative (object, 0.0, dy);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Align selected objects to viertical center of objects.                    */
/*****************************************************************************/
void
gl_label_align_selection_vcenter (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;
	gdouble        dy;
	gdouble        dymin;
	gdouble        ysum, yavg;
        glLabelRegion  obj_extent;
	gdouble        ycenter;
	gint           n;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	g_return_if_fail (!gl_label_is_selection_empty (label) &&
			  !gl_label_is_selection_atomic (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	/* find average center of objects */
	ysum = 0.0;
	n = 0;
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		ysum += (obj_extent.y1 + obj_extent.y2) / 2.0;
		n++;
	}
	yavg = ysum / n;

	/* find center of object closest to average center */
	p = selection_list;
	object = GL_LABEL_OBJECT (p->data);

	gl_label_object_get_extent (object, &obj_extent);
	dymin = fabs (yavg - (obj_extent.y1 + obj_extent.y2)/2.0);
	ycenter = (obj_extent.y1 + obj_extent.y2)/2.0;
	for (p = p->next; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		dy = fabs (yavg - (obj_extent.y1 + obj_extent.y2)/2.0);
		if ( dy < dymin )
                {
			dymin = dy;
			ycenter = (obj_extent.y1 + obj_extent.y2)/2.0;
		}
	}

	/* now adjust the object positions to line up this center */
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		dy = ycenter - (obj_extent.y1 + obj_extent.y2)/2.0;
		gl_label_object_set_position_relative (object, 0.0, dy);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Center selected objects to in center of label.                            */
/*****************************************************************************/
void
gl_label_center_selection_horiz (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;
	gdouble        dx;
	gdouble        x_label_center;
	gdouble        x_obj_center;
	glLabelRegion  obj_extent;
	gdouble        w, h;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	g_return_if_fail (!gl_label_is_selection_empty (label));

        begin_selection_op (label);

	gl_label_get_size (label, &w, &h);
	x_label_center = w / 2.0;

	/* adjust the object positions */
        selection_list = gl_label_get_selection_list (label);
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		x_obj_center = (obj_extent.x1 + obj_extent.x2) / 2.0;
		dx = x_label_center - x_obj_center;
		gl_label_object_set_position_relative (object, dx, 0.0);
	}
        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Center selected objects to in center of label.                            */
/*****************************************************************************/
void
gl_label_center_selection_vert (glLabel *label)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;
	gdouble        dy;
	gdouble        y_label_center;
	gdouble        y_obj_center;
	glLabelRegion  obj_extent;
	gdouble        w, h;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	g_return_if_fail (!gl_label_is_selection_empty (label));

        begin_selection_op (label);

	gl_label_get_size (label, &w, &h);
	y_label_center = h / 2.0;

	/* adjust the object positions */
        selection_list = gl_label_get_selection_list (label);
	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_get_extent (object, &obj_extent);
		y_obj_center = (obj_extent.y1 + obj_extent.y2) / 2.0;
		dy = y_label_center - y_obj_center;
		gl_label_object_set_position_relative (object, 0.0, dy);
	}
        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Move selected objects                                                     */
/*****************************************************************************/
void
gl_label_move_selection (glLabel  *label,
                         gdouble   dx,
                         gdouble   dy)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);

		gl_label_object_set_position_relative (object, dx, dy);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set font family for all text contained in selected objects.               */
/*****************************************************************************/
void
gl_label_set_selection_font_family (glLabel      *label,
                                    const gchar  *font_family)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_font_family (object, font_family);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set font size for all text contained in selected objects.                 */
/*****************************************************************************/
void
gl_label_set_selection_font_size (glLabel  *label,
                                  gdouble   font_size)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_font_size (object, font_size);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set font weight for all text contained in selected objects.               */
/*****************************************************************************/
void
gl_label_set_selection_font_weight (glLabel      *label,
                                    PangoWeight   font_weight)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_font_weight (object, font_weight);
	}

        g_list_free (selection_list);


	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set font italic flag for all text contained in selected objects.          */
/*****************************************************************************/
void
gl_label_set_selection_font_italic_flag (glLabel   *label,
                                         gboolean   font_italic_flag)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_font_italic_flag (object, font_italic_flag);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set text alignment for all text contained in selected objects.            */
/*****************************************************************************/
void
gl_label_set_selection_text_alignment (glLabel        *label,
                                       PangoAlignment  text_alignment)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_text_alignment (object, text_alignment);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set text line spacing for all text contained in selected objects.         */
/*****************************************************************************/
void
gl_label_set_selection_text_line_spacing (glLabel  *label,
                                          gdouble   text_line_spacing)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_text_line_spacing (object, text_line_spacing);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set text color for all text contained in selected objects.                */
/*****************************************************************************/
void
gl_label_set_selection_text_color (glLabel      *label,
                                   glColorNode  *text_color_node)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_text_color (object, text_color_node);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set fill color for all selected objects.                                  */
/*****************************************************************************/
void
gl_label_set_selection_fill_color (glLabel      *label,
                                   glColorNode  *fill_color_node)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_fill_color (object, fill_color_node);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set line color for all selected objects.                                  */
/*****************************************************************************/
void
gl_label_set_selection_line_color (glLabel      *label,
                                   glColorNode  *line_color_node)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_line_color (object, line_color_node);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set line width for all selected objects.                                  */
/*****************************************************************************/
void
gl_label_set_selection_line_width (glLabel  *label,
				  gdouble  line_width)
{
        GList         *selection_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        begin_selection_op (label);

        selection_list = gl_label_get_selection_list (label);

	for (p = selection_list; p != NULL; p = p->next)
        {
		object = GL_LABEL_OBJECT (p->data);
		gl_label_object_set_line_width (object, line_width);
	}

        g_list_free (selection_list);

        end_selection_op (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* "Cut" selected items and place on clipboard.                              */
/*****************************************************************************/
void
gl_label_cut_selection (glLabel       *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	gl_label_copy_selection (label);
	gl_label_delete_selection (label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* "Copy" selected items to clipboard.                                       */
/*****************************************************************************/
void
gl_label_copy_selection (glLabel       *label)
{
        GtkClipboard      *clipboard;
	GList             *selection_list;
        glLabel           *label_copy;
	GList             *p;
	glLabelObject     *object;
        gchar             *buffer;
        glXMLLabelStatus   status;
        const GdkPixbuf   *pixbuf;

        ClipboardData     *data;

        static GtkTargetEntry glabels_targets[] = { { "application/glabels", 0, 0 },
                                                    { "text/xml",            0, 0 },
        };

        GtkTargetList  *target_list;
        GtkTargetEntry *target_table;
        guint           n_targets;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

        selection_list = gl_label_get_selection_list (label);

	if (selection_list)
        {

                data = g_new0 (ClipboardData, 1);

                target_list = gtk_target_list_new (glabels_targets, G_N_ELEMENTS(glabels_targets));

                /*
                 * Serialize selection by encoding as an XML label document.
                 */
		label_copy = GL_LABEL(gl_label_new ());

		gl_label_set_template (label_copy, label->priv->template);
		gl_label_set_rotate_flag (label_copy, label->priv->rotate_flag);

		for (p = selection_list; p != NULL; p = p->next)
                {
			object = GL_LABEL_OBJECT (p->data);

			gl_label_object_dup (object, label_copy);
		}

                data->xml_buffer = gl_xml_label_save_buffer (label_copy, &status);

                g_object_unref (G_OBJECT (label_copy));


                /*
                 * Is it an atomic text selection?  If so, also make available as text.
                 */
                if ( gl_label_is_selection_atomic (label) &&
                     GL_IS_LABEL_TEXT (selection_list->data) )
                {
                        glLabelText *text_object = GL_LABEL_TEXT (selection_list->data);

                        gtk_target_list_add_text_targets (target_list, 1);

                        data->text = gl_label_text_get_text (text_object);
                }


                /*
                 * Is it an atomic image selection?  If so, also make available as pixbuf.
                 */
                if ( gl_label_is_selection_atomic (label) &&
                     GL_IS_LABEL_IMAGE (selection_list->data) )
                {
                        glLabelImage       *image_object = GL_LABEL_IMAGE (selection_list->data);
                        const GdkPixbuf    *pixbuf = gl_label_image_get_pixbuf (image_object, NULL);

                        gtk_target_list_add_image_targets (target_list, 2, TRUE);

                        data->pixbuf = g_object_ref (G_OBJECT (pixbuf));
                }


                target_table = gtk_target_table_new_from_list (target_list, &n_targets);

                gtk_clipboard_set_with_data (clipboard,
                                             target_table, n_targets,
                                             (GtkClipboardGetFunc)clipboard_get_cb,
                                             (GtkClipboardClearFunc)clipboard_clear_cb,
                                             data);

                gtk_target_table_free (target_table, n_targets);
                gtk_target_list_unref (target_list);
	}


        g_list_free (selection_list);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* "Paste" from clipboard.                                                   */
/*****************************************************************************/
void
gl_label_paste (glLabel       *label)
{
        GtkClipboard  *clipboard;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

        gtk_clipboard_request_targets (clipboard,
                                       (GtkClipboardTargetsReceivedFunc)receive_targets_cb,
                                       label);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Is there anything that can be pasted?                                     */
/*****************************************************************************/
gboolean
gl_label_can_paste (glLabel       *label)
{
        GtkClipboard *clipboard;
        gboolean      can_flag;

	gl_debug (DEBUG_LABEL, "START");

        clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

        can_flag = gtk_clipboard_wait_is_target_available (clipboard,
                                                           gdk_atom_intern("application/glabels", TRUE))
                || gtk_clipboard_wait_is_text_available (clipboard)
                || gtk_clipboard_wait_is_image_available (clipboard);

	gl_debug (DEBUG_LABEL, "END");
        return can_flag;
}


/****************************************************************************/
/* Clipboard "Get" function.                                                */
/****************************************************************************/
static void
clipboard_get_cb (GtkClipboard     *clipboard,
                  GtkSelectionData *selection_data,
                  guint             info,
                  ClipboardData    *data)
{
	gl_debug (DEBUG_LABEL, "START");

        switch (info)
        {

        case 0:
                gtk_selection_data_set (selection_data,
                                        gtk_selection_data_get_target (selection_data),
                                        8,
                                        data->xml_buffer, strlen (data->xml_buffer));
                break;

        case 1:
                gtk_selection_data_set_text (selection_data, data->text, -1);
                break;

        case 2:
                gtk_selection_data_set_pixbuf (selection_data, data->pixbuf);
                break;

        default:
                g_assert_not_reached ();
                break;

        }

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Clipboard "Clear" function.                                              */
/****************************************************************************/
static void
clipboard_clear_cb (GtkClipboard     *clipboard,
                    ClipboardData    *data)
{
	gl_debug (DEBUG_LABEL, "START");

        g_free (data->xml_buffer);
        g_free (data->text);
        if (data->pixbuf)
        {
                g_object_unref (data->pixbuf);
        }

        g_free (data);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Deal with clipboard data.                                                */
/****************************************************************************/
static void
receive_targets_cb (GtkClipboard *clipboard,
                    GdkAtom      *targets,
                    gint          n_targets,
                    glLabel      *label)
{
        gint i;

        /*
         * Application/glabels
         */
        for ( i = 0; i < n_targets; i++ )
        {
                if ( strcmp(gdk_atom_name(targets[i]), "application/glabels") == 0 )
                {
                        gtk_clipboard_request_contents (clipboard,
                                                        gdk_atom_intern("application/glabels", TRUE),
                                                        (GtkClipboardReceivedFunc)paste_xml_received_cb,
                                                        label);
                        return;
                }
        }

        /*
         * Text
         */
        if ( gtk_targets_include_text (targets, n_targets) )
        {
                gtk_clipboard_request_text (clipboard,
                                            (GtkClipboardTextReceivedFunc)paste_text_received_cb,
                                            label);
                return;
        }

        /*
         * Image
         */
        if ( gtk_targets_include_image (targets, n_targets, TRUE) )
        {
                gtk_clipboard_request_image (clipboard,
                                             (GtkClipboardImageReceivedFunc)paste_image_received_cb,
                                             label);
                return;
        }
}


/****************************************************************************/
/* Paste received glabels XML callback.                                     */
/****************************************************************************/
static void
paste_xml_received_cb (GtkClipboard     *clipboard,
                       GtkSelectionData *selection_data,
                       glLabel          *label)
{
        const gchar      *xml_buffer;
        glLabel          *label_copy;
        glXMLLabelStatus  status;
        GList            *p;
        glLabelObject    *object, *newobject;

	gl_debug (DEBUG_LABEL, "START");

        xml_buffer = gtk_selection_data_get_data (selection_data);

        /*
         * Deserialize XML label document and extract objects.
         */
        label_copy = gl_xml_label_open_buffer (xml_buffer, &status);
        if ( label_copy )
        {
                gl_label_unselect_all (label);

                for (p = label_copy->priv->object_list; p != NULL; p = p->next)
                {
                        object = (glLabelObject *) p->data;
                        newobject = gl_label_object_dup (object, label);

                        gl_label_select_object (label, newobject);

                        gl_debug (DEBUG_LABEL, "object pasted");
                }

                g_object_unref (G_OBJECT (label_copy));
        }

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Paste received text callback.                                            */
/****************************************************************************/
static void
paste_text_received_cb (GtkClipboard     *clipboard,
                        const gchar      *text,
                        glLabel          *label)
{
        glLabelObject    *object;

	gl_debug (DEBUG_LABEL, "START");

        gl_label_unselect_all (label);

        object = GL_LABEL_OBJECT (gl_label_text_new (label));
        gl_label_text_set_text (GL_LABEL_TEXT (object), text);
        gl_label_object_set_position (object, 18, 18);

        gl_label_select_object (label, object);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Paste received image callback.                                           */
/****************************************************************************/
static void
paste_image_received_cb (GtkClipboard     *clipboard,
                         GdkPixbuf        *pixbuf,
                         glLabel          *label)
{
        glLabelObject    *object;

	gl_debug (DEBUG_LABEL, "START");

        gl_label_unselect_all (label);

        object = GL_LABEL_OBJECT (gl_label_image_new (label));
        gl_label_image_set_pixbuf (GL_LABEL_IMAGE (object), pixbuf);
        gl_label_object_set_position (object, 18, 18);

        gl_label_select_object (label, object);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default font family.                                                 */
/****************************************************************************/
void
gl_label_set_default_font_family (glLabel     *label,
                                  const gchar *font_family)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        g_free (label->priv->default_font_family);
	label->priv->default_font_family = g_strdup (font_family);

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default font size.                                                   */
/****************************************************************************/
void
gl_label_set_default_font_size (glLabel *label,
                                gdouble  font_size)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_font_size = font_size;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default font weight.                                                 */
/****************************************************************************/
void
gl_label_set_default_font_weight (glLabel     *label,
                                  PangoWeight  font_weight)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_font_weight = font_weight;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default font italic flag.                                            */
/****************************************************************************/
void
gl_label_set_default_font_italic_flag (glLabel  *label,
                                       gboolean  font_italic_flag)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_font_italic_flag = font_italic_flag;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default text color.                                                  */
/****************************************************************************/
void
gl_label_set_default_text_color (glLabel *label,
                                 guint    text_color)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_text_color = text_color;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default text alignment.                                              */
/****************************************************************************/
void
gl_label_set_default_text_alignment (glLabel        *label,
                                     PangoAlignment  text_alignment)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_text_alignment = text_alignment;
	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default text line spacing.                                           */
/****************************************************************************/
void
gl_label_set_default_text_line_spacing (glLabel *label,
                                        gdouble  text_line_spacing)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_text_line_spacing = text_line_spacing;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default line width.                                                  */
/****************************************************************************/
void
gl_label_set_default_line_width (glLabel *label,
                                 gdouble  line_width)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_line_width = line_width;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default line color.                                                  */
/****************************************************************************/
void
gl_label_set_default_line_color (glLabel *label,
                                 guint    line_color)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_line_color = line_color;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Set default fill color.                                                  */
/****************************************************************************/
void
gl_label_set_default_fill_color (glLabel *label,
                                 guint    fill_color)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	label->priv->default_fill_color = fill_color;

	gl_debug (DEBUG_LABEL, "END");
}


/****************************************************************************/
/* Get default font family.                                                 */
/****************************************************************************/
gchar *
gl_label_get_default_font_family (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), NULL);

	gl_debug (DEBUG_LABEL, "END");

	return g_strdup (label->priv->default_font_family);
}


/****************************************************************************/
/* Get default font size.                                                   */
/****************************************************************************/
gdouble
gl_label_get_default_font_size (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), 12.0);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_font_size;
}


/****************************************************************************/
/* Get default font weight.                                                 */
/****************************************************************************/
PangoWeight
gl_label_get_default_font_weight (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), PANGO_WEIGHT_NORMAL);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_font_weight;
}


/****************************************************************************/
/* Get default font italic flag.                                            */
/****************************************************************************/
gboolean
gl_label_get_default_font_italic_flag (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), FALSE);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_font_italic_flag;
}


/****************************************************************************/
/* Get default text color.                                                  */
/****************************************************************************/
guint
gl_label_get_default_text_color (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), 0);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_text_color;
}


/****************************************************************************/
/* Get default text alignment.                                              */
/****************************************************************************/
PangoAlignment
gl_label_get_default_text_alignment (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), PANGO_ALIGN_LEFT);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_text_alignment;
}


/****************************************************************************/
/* Get default text line spacing.                                           */
/****************************************************************************/
gdouble
gl_label_get_default_text_line_spacing (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), 1.0);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_text_line_spacing;
}


/****************************************************************************/
/* Get default line width.                                                  */
/****************************************************************************/
gdouble
gl_label_get_default_line_width (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), 1.0);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_line_width;
}


/****************************************************************************/
/* Get default line color.                                                  */
/****************************************************************************/
guint
gl_label_get_default_line_color (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), 0);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_line_color;
}


/****************************************************************************/
/* Get default fill color.                                                  */
/****************************************************************************/
guint
gl_label_get_default_fill_color (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), 0);

	gl_debug (DEBUG_LABEL, "END");

	return label->priv->default_fill_color;
}


/****************************************************************************/
/* Draw label.                                                              */
/****************************************************************************/
void
gl_label_draw (glLabel       *label,
               cairo_t       *cr,
               gboolean       screen_flag,
               glMergeRecord *record)
{
	GList            *p_obj;
	glLabelObject    *object;

	g_return_if_fail (label && GL_IS_LABEL (label));

	for (p_obj = label->priv->object_list; p_obj != NULL; p_obj = p_obj->next)
        {
		object = GL_LABEL_OBJECT (p_obj->data);

                gl_label_object_draw (object, cr, screen_flag, record);
	}
}


/****************************************************************************/
/* Get object located at coordinates.                                       */
/****************************************************************************/
glLabelObject *gl_label_object_at              (glLabel       *label,
                                                cairo_t       *cr,
                                                gdouble        x_pixels,
                                                gdouble        y_pixels)
{
	GList            *p_obj;
	glLabelObject    *object;

	g_return_val_if_fail (label && GL_IS_LABEL (label), NULL);

	for (p_obj = g_list_last (label->priv->object_list); p_obj != NULL; p_obj = p_obj->prev)
        {
		object = GL_LABEL_OBJECT (p_obj->data);

                if (gl_label_object_is_located_at (object, cr, x_pixels, y_pixels))
                {
                        return object;
                }

	}

        return NULL;
}


/****************************************************************************/
/* Return handle and associated object at coordinates.                      */
/****************************************************************************/
glLabelObject *
gl_label_get_handle_at (glLabel             *label,
                        cairo_t             *cr,
                        gdouble              x_pixels,
                        gdouble              y_pixels,
                        glLabelObjectHandle *handle)
{
        GList            *selection_list;
	GList            *p_obj;
	glLabelObject    *object;

	g_return_val_if_fail (label && GL_IS_LABEL (label), NULL);

        selection_list = gl_label_get_selection_list (label);

	for (p_obj = g_list_last (selection_list); p_obj != NULL; p_obj = p_obj->prev)
        {

		object = GL_LABEL_OBJECT (p_obj->data);

                if ((*handle = gl_label_object_handle_at (object, cr, x_pixels, y_pixels)))
                {
                        g_list_free (selection_list);
                        return object;
                }

	}

        g_list_free (selection_list);

        *handle = GL_LABEL_OBJECT_HANDLE_NONE;
        return NULL;
}


/****************************************************************************/
/* Can undo?                                                                */
/****************************************************************************/
gboolean
gl_label_can_undo (glLabel *label)
{
	return FALSE;
}


/****************************************************************************/
/* Can redo?                                                                */
/****************************************************************************/
gboolean
gl_label_can_redo (glLabel *label)
{
	return FALSE;
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
