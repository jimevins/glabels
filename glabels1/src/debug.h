/* Macros/functions for debugging gLabels */
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "glib.h"

#ifdef __GNUC__

#define DEBUG(args...) { \
                         gchar *__string__; \
                         __string__ = g_strdup_printf( args ); \
                         g_message( "%s:%d (%s): %s\n", \
                         __FILE__, __LINE__, __FUNCTION__, __string__ ); \
                         g_free( __string__ ); \
                         __string__ = NULL; \
                       }

#define WARN(args...) { \
                         gchar *__string__; \
                         __string__ = g_strdup_printf( args ); \
                         g_warning( "%s:%d (%s): %s\n", \
                         __FILE__, __LINE__, __FUNCTION__, __string__ ); \
                         g_free( __string__ ); \
                         __string__ = NULL; \
                       }

#define ERROR(args...) { \
                         gchar *__string__; \
                         __string__ = g_strdup_printf( args ); \
                         g_error( "%s:%d (%s): %s\n", \
                         __FILE__, __LINE__, __FUNCTION__, __string__ ); \
                         g_free( __string__ ); \
                         __string__ = NULL; \
                       }

#else				/* !__GNUC__ */

static
DEBUG (const gchar * format,
       ...)
{
	va_list args;
	va_start (args, format);
	g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, format, args);
	va_end (args);
}

static
WARN (const gchar * format,
      ...)
{
	va_list args;
	va_start (args, format);
	g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, format, args);
	va_end (args);
}

static
ERROR (const gchar * format,
       ...)
{
	va_list args;
	va_start (args, format);
	g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, format, args);
	va_end (args);
}

#endif				/* !__GNUC__ */

#endif				/* __DEBUG_H__ */
