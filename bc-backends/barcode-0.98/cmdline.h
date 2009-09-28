/*
 * cmdline.h -- generic commandline editing (uses getopt, only short)
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
 */

/*
 * The options may have different arguments of different type
 */
enum  option_type {
    CMDLINE_NONE=0,  /* no argument after option */
    CMDLINE_I,       /* integer (any base)       */
    CMDLINE_D,       /* decimal integer          */
    CMDLINE_X,       /* hex integer              */
    CMDLINE_O,       /* octal integer            */
    CMDLINE_S,       /* string                   */
    CMDLINE_F,       /* double float             */
    CMDLINE_P,       /* pointer                  */
};


struct commandline {
    int option;         /* Single byte: option id */
    int type;           /* Type of argument, used in sscanf */
    void *result;       /* Store data here, if non null */
    int (*fun)(void *); /* Call if defined: arg is "result" or input string */
    char *env;          /* Where to get runtime defaults, may be NULL */
    char *default_v;    /* The compile-time default, may be NULL */
    char *descrip;      /* For err msg. May have %s's for default and env */
};

/* returns: 0 or -1. "optarg" is global (see getopt) */
extern int commandline(struct commandline *args,
		       int argc, char **argv, char *errorhead);

/* prints an error message based on "args" */
extern int commandline_errormsg(FILE *f, struct commandline *args,
				char *prgname, char *messagehead);




