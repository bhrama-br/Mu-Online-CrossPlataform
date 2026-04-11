/* jconfig.h -- Android/ARM configuration for IJG JPEG library */

#define HAVE_PROTOTYPES
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
#define HAVE_STDDEF_H
#define HAVE_STDLIB_H
#define HAVE_LOCALE_H

#ifdef JPEG_INTERNALS
#define INLINE __inline__
#endif

#ifdef JPEG_CJPEG_DJPEG
#define BMP_SUPPORTED
#define PPM_SUPPORTED
#endif
