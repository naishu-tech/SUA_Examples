#ifndef IVI_VERSION_H
#define IVI_VERSION_H

#define PROJECT_NAME                             "IVI"


#define IVI_PROJECT_VER                       "1.1.1"
#define IVI_PROJECT_VER_MAJOR                 1
#define IVI_PROJECT_VER_MINOR                 1
#define IVI_PROJECT_VER_PATCH                 1
#define IVI_PROJECT_VER_CODE                  (((IVI_PROJECT_VER_MAJOR) << 16) + \
                                                   ((IVI_PROJECT_VER_MINOR) << 8) + \
                                                   (IVI_PROJECT_VER_PATCH))

#define IVI_GIT_TAG_VERSION                   "v1.1.1"
#define IVI_GIT_TAG_VERSION_STRING            "1.1.1"

#define IVI_VER_(major, minor, patch)         (((major) << 16) + ((minor) << 8) + (patch))
#define IVI_CURRENT_VERSION_CODE              IVI_VER_(IVI_PROJECT_VER_MAJOR, IVI_PROJECT_VER_MINOR, IVI_PROJECT_VER_PATCH)

#define IVI_BUILD_TIME "20251126_121906"
#define IVI_GIT_INFO "fix/v1.1.2-bug_2b62f11"

/* #undef IVI_DEBUG_EN */
#define IVI_ERROR_EN
#define IVI_WARN_EN
#define IVI_PRINT_EN

#define IVI_WITH_VPS
#define IVI_WITH_XDMA

#endif //IVI_VERSION_H
