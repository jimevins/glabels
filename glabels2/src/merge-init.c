/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge.c:  document merge initialization
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

#include "merge-init.h"
#include "merge-text.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/*****************************************************************************/
/* Initailize document merge module, by registering all available backends.  */
/*****************************************************************************/
void
gl_merge_init (void)
{

	gl_merge_register_backend (GL_TYPE_MERGE_TEXT,
				   "Text/Comma",
				   _("Text file with comma delimeters (CSV)"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", ',',
				   NULL);

	gl_merge_register_backend (GL_TYPE_MERGE_TEXT,
				   "Text/Colon",
				   _("Text file with colon delimeters"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", ',',
				   NULL);

	gl_merge_register_backend (GL_TYPE_MERGE_TEXT,
				   "Text/Tab",
				   _("Text file with tab delimeters (CSV)"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", ',',
				   NULL);

}
