/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label.c:  GLabels label module
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

#include <gnome.h>

#include "label.h"
#include "label-object.h"
#include "marshal.h"
#include "util.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelPrivate {

        glTemplate  *template;
        gboolean     rotate_flag;

	gchar       *filename;
	gint         compression;
	gboolean     modified_flag;
	gint         untitled_instance;

	glMerge     *merge;

	GHashTable  *pixbuf_cache;
};

enum {
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

static GObjectClass *parent_class = NULL;

static guint signals[LAST_SIGNAL] = {0};

static guint untitled = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_label_class_init    (glLabelClass *klass);
static void gl_label_instance_init (glLabel      *label);
static void gl_label_finalize      (GObject      *object);

static void object_changed_cb      (glLabelObject *object,
				    glLabel       *label);

static void object_moved_cb        (glLabelObject *object,
			            gdouble        x,
				    gdouble        y,
				    glLabel       *label);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glLabelClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_class_init,
			NULL,
			NULL,
			sizeof (glLabel),
			0,
			(GInstanceInitFunc) gl_label_instance_init,
			NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "glLabel", &info, 0);
	}

	return type;
}

static void
gl_label_class_init (glLabelClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gl_debug (DEBUG_LABEL, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_label_finalize;

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
gl_label_instance_init (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	label->private = g_new0 (glLabelPrivate, 1);
	label->private->merge = NULL;
	label->private->pixbuf_cache = gl_pixbuf_cache_new ();

	gl_debug (DEBUG_LABEL, "END");
}

static void
gl_label_finalize (GObject *object)
{
	glLabel *label;
	GList   *p, *p_next;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (object && GL_IS_LABEL (object));

	label = GL_LABEL (object);

	gl_template_free (label->private->template);

	for (p = label->objects; p != NULL; p = p_next) {
		p_next = p->next;	/* NOTE: p will be left dangling */
		g_object_unref (G_OBJECT(p->data));
	}

	if (label->private->merge != NULL) {
		g_object_unref (G_OBJECT(label->private->merge));
	}

	gl_pixbuf_cache_free (label->private->pixbuf_cache);

	g_free (label->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_LABEL, "END");
}

GObject *
gl_label_new (void)
{
	glLabel *label;

	gl_debug (DEBUG_LABEL, "START");

	label = g_object_new (gl_label_get_type(), NULL);

	label->private->compression = 9;

	label->private->modified_flag = FALSE;

	gl_debug (DEBUG_LABEL, "END");

	return G_OBJECT (label);
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
	g_return_if_fail (GL_IS_LABEL_OBJECT (object));

	object->parent = label;
	label->objects = g_list_append (label->objects, object);

	label->private->modified_flag = TRUE;

	g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);

	g_signal_connect (G_OBJECT(object), "changed",
			  G_CALLBACK(object_changed_cb), label);

	g_signal_connect (G_OBJECT(object), "moved",
			  G_CALLBACK(object_moved_cb), label);

	gl_debug (DEBUG_LABEL, "END");
}

/*****************************************************************************/
/* Remove object from label.                                                 */
/*****************************************************************************/
void
gl_label_remove_object (glLabel       *label,
			glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (GL_IS_LABEL_OBJECT (object));

	object->parent = NULL;
	label->objects = g_list_remove (label->objects, object);

	if ( G_OBJECT(label)->ref_count /* not finalized */ ) {

		g_signal_handlers_disconnect_by_func (object,
						      G_CALLBACK(object_changed_cb),
						      label);
		g_signal_handlers_disconnect_by_func (object,
						      G_CALLBACK(object_moved_cb),
						      label);

		label->private->modified_flag = TRUE;

		g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
		g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);

	}

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Object changed callback.                                        */
/*---------------------------------------------------------------------------*/
static void
object_changed_cb (glLabelObject *object,
		   glLabel       *label)
{

	if ( !label->private->modified_flag ) {

		label->private->modified_flag = TRUE;

		g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	}

	g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Object moved callback.                                          */
/*---------------------------------------------------------------------------*/
static void
object_moved_cb (glLabelObject *object,
		 gdouble        x,
		 gdouble        y,
		 glLabel       *label)
{

	if ( !label->private->modified_flag ) {

		label->private->modified_flag = TRUE;

		g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	}

	g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);
}

/****************************************************************************/
/* Bring label object to front/top.                                         */
/****************************************************************************/
void
gl_label_raise_object_to_top (glLabel       *label,
			      glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	/* Move to end of list, representing front most object */
	label->objects = g_list_remove (label->objects, object);
	label->objects = g_list_append (label->objects, object);

	label->private->modified_flag = TRUE;

	g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Send label object to rear/bottom.                                        */
/****************************************************************************/
void
gl_label_lower_object_to_bottom (glLabel       *label,
				 glLabelObject *object)
{
	gl_debug (DEBUG_LABEL, "START");

	/* Move to front of list, representing rear most object */
	label->objects = g_list_remove (label->objects, object);
	label->objects = g_list_prepend (label->objects, object);

	label->private->modified_flag = TRUE;

	g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* set template.                                                            */
/****************************************************************************/
extern void
gl_label_set_template (glLabel    *label,
		       glTemplate *template)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	if ((label->private->template == NULL) ||
	    (g_strcasecmp (template->name, label->private->template->name) != 0)) {

		gl_template_free (label->private->template);
		label->private->template = gl_template_dup (template);

		label->private->modified_flag = TRUE;

		g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
		g_signal_emit (G_OBJECT(label), signals[SIZE_CHANGED], 0);
		g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);

	}

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* set rotate flag.                                                         */
/****************************************************************************/
extern void
gl_label_set_rotate_flag (glLabel *label,
			  gboolean rotate_flag)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	if (rotate_flag != label->private->rotate_flag) {

		label->private->rotate_flag = rotate_flag;

		label->private->modified_flag = TRUE;

		g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
		g_signal_emit (G_OBJECT(label), signals[SIZE_CHANGED], 0);
		g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);

	}

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Get template.                                                            */
/****************************************************************************/
glTemplate *
gl_label_get_template (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	gl_debug (DEBUG_LABEL, "END");

	return gl_template_dup (label->private->template);
}

/****************************************************************************/
/* Get rotate flag.                                                         */
/****************************************************************************/
gboolean
gl_label_get_rotate_flag (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	gl_debug (DEBUG_LABEL, "END");

	return label->private->rotate_flag;
}

/****************************************************************************/
/* Get label size.                                                          */
/****************************************************************************/
void
gl_label_get_size (glLabel *label,
		   gdouble *w,
		   gdouble *h)
{
	glTemplate                *template;
	const glTemplateLabelType *label_type;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	template = label->private->template;
	if ( !template ) {
		gl_debug (DEBUG_LABEL, "END -- template NULL");
		*w = *h = 0;
		return;
	}
	label_type = gl_template_get_first_label_type (template);

	if (!label->private->rotate_flag) {
		gl_template_get_label_size (label_type, w, h);
	} else {
		gl_template_get_label_size (label_type, h, w);
	}

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* set merge information structure.                                         */
/****************************************************************************/
extern void
gl_label_set_merge (glLabel *label,
		    glMerge *merge)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	if ( label->private->merge != NULL ) {
		g_object_unref (G_OBJECT(label->private->merge));
	}
	label->private->merge = gl_merge_dup (merge);

	label->private->modified_flag = TRUE;

	g_signal_emit (G_OBJECT(label), signals[MERGE_CHANGED], 0);
	g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	g_signal_emit (G_OBJECT(label), signals[CHANGED], 0);

	gl_debug (DEBUG_LABEL, "END");
}

/****************************************************************************/
/* Get merge information structure.                                         */
/****************************************************************************/
glMerge *
gl_label_get_merge (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

	gl_debug (DEBUG_LABEL, "END");

	return gl_merge_dup (label->private->merge);
}

/****************************************************************************/
/* return filename.                                                         */
/****************************************************************************/
gchar *
gl_label_get_filename (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "");

	return g_strdup ( label->private->filename );
}

