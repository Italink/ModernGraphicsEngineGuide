#ifndef RENDERDOC_API_H
#define RENDERDOC_API_H

#include <QtCore/qglobal.h>

#if defined(RENDERDOC_LIBRARY)
#  define RENDERDOC_API Q_DECL_EXPORT
#else
#  define RENDERDOC_API Q_DECL_IMPORT
#endif

#endif // RENDERDOC_API_H