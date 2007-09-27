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
 * Initialize all libglabels subsystems.  This function reads in all files
 * containing paper definitions, category definitions, and templates to build
 * its template data base. This function will search both system and user template
 * directories for these files.
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