/****************************************************************************/
/* return short filename.                                                   */
/****************************************************************************/
gchar *
gl_label_get_short_name (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "");

	if ( label->private->filename == NULL ) {

		if ( label->private->untitled_instance == 0 ) {
			label->private->untitled_instance = ++untitled;
		}

		return g_strdup_printf ( "%s %d", _("Untitled"),
					 label->private->untitled_instance );

	} else {
		gchar *temp_name, *short_name;

		temp_name = g_path_get_basename ( label->private->filename );
		short_name = gl_util_remove_extension (temp_name);
		g_free (temp_name);

		return short_name;
	}
}

/****************************************************************************/
/* Get pixbuf cache.                                                        */
/****************************************************************************/
GHashTable *
gl_label_get_pixbuf_cache (glLabel       *label)
{
	return label->private->pixbuf_cache;
}

/****************************************************************************/
/* Is label modified?                                                       */
/****************************************************************************/
gboolean
gl_label_is_modified (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "return %d", label->private->modified_flag);
	return label->private->modified_flag;
}

/****************************************************************************/
/* Is label untitled?                                                       */
/****************************************************************************/
gboolean
gl_label_is_untitled (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "return %d",(label->private->filename == NULL));
	return (label->private->filename == NULL);
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


/****************************************************************************/
/* Set filename.                                                            */
/****************************************************************************/
void
gl_label_set_filename (glLabel     *label,
		       const gchar *filename)
{
	label->private->filename = g_strdup (filename);

	g_signal_emit (G_OBJECT(label), signals[NAME_CHANGED], 0);
}

/****************************************************************************/
/* Clear modified flag.                                                     */
/****************************************************************************/
void
gl_label_clear_modified (glLabel *label)
{

	if ( label->private->modified_flag ) {

		label->private->modified_flag = FALSE;

		g_signal_emit (G_OBJECT(label), signals[MODIFIED_CHANGED], 0);
	}

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
	if ((compression < 0) || (compression >9)) {
		compression = 9;
	}

	gl_debug (DEBUG_LABEL, "actual set %d", compression);
	label->private->compression = compression;
}


/****************************************************************************/
/* Get compression level.                                                   */
/****************************************************************************/
gint
gl_label_get_compression (glLabel *label)
{
	gl_debug (DEBUG_LABEL, "return %d", label->private->compression);
	return label->private->compression;
}

