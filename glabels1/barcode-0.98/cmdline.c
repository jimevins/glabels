/*
 * cmdline.c -- parse a command line using getopt, environment and defaults
 *
 * Copyright (c) 1999 Alessandro Rubini (rubini@gnu.org)
 * Copyright (c) 1999 Prosa Srl. (prosa@prosa.it)
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Changes:
 *      Michele Comitini (mcm@glisco.it): better handling of numeric type args.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NO_GETOPT
  /* use replacement */
  #include "compat/getopt.h"
#else
  #include <getopt.h>
#endif

#include "cmdline.h"

/* In case of error, print help string and return error (-1) */
int commandline_errormsg(FILE *f, struct commandline *args,
				       char *prgname, char *messagehead)
{
    char *buffer;
    char *buffer2;
    int len;
    struct commandline *ptr;

    if (!messagehead) {
	fprintf(f,"%s: Error in cmdline\nPossible options:\n",prgname);
    } else {
	fprintf(f,"%s: %s",prgname,messagehead);
    }

    for (ptr = args; ptr->option; ptr++) {
	if (ptr->descrip) {
	    len = strlen(ptr->descrip);
	    len +=  ptr->default_v ? strlen(ptr->default_v) : 6;
	    len += ptr->env ? strlen(ptr->env) : 6;
	    buffer2 = malloc(len+3); /* 3 to be safe :) */
	    len += 32;
	    if (buffer2) {
		buffer = malloc(len);
		if (buffer) {
		    sprintf(buffer2, ptr->descrip,
			    ptr->default_v ? ptr->default_v : "(none)",
			    ptr->env ? ptr->env : "(none)");
		    sprintf(buffer, "   -%c %s     %s", ptr->option,
			    ptr->type ? "<arg>" : "     ", buffer2);
		    fprintf(f, "%s\n",buffer);
		    free(buffer);
		}
		free(buffer2);
	    }
	}
    }
    return -1;
}

/*
 * Associate formats to type.
 */
struct type_fmt {
    enum option_type type;
    char *fmt;
};
static struct type_fmt formats[] = {
    {CMDLINE_I, "%i%s"},   /* the trailing "%s" is used for error check */
    {CMDLINE_D, "%d%s"},
    {CMDLINE_X, "%x%s"},
    {CMDLINE_O, "%o%s"},
    {CMDLINE_F, "%lf%s"},
    {CMDLINE_P, "%p%s"},
    {0, NULL}  /* If we get here, it's CMDLINE_S: no sscanf buf strcpy() */
};


/* Parse one argument (default or provided) */
static int commandline_oneopt(struct commandline *ptr, char *value)
{
    struct type_fmt *tptr;
    char *trash = value ? strdup(value) : NULL;

    /* convert it, if needed */
    if ((ptr->type != CMDLINE_NONE) && ptr->result) {
	for (tptr = formats; tptr->type; tptr++)
	    if (tptr->type == ptr->type)
		break;
	if (!tptr->type) /* not found: it is a string */
	    *(char **)(ptr->result) = value;
	else
	    if (sscanf(value, tptr->fmt, ptr->result, trash)!=1) {
		free(trash);
		return -1;
	    }
    }
    if (trash)
	free(trash);
    
    if ((ptr->type == CMDLINE_NONE) && ptr->result) /* no type, just count */
	(*(int *)(ptr->result))++;

    /* call the function, if needed */
    if (ptr->fun) {
	if (ptr->type) {
	    if (ptr->result) return ptr->fun(ptr->result); /* converted */
	    return ptr->fun(value); /* type, but not converted */
	}
	return ptr->fun(NULL); /* no type: just call it */
    }
    return 0;
}

/* The main function */
int commandline(struct commandline *args, int argc, char **argv,
		char *errorhead)
{
    struct commandline *ptr;
    char *getopt_desc = (char *)calloc(512, 1);
    int desc_offset = 0;
    int opt, retval;
    char *value;

    /* Build getopt string and process defaults values */
    for (ptr = args; ptr->option; ptr++) {
	getopt_desc[desc_offset++] = ptr->option;
	if (ptr->type) getopt_desc[desc_offset++] = ':';

	value = NULL;
	if (ptr->env)
	    value = getenv(ptr->env);
	if (!value)
	    value = ptr->default_v;
	if (value && (retval = commandline_oneopt(ptr, value))) {
	    /*
	     * if the function returns a specific (not -1) value, it already
	     * printed its message, so avoid the generic help
	     */
	    if (retval == -1)
		commandline_errormsg(stderr, args, argv[0], errorhead);
	    return retval;
	}
    }

    /* Run getopt and process material */
    while ((opt = getopt(argc, argv, getopt_desc)) != -1) {
	for (ptr = args; ptr->option; ptr++)
	    if (opt == ptr->option)
		break;
	if (!ptr->option) /* unknown option */
	    return commandline_errormsg(stderr, args, argv[0], errorhead);
	if ( (retval = commandline_oneopt(ptr, optarg)) ) { /*  wrong arg */
	    if (retval == -1)
		commandline_errormsg(stderr, args, argv[0], errorhead);
	    return retval;
	}
    }
    return 0;
}
