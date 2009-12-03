/*
 *  ui-commands.c
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

#include "ui-commands.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "ui.h"
#include "view.h"
#include "file.h"
#include "template-designer.h"
#include "print-op-dialog.h"
#include "prefs.h"
#include "prefs-dialog.h"
#include "recent.h"
#include "debug.h"


/****************************************************************************/
/** File/New command.                                                       */
/****************************************************************************/
void 
gl_ui_cmd_file_new (GtkAction *action,
                    glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_file_new (window);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Properties command.                                                */
/****************************************************************************/
void 
gl_ui_cmd_file_properties (GtkAction *action,
                           glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_file_properties (GL_VIEW(window->view)->label, window);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Template-Designer command.                                         */
/****************************************************************************/
void
gl_ui_cmd_file_template_designer (GtkAction *action,
                                  glWindow  *window)
{
        GtkWidget *dialog;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        dialog = gl_template_designer_new (GTK_WINDOW(window));

        gtk_widget_show (dialog);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Open command.                                                      */
/****************************************************************************/
void 
gl_ui_cmd_file_open (GtkAction *action,
                     glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_file_open (window);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Open-Recent command.                                               */
/****************************************************************************/
void 
gl_ui_cmd_file_open_recent (GtkRecentChooser *chooser,
                            glWindow         *window)
{
        GtkRecentInfo *item;
        gchar         *utf8_filename;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (chooser && GTK_IS_RECENT_CHOOSER(chooser));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        item = gtk_recent_chooser_get_current_item (chooser);
        if (!item)
                return;

        utf8_filename = gl_recent_get_utf8_filename (item);

        gl_debug (DEBUG_COMMANDS, "Selected %s\n", utf8_filename);
        gl_file_open_recent (utf8_filename, window);

        gtk_recent_info_unref (item);
        
        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Save command.                                                      */
/****************************************************************************/
void 
gl_ui_cmd_file_save (GtkAction *action,
                     glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_file_save (GL_VIEW(window->view)->label, window);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Save-as command.                                                   */
/****************************************************************************/
void 
gl_ui_cmd_file_save_as (GtkAction *action,
                        glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_file_save_as (GL_VIEW(window->view)->label, window);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Print command.                                                     */
/****************************************************************************/
void
gl_ui_cmd_file_print (GtkAction *action,
                      glWindow  *window)
{
        glPrintOpDialog         *op;
        GtkPrintOperationResult  result;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        op = gl_print_op_dialog_new (GL_VIEW(window->view)->label);

        if (window->print_settings)
        {
                gl_print_op_set_settings (GL_PRINT_OP (op), window->print_settings);
        }

        result = gtk_print_operation_run (GTK_PRINT_OPERATION (op),
                                          GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                          GTK_WINDOW (window),
                                          NULL);

        if ( result == GTK_PRINT_OPERATION_RESULT_APPLY )
        {
                gl_print_op_free_settings (window->print_settings);
                window->print_settings = gl_print_op_get_settings (GL_PRINT_OP (op));
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Close command.                                                     */
/****************************************************************************/
void 
gl_ui_cmd_file_close (GtkAction *action,
                      glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_file_close (window);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** File/Quit command.                                                      */
/****************************************************************************/
void 
gl_ui_cmd_file_quit (GtkAction *action,
                     glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_file_exit ();

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Edit/Cut command.                                                       */
/****************************************************************************/
void 
gl_ui_cmd_edit_cut (GtkAction *action,
                    glWindow  *window)
{
        GtkClipboard  *glabels_clipboard;
        GtkClipboard  *std_clipboard;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        glabels_clipboard = gtk_widget_get_clipboard (GTK_WIDGET (window),
                                                      GL_UI_GLABELS_CLIPBOARD);

        std_clipboard     = gtk_widget_get_clipboard (GTK_WIDGET (window),
                                                      GDK_SELECTION_CLIPBOARD);

        gl_label_cut_selection (window->label, glabels_clipboard, std_clipboard);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Edit/Copy command.                                                      */
/****************************************************************************/
void 
gl_ui_cmd_edit_copy (GtkAction *action,
                     glWindow  *window)
{
        GtkClipboard  *glabels_clipboard;
        GtkClipboard  *std_clipboard;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        glabels_clipboard = gtk_widget_get_clipboard (GTK_WIDGET (window),
                                                      GL_UI_GLABELS_CLIPBOARD);

        std_clipboard     = gtk_widget_get_clipboard (GTK_WIDGET (window),
                                                      GDK_SELECTION_CLIPBOARD);

        gl_label_copy_selection (window->label, glabels_clipboard, std_clipboard);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Edit/Paste command.                                                     */
/****************************************************************************/
void 
gl_ui_cmd_edit_paste (GtkAction *action,
                      glWindow  *window)
{
        GtkClipboard  *glabels_clipboard;
        GtkClipboard  *std_clipboard;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        glabels_clipboard = gtk_widget_get_clipboard (GTK_WIDGET (window),
                                                      GL_UI_GLABELS_CLIPBOARD);

        std_clipboard     = gtk_widget_get_clipboard (GTK_WIDGET (window),
                                                      GDK_SELECTION_CLIPBOARD);

        gl_label_paste (window->label, glabels_clipboard, std_clipboard);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Edit/Delete command.                                                    */
/****************************************************************************/
void 
gl_ui_cmd_edit_delete (GtkAction *action,
                       glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_label_delete_selection (GL_VIEW(window->view)->label);

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Edit/Select-all command.                                                */
/****************************************************************************/
void
gl_ui_cmd_edit_select_all (GtkAction *action,
                           glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_label_select_all (GL_VIEW(window->view)->label); 

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Edit/Unselect-all command.                                              */
/****************************************************************************/
void
gl_ui_cmd_edit_unselect_all (GtkAction *action,
                             glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gl_label_unselect_all (GL_VIEW(window->view)->label); 

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Edit/Preferences command.                                               */
/****************************************************************************/
void
gl_ui_cmd_edit_preferences (GtkAction *action,
                            glWindow  *window)
{
        static GtkWidget *dialog = NULL;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (dialog != NULL)
        {
                gtk_window_present (GTK_WINDOW (dialog));
                gtk_window_set_transient_for (GTK_WINDOW (dialog),        
                                              GTK_WINDOW(window));

        } else {
                
                dialog = gl_prefs_dialog_new (GTK_WINDOW(window));

                g_signal_connect (G_OBJECT (dialog), "destroy",
                                  G_CALLBACK (gtk_widget_destroyed), &dialog);
        
                gtk_widget_show (dialog);

        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** View/Property-bar-toggle command.                                        */
/*****************************************************************************/
void
gl_ui_cmd_view_property_bar_toggle (GtkToggleAction *action,
                                    glWindow        *window)
{
        gboolean     state;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_TOGGLE_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        state =  gtk_toggle_action_get_active (action);

        gl_prefs_model_set_property_toolbar_visible (gl_prefs, state);
        if (state) {
                gtk_widget_show (GTK_WIDGET (window->property_bar));
        } else {
                gtk_widget_hide (GTK_WIDGET (window->property_bar));
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** View/Grid-toggle command.                                                */
/*****************************************************************************/
void
gl_ui_cmd_view_grid_toggle (GtkToggleAction *action,
                            glWindow        *window)
{
        gboolean     state;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_TOGGLE_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        state =  gtk_toggle_action_get_active (action);

        if (window->view != NULL)
        {
                if (state) {
                        gl_view_show_grid (GL_VIEW(window->view));
                } else {
                        gl_view_hide_grid (GL_VIEW(window->view));
                }
        }

        gl_prefs_model_set_grid_visible (gl_prefs, state);

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** View/Markup-toggle command.                                              */
/*****************************************************************************/
void
gl_ui_cmd_view_markup_toggle (GtkToggleAction *action,
                              glWindow        *window)
{
        gboolean     state;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_TOGGLE_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        state =  gtk_toggle_action_get_active (action);

        if (window->view != NULL)
        {
                if (state) {
                        gl_view_show_markup (GL_VIEW(window->view));
                } else {
                        gl_view_hide_markup (GL_VIEW(window->view));
                }
        }

        gl_prefs_model_set_markup_visible (gl_prefs, state);

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** View/Zoom-in command.                                                    */
/*****************************************************************************/
void
gl_ui_cmd_view_zoomin (GtkAction *action,
                       glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_zoom_in (GL_VIEW(window->view));
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** View/Zoom-out command.                                                   */
/*****************************************************************************/
void
gl_ui_cmd_view_zoomout (GtkAction *action,
                        glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_zoom_out (GL_VIEW(window->view));
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** View/Zoom-1:1 command.                                                   */
/*****************************************************************************/
void
gl_ui_cmd_view_zoom1to1 (GtkAction *action,
                         glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_set_zoom (GL_VIEW(window->view), 1.0);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** View/Zoom-to-fit command.                                                */
/*****************************************************************************/
void
gl_ui_cmd_view_zoom_to_fit (GtkAction *action,
                            glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_zoom_to_fit (GL_VIEW(window->view));
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Arrow-mode command.                                              */
/*****************************************************************************/
void
gl_ui_cmd_objects_arrow_mode (GtkAction *action,
                              glWindow  *window)
{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_arrow_mode (GL_VIEW(window->view));
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Create-text object command.                                      */
/*****************************************************************************/
void
gl_ui_cmd_objects_create_text (GtkAction *action,
                               glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_object_create_mode (GL_VIEW(window->view),
                                            GL_LABEL_OBJECT_TEXT);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Create-box object command.                                       */
/*****************************************************************************/
void
gl_ui_cmd_objects_create_box (GtkAction *action,
                              glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_object_create_mode (GL_VIEW(window->view),
                                            GL_LABEL_OBJECT_BOX);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Create-line object command.                                      */
/*****************************************************************************/
void
gl_ui_cmd_objects_create_line (GtkAction *action,
                               glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_object_create_mode (GL_VIEW(window->view),
                                            GL_LABEL_OBJECT_LINE);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Create-ellipse object command.                                   */
/*****************************************************************************/
void
gl_ui_cmd_objects_create_ellipse (GtkAction *action,
                                  glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_object_create_mode (GL_VIEW(window->view),
                                            GL_LABEL_OBJECT_ELLIPSE);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Create-image object command.                                     */
/*****************************************************************************/
void
gl_ui_cmd_objects_create_image (GtkAction *action,
                                glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_object_create_mode (GL_VIEW(window->view),
                                            GL_LABEL_OBJECT_IMAGE);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Create-barcode object command.                                   */
/*****************************************************************************/
void
gl_ui_cmd_objects_create_barcode (GtkAction *action,
                                  glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_view_object_create_mode (GL_VIEW(window->view),
                                            GL_LABEL_OBJECT_BARCODE);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Raise command.                                                   */
/*****************************************************************************/
void
gl_ui_cmd_objects_raise (GtkAction *action,
                         glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_raise_selection_to_top (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Lower command.                                                   */
/*****************************************************************************/
void
gl_ui_cmd_objects_lower (GtkAction *action,
                         glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_lower_selection_to_bottom (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Rotate-left-90-degrees command.                                  */
/*****************************************************************************/
void
gl_ui_cmd_objects_rotate_left (GtkAction *action,
                               glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_rotate_selection_left (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Rotate-right-90-degrees command.                                 */
/*****************************************************************************/
void
gl_ui_cmd_objects_rotate_right (GtkAction *action,
                                glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_rotate_selection_right (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Flip-horizontally command.                                       */
/*****************************************************************************/
void
gl_ui_cmd_objects_flip_horiz (GtkAction *action,
                              glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_flip_selection_horiz (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Flip-vertically command.                                         */
/*****************************************************************************/
void
gl_ui_cmd_objects_flip_vert (GtkAction *action,
                             glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_flip_selection_vert (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Align-left command.                                              */
/*****************************************************************************/
void
gl_ui_cmd_objects_align_left (GtkAction *action,
                              glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_align_selection_left (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Align-right command.                                             */
/*****************************************************************************/
void
gl_ui_cmd_objects_align_right (GtkAction *action,
                               glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_align_selection_right (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Align-horizontal-center command.                                 */
/*****************************************************************************/
void
gl_ui_cmd_objects_align_hcenter (GtkAction *action,
                                 glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_align_selection_hcenter (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Align-top command.                                               */
/*****************************************************************************/
void
gl_ui_cmd_objects_align_top (GtkAction *action,
                             glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_align_selection_top (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Align-bottom command.                                            */
/*****************************************************************************/
void
gl_ui_cmd_objects_align_bottom (GtkAction *action,
                                glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_align_selection_bottom (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Align-vertical center command.                                   */
/*****************************************************************************/
void
gl_ui_cmd_objects_align_vcenter (GtkAction *action,
                                 glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_align_selection_vcenter (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Center-horizontally command.                                     */
/*****************************************************************************/
void
gl_ui_cmd_objects_center_horiz (GtkAction *action,
                                glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_center_selection_horiz (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Center-vertically command.                                       */
/*****************************************************************************/
void
gl_ui_cmd_objects_center_vert (GtkAction *action,
                               glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->view != NULL) {
                gl_label_center_selection_vert (GL_VIEW(window->view)->label);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/*****************************************************************************/
/** Objects/Edit- merge-properties command.                                  */
/*****************************************************************************/
void
gl_ui_cmd_objects_merge_properties (GtkAction *action,
                                    glWindow  *window)

{
        gl_debug (DEBUG_COMMANDS, "START");
        
        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (window->merge_dialog) {

                gtk_window_present (GTK_WINDOW(window->merge_dialog));
                gtk_window_set_transient_for (GTK_WINDOW (window->merge_dialog),
                                              GTK_WINDOW (window));

        } else {

                window->merge_dialog =
                        g_object_ref (
                                gl_merge_properties_dialog_new (GL_VIEW(window->view)->label,
                                                                GTK_WINDOW(window)) );

                g_signal_connect (G_OBJECT(window->merge_dialog), "destroy",
                                  G_CALLBACK (gtk_widget_destroyed),
                                  &window->merge_dialog);

                gtk_widget_show (GTK_WIDGET (window->merge_dialog));

        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Help/Contents command.                                                  */
/****************************************************************************/
void 
gl_ui_cmd_help_contents (GtkAction *action,
                         glWindow  *window)
{
        GError *error = NULL;

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (window)),
                      "ghelp:glabels",
                      GDK_CURRENT_TIME, 
                      &error);
        
        if (error != NULL)
        {
                g_message ("%s", error->message);

                g_error_free (error);
        }

        gl_debug (DEBUG_COMMANDS, "END");
}


/****************************************************************************/
/** Help/About command.                                                     */
/****************************************************************************/
void 
gl_ui_cmd_help_about (GtkAction *action,
                      glWindow  *window)
{
        static GtkWidget *about = NULL;

        GdkPixbuf        *pixbuf = NULL;
        
        const gchar *authors[] = {
                "Jim Evins",
                " ",
                _("Glabels includes contributions from:"),
                "Frederic Ruaudel",
                "Wayne Schuller",
                "Emmanuel Pacaud",
                "Austin Henry",
                " ",
                _("See the file AUTHORS for additional credits,"),
                _("or visit http://glabels.org/"),
                NULL
        };
        
        const gchar *artists[] = {
                "Nestor Di",
                "Jim Evins",
                NULL
        };
        
        const gchar *copy_text = "Copyright \xc2\xa9 2001-2009 Jim Evins";

        const gchar *about_text = _("A label and business card creation program.\n");

        const gchar *url = "http://glabels.org";

        const gchar *translator_credits = _("translator-credits");

        const gchar *license = _(
                "gLabels is free software: you can redistribute it and/or modify\n"
                "it under the terms of the GNU General Public License as published by\n"
                "the Free Software Foundation, either version 3 of the License, or\n"
                "(at your option) any later version.\n"
                "\n"
                "gLabels is distributed in the hope that it will be useful,\n"
                "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                "GNU General Public License for more details.\n");

        gl_debug (DEBUG_COMMANDS, "START");

        g_return_if_fail (action && GTK_IS_ACTION(action));
        g_return_if_fail (window && GL_IS_WINDOW(window));

        if (about != NULL)
        {

                gtk_window_present (GTK_WINDOW (about));
                gtk_window_set_transient_for (GTK_WINDOW (about),
                                              GTK_WINDOW (window));

        } else {
        
                pixbuf = gdk_pixbuf_new_from_file (GLABELS_PIXMAP_DIR "glabels-splash.png", NULL);

                about = gtk_about_dialog_new ();
                gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG(about), _("glabels"));
                gtk_about_dialog_set_version      (GTK_ABOUT_DIALOG(about), VERSION);
                gtk_about_dialog_set_copyright    (GTK_ABOUT_DIALOG(about), copy_text);
                gtk_about_dialog_set_comments     (GTK_ABOUT_DIALOG(about), about_text);
                gtk_about_dialog_set_website      (GTK_ABOUT_DIALOG(about), url);
                gtk_about_dialog_set_logo         (GTK_ABOUT_DIALOG(about), pixbuf);

                gtk_about_dialog_set_authors      (GTK_ABOUT_DIALOG(about), authors);
                gtk_about_dialog_set_artists      (GTK_ABOUT_DIALOG(about), artists);
                gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG(about),
                                                         translator_credits);
                gtk_about_dialog_set_license   (GTK_ABOUT_DIALOG(about), license);
        
                gtk_window_set_destroy_with_parent (GTK_WINDOW (about), TRUE);

                g_signal_connect (G_OBJECT (about), "response",
                                  G_CALLBACK (gtk_widget_destroy), NULL);
                g_signal_connect (G_OBJECT (about), "destroy",
                                  G_CALLBACK (gtk_widget_destroyed), &about);

                gtk_window_set_transient_for (GTK_WINDOW (about),
                                              GTK_WINDOW (window));

                gtk_window_present (GTK_WINDOW (about));

                if (pixbuf != NULL)
                        g_object_unref (pixbuf);

        }
        
        gl_debug (DEBUG_COMMANDS, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
