/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  init.c:  initialization module
 *
 *  Copyright (C) 2003, 2004  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of the LIBGLABELS library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA
 */
#include <config.h>

#include "init.h"

#include "paper.h"
#include "category.h"
#include "template.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/



/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

/**
 * lgl_init:
 *
 * Initialize all libglabels subsystems.  It is not necessary for an application to call
 * lgl_init(), because libglabels will initialize on demand.  An application programmer may
 * choose to call lgl_init() at startup to minimize the impact of the first libglabels call
 * on GUI response time.
 *
 * This function initializes its paper definitions, category definitions, and its template
 * database.. It will search both system and user template directories to locate
 * this data.
 */
void
lgl_init (void)
{
	static gboolean initialized = FALSE;

	if (!initialized)
	{

		lgl_paper_init();
		lgl_category_init();
		lgl_template_init();

		initialized = TRUE;
	}
}
