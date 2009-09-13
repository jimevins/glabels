/*
 *  glabels-batch.c
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

#include <glib/gi18n.h>

#include "merge-init.h"
#include "xml-label.h"
#include "print.h"
#include "print-op.h"
#include <libglabels/db.h>
#include "util.h"
#include <math.h>
#include "prefs.h"
#include "debug.h"

/*============================================*/
/* Private globals                            */
/*============================================*/
static gchar    *output          = "output.pdf";
static gint     n_copies         = 1;
static gint     n_sheets         = 1;
static gint     first            = 1;
static gboolean outline_flag     = FALSE;
static gboolean reverse_flag     = FALSE;
static gboolean crop_marks_flag  = FALSE;
static gchar    *input           = NULL;
static gchar    **remaining_args = NULL;

static GOptionEntry option_entries[] = {
        {"output", 'o', 0, G_OPTION_ARG_STRING, &output,
         N_("set output filename (default=\"output.pdf\")"), N_("filename")},
        {"sheets", 's', 0, G_OPTION_ARG_INT, &n_sheets,
         N_("number of sheets (default=1)"), N_("sheets")},
        {"copies", 'c', 0, G_OPTION_ARG_INT, &n_copies,
         N_("number of copies (default=1)"), N_("copies")},
        {"first", 'f', 0, G_OPTION_ARG_INT, &first,
         N_("first label on first sheet (default=1)"), N_("first")},
        {"outline", 'l', 0, G_OPTION_ARG_NONE, &outline_flag,
         N_("print outlines (to test printer alignment)"), NULL},
        {"reverse", 'r', 0, G_OPTION_ARG_NONE, &reverse_flag,
         N_("print in reverse (i.e. a mirror image)"), NULL},
        {"cropmarks", 'C', 0, G_OPTION_ARG_NONE, &crop_marks_flag,
         N_("print crop marks"), NULL},
        {"input", 'i', 0, G_OPTION_ARG_STRING, &input,
         N_("input file for merging"), N_("filename")},
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY,
          &remaining_args, NULL, N_("[FILE...]") },
        { NULL }
};



/*****************************************************************************/
/* Main                                                                      */
/*****************************************************************************/
int
main (int argc, char **argv)
{
	GOptionContext    *option_context;
        GList             *p, *file_list = NULL;
        gchar             *abs_fn;
        glLabel           *label = NULL;
        glMerge           *merge = NULL;
        lglTemplateFrame  *frame;
        glXMLLabelStatus   status;
        glPrintOp         *print_op;
	gchar	          *utf8_filename;
        GError            *error = NULL;

        bindtextdomain (GETTEXT_PACKAGE, GLABELS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

	option_context = g_option_context_new (NULL);
        g_option_context_set_summary (option_context,
                                      _("Print files created with gLabels."));
	g_option_context_add_main_entries (option_context, option_entries, GETTEXT_PACKAGE);


        /* Initialize minimal gnome program */
        gtk_init (&argc, &argv);
        if (!g_option_context_parse (option_context, &argc, &argv, &error))
	{
	        g_print(_("%s\nRun '%s --help' to see a full list of available command line options.\n"),
			error->message, argv[0]);
		g_error_free (error);
		return 1;
	}


        /* create file list */
	if (remaining_args != NULL) {
		gint i, num_args;

		num_args = g_strv_length (remaining_args);
		for (i = 0; i < num_args; ++i) {
			utf8_filename = g_filename_to_utf8 (remaining_args[i], -1, NULL, NULL, NULL);
			if (utf8_filename)
				file_list = g_list_append (file_list, utf8_filename);
		}
		g_strfreev (remaining_args);
		remaining_args = NULL;
	}

        /* initialize components */
        gl_debug_init ();
        gl_merge_init ();
        lgl_db_init ();
	gl_prefs_init ();

        /* now print the files */
        for (p = file_list; p; p = p->next) {
                g_print ("LABEL FILE = %s\n", (gchar *) p->data);
                label = gl_xml_label_open (p->data, &status);


                if ( status == XML_LABEL_OK ) {

                        merge = gl_label_get_merge (label);
                        if (input != NULL) {
                                if (merge != NULL) {
                                        gl_merge_set_src(merge, input);
                                        gl_label_set_merge(label, merge);
                                } else {
                                        fprintf ( stderr,
                                                  _("cannot perform document merge with glabels file %s\n"),
                                                  (char *)p->data );
                                }
                        }
                        abs_fn = gl_util_make_absolute ( output );
                        frame = (lglTemplateFrame *)label->template->frames->data;

                        print_op = gl_print_op_new (label);
                        gl_print_op_set_filename        (print_op, abs_fn);
                        gl_print_op_set_n_copies        (print_op, n_copies);
                        gl_print_op_set_first           (print_op, first);
                        gl_print_op_set_outline_flag    (print_op, outline_flag);
                        gl_print_op_set_reverse_flag    (print_op, reverse_flag);
                        gl_print_op_set_crop_marks_flag (print_op, crop_marks_flag);
                        if (merge)
                        {
                                gl_print_op_set_n_sheets (print_op,
                                                          ceil ((double)(first-1 + n_copies * gl_merge_get_record_count(merge))
                                                                / lgl_template_frame_get_n_labels (frame)));
                        }
                        else
                        {
                                gl_print_op_set_n_sheets (print_op, n_sheets);
                                gl_print_op_set_last     (print_op,
                                                          lgl_template_frame_get_n_labels (frame));
                        }
                        gtk_print_operation_run (GTK_PRINT_OPERATION (print_op),
                                                 GTK_PRINT_OPERATION_ACTION_EXPORT,
                                                 NULL,
                                                 NULL);

                        g_object_unref (label);
                }
                else {
                        fprintf ( stderr, _("cannot open glabels file %s\n"),
                                  (char *)p->data );
                }
        }

        g_list_free (file_list);

        return 0;
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
