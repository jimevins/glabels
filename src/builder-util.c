/*
 *  builder-util.c
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

#include "builder-util.h"


/****************************************************************************/
/* Get widgets from GtkBuilder "en masse."                                  */
/****************************************************************************/
void gl_builder_util_get_widgets (GtkBuilder *builder,
                                  gchar      *first_name,
                                  ...)
{
        va_list     args;
        gchar      *name;
        GtkWidget **p_widget;

        va_start (args, first_name);

        for ( name = first_name; name; name = va_arg (args, gchar *) )
        {
                p_widget = va_arg (args, GtkWidget **);

                *p_widget = GTK_WIDGET (gtk_builder_get_object (builder, name));

                if (!*p_widget)
                {
                        g_critical ("Could not load widget \"%s\".\n\ngLabels may not be installed correctly!",
                                    name);
                        break;
                }
        }

        va_end (args);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
