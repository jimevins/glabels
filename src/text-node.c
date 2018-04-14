/*
 *  text-node.c
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

#include "text-node.h"

#include <string.h>

#include "merge.h"

#include "debug.h"


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static glTextNode *extract_text_node  (const gchar         *text,
				       gint                *n);

static gboolean    is_empty_field     (const glTextNode    *text_node,
				       const glMergeRecord *record);


/****************************************************************************/
/* Expand single node into representative string.                           */
/****************************************************************************/
gchar *
gl_text_node_expand (const glTextNode    *text_node,
		     const glMergeRecord *record)
{
	gchar *text;

	if (text_node->field_flag) {
		if (record == NULL) {
			return g_strdup_printf ("${%s}", text_node->data);
		} else {
			text = gl_merge_eval_key (record, text_node->data);
			if (text != NULL) {
				return text;
			} else {
				return g_strdup_printf ("%s", "");
			}
		}
	} else {
		return g_strdup (text_node->data);
	}
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Is node a field that evaluates empty?                          */
/*--------------------------------------------------------------------------*/
static gboolean
is_empty_field (const glTextNode    *text_node,
		const glMergeRecord *record)
{
	gchar    *text;
	gboolean  ret = FALSE;

	if ( (record != NULL) && text_node->field_flag) {
		text = gl_merge_eval_key (record, text_node->data);
		if ( (text == NULL) || (text[0] == 0) ) {
			ret = TRUE;
		}
		g_free (text);
	}

	return ret;
}


/****************************************************************************/
/* Create a single text node from given text.                               */
/****************************************************************************/
glTextNode *
gl_text_node_new_from_text (const gchar *text)
{
	gint n;

	return extract_text_node (text, &n);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create a single text node from given text. n = characters used */
/*--------------------------------------------------------------------------*/
static glTextNode *
extract_text_node (const gchar *text,
		   gint        *n)
{
	glTextNode *text_node;
	gchar      *p;
	gint        m;

	text_node = g_new0 (glTextNode, 1);

	if (strncmp (text, "${", strlen ("${")) == 0) {
		/* We are at the beginning of a "FIELD" node */
		text_node->field_flag = TRUE;
		*n = strlen ("${");
		text += *n;
		for (p = (gchar *)text, m = 0; *p != 0; p++, m++, (*n)++) {
			if (*p == '}') {
				(*n)++;
				break;
			}
		}
		text_node->data = g_strndup (text, m);
	} else {
		/* We are at the beginning of a literal node */
		text_node->field_flag = FALSE;
		for (p = (gchar *)text, *n = 0; *p != 0; p++, (*n)++) {
			if (strncmp (p, "${", strlen ("${")) == 0)
				break;
			if (*p == '\n')
				break;
		}
		text_node->data = g_strndup (text, *n);
	}

	return text_node;
}


/****************************************************************************/
/* Copy a single text node.                                                 */
/****************************************************************************/
glTextNode *
gl_text_node_dup (const glTextNode *src)
{
	glTextNode *dst;

	if ( src == NULL ) return NULL;

	dst = g_new0 (glTextNode, 1);

	dst->field_flag = src->field_flag;
	dst->data = g_strdup (src->data);

	return dst;
}


/****************************************************************************/
/* Free a single text node.                                                 */
/****************************************************************************/
void
gl_text_node_free (glTextNode **text_node)
{
	if ( *text_node == NULL ) return;

	g_free ((*text_node)->data);
	(*text_node)->data = NULL;
	g_free (*text_node);
	*text_node = NULL;
}


/****************************************************************************/
/* Compare 2 text nodes for equality.                                       */
/****************************************************************************/
gboolean
gl_text_node_equal (const glTextNode     *text_node1,
		    const glTextNode     *text_node2)
{
	/* First take care of the case of either or both being NULL. */
	if ( text_node1 == NULL ) {
		return ( text_node2 == NULL );
	} else {
		if ( text_node2 == NULL ) {
			return FALSE;
		}
	}

	/* Bail if field flags differ. */
	if ( text_node1->field_flag != text_node2->field_flag ) {
		return FALSE;
	}

	/* Now take care of the case of either or both data fields being NULL. */
	if ( text_node1->data == NULL ) {
		return ( text_node2->data == NULL );
	} else {
		if ( text_node2->data == NULL ) {
			return FALSE;
		}
	}

	/* Field flags are identical, so now compare the data. */
	return (strcmp (text_node1->data, text_node2->data) == 0);
}


/****************************************************************************/
/* Expand text lines into single string.                                    */
/****************************************************************************/
gchar *
gl_text_node_lines_expand (GList               *lines,
			   const glMergeRecord *record)
{
	GList      *p_line, *p_node;
	glTextNode *text_node;
	gchar       *text, *old_text, *expanded_node;
        gboolean   first_line = TRUE;

	text = g_strdup ("");	/* prime pointer for concatenation */
	for (p_line = lines; p_line != NULL; p_line = p_line->next) {

		/* special case: something like ${ADDRESS2} = "" on line by itself. */ 
		/*               in such circumstances ignore the line completely.  */
		p_node = (GList *)p_line->data;
		if (p_node && p_node->next == NULL) {
			text_node = (glTextNode *) p_node->data;
			if ( is_empty_field (text_node, record) ) {
				continue;
			}
		}

		/* prepend newline if it's not the first line */
                if (!first_line) {
			old_text = text;
			text = g_strconcat (text, "\n", NULL);
			g_free (old_text);
		} else {
			first_line = FALSE;
                }

		/* expand each node */
		for (p_node = (GList *) p_line->data; p_node != NULL;
		     p_node = p_node->next) {
			text_node = (glTextNode *) p_node->data;
			old_text = text;
			expanded_node = gl_text_node_expand (text_node, record);
			text = g_strconcat (text, expanded_node, NULL);
			g_free (old_text);
			g_free (expanded_node);
		}
	}

	return text;
}


/****************************************************************************/
/* Parse a string back into text lines.                                     */
/****************************************************************************/
GList *
gl_text_node_lines_new_from_text (const gchar *text)
{
	GList      *lines, *nodes;
	glTextNode *text_node;
	gchar      *p;
	gint        n;

	lines = NULL;
	nodes = NULL;
	for (p = (gchar *)text; *p != 0; p += n) {
		if (*p != '\n') {
			text_node = extract_text_node (p, &n);
			nodes = g_list_append (nodes, text_node);
		} else {
			n = 1;
			lines = g_list_append (lines, nodes);
			nodes = NULL;
		}
	}
	if (p != text && *(p - 1) != '\n') {
		lines = g_list_append (lines, nodes);
	}

	return lines;
}


/****************************************************************************/
/* Copy a list of text lines.                                               */
/****************************************************************************/
GList *
gl_text_node_lines_dup (GList *src_lines)
{
	GList      *dst_lines=NULL;
	GList      *p_line, *line, *p_node;
	glTextNode *node;

	for (p_line = src_lines; p_line != NULL; p_line = p_line->next)
        {
		line = NULL;
		for (p_node = (GList *) p_line->data; p_node != NULL; p_node = p_node->next)
                {
			node = gl_text_node_dup ((glTextNode *)p_node->data);
			line = g_list_append (line, node);
		}
		dst_lines = g_list_append (dst_lines, line);
	}

	return dst_lines;
}


/****************************************************************************/
/* Free a list of text lines.                                               */
/****************************************************************************/
void
gl_text_node_lines_free (GList **lines)
{
	GList *p_line, *p_node;
        glTextNode     *text_node;

	for (p_line = *lines; p_line != NULL; p_line = p_line->next)
        {
		for (p_node = (GList *) p_line->data; p_node != NULL; p_node = p_node->next)
                {
                        text_node = (glTextNode *)p_node->data;
                        p_node->data = NULL;
			gl_text_node_free ( &text_node );
		}
		g_list_free ((GList *) p_line->data);
		p_line->data = NULL;
	}

	g_list_free (*lines);
	*lines = NULL;
}


/****************************************************************************/
/* For debugging:  descend and print lines list.                            */
/****************************************************************************/
void
gl_text_node_lines_print (GList * lines )
{
	GList *p_line, *p_node;
	glTextNode *text_node;
	gint i_line, i_node;

	for (p_line=lines, i_line=0; p_line != NULL; p_line=p_line->next, i_line++) {
		for (p_node = (GList *) p_line->data, i_node=0; p_node != NULL;
		     p_node = p_node->next, i_node++) {
			text_node = (glTextNode *) p_node->data;
			g_print( "LINE[%d], NODE[%d] = { %d, \"%s\" }\n",
				 i_line, i_node,
				 text_node->field_flag, text_node->data );

		}
	}

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
