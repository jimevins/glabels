/*
 *  merge-init.c
 *  Copyright (C) 2002-2009  Jim Evins <evins@snaught.com>.
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
#include "merge-text.h"

#ifdef HAVE_LIBEBOOK
#include "merge-evolution.h"
#include "merge-vcard.h"
#endif /* HAVE_LIBEBOOK */

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
				   _("Text: Comma Separated Values (CSV)"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", ',',
				   NULL);

	gl_merge_register_backend (GL_TYPE_MERGE_TEXT,
				   "Text/Comma/Line1Keys",
				   _("Text: Comma Separated Values (CSV) with keys on line 1"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", ',',
                                   "line1_has_keys", TRUE,
				   NULL);

	gl_merge_register_backend (GL_TYPE_MERGE_TEXT,
				   "Text/Tab",
				   _("Text: Tab Separated Values (TSV)"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", '\t',
				   NULL);

	gl_merge_register_backend (GL_TYPE_MERGE_TEXT,
				   "Text/Tab/Line1Keys",
				   _("Text: Tab Separated Values (TSV) with keys on line 1"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", '\t',
                                   "line1_has_keys", TRUE,
				   NULL);

	gl_merge_register_backend (GL_TYPE_MERGE_TEXT,
				   "Text/Colon",
				   _("Text: Colon separated values"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", ':',
				   NULL);

	gl_merge_register_backend (GL_TYPE_MERGE_TEXT,
				   "Text/Colon/Line1Keys",
				   _("Text: Colon separated values with keys on line 1"),
				   GL_MERGE_SRC_IS_FILE,
				   "delim", ':',
                                   "line1_has_keys", TRUE,
				   NULL);

#ifdef HAVE_LIBEBOOK

	gl_merge_register_backend (GL_TYPE_MERGE_EVOLUTION,
				   "ebook/eds",
				   _("Evolution Addressbook"),
				   GL_MERGE_SRC_IS_FIXED,
				   NULL);

	gl_merge_register_backend (GL_TYPE_MERGE_VCARD,
				   "ebook/vcard",
				   _("VCards"),
				   GL_MERGE_SRC_IS_FILE,
				   NULL);

#endif /* HAVE_LIBEBOOK */

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
