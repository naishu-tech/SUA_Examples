#ifndef IVI_VERSION_H
#define IVI_VERSION_H

#define PROJECT_NAME                             "ivi_tests"
#define IVI_PROJECT_VER                       ""
#define IVI_PROJECT_VER_MAJOR                 
#define IVI_PROJECT_VER_MINOR                 
#define IVI_PROJECT_VER_PATCH                 
#define IVI_PROJECT_VER_CODE                  (((IVI_PROJECT_VER_MAJOR) << 16) + \
                                                   ((IVI_PROJECT_VER_MINOR) << 8) + \
                                                   (IVI_PROJECT_VER_PATCH))
#define IVI_VER_(major, minor, patch)         (((major) << 16) + ((minor) << 8) + (patch))

#define IVI_BUILD_TIME ""
#define IVI_GIT_INFO "_"

/* #undef IVI_DEBUG_EN */
/* #undef IVI_ERROR_EN */
/* #undef IVI_WARN_EN */
/* #undef IVI_PRINT_EN */

/* #undef IVI_WITH_VPS */
/* #undef IVI_WITH_XDMA */

#endif //IVI_VERSION_H
