/*
 *  template-history.c
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#include "template-history.h"


#define MAX_TEMPLATES 5


/*========================================================*/
/* Public globals.                                        */
/*========================================================*/

glTemplateHistoryModel *gl_template_history = NULL;


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
/* Initialize template history.                                                  */
/*****************************************************************************/
void
gl_template_history_init (void)
{
        gl_template_history = gl_template_history_model_new (MAX_TEMPLATES);
}


/*****************************************************************************/
/* Initialize template history with null model.                              */
/*****************************************************************************/
void
gl_template_history_init_null (void)
{
        gl_template_history = gl_template_history_model_new_null ();
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
