#
# Try to find GLFW library and include path.
# Once done this will define
#
# GLFW_FOUND
# GLFW_INCLUDE_PATH
# GLFW_LIBRARY
# 

IF(WIN32)
  FIND_PATH( GLFW_INCLUDE_PATH GL/glfw.h
    $ENV{PROGRAMFILES}/GLFW/include
    ${GLFW_ROOT_DIR}/include
    $ENV{GLFW_ROOT_DIR}/include
    DOC "The directory where GL/glfw.h resides")

    FIND_LIBRARY( GLFW_LIBRARY
        NAMES glfw GLFW glfw32 glfw32s
        PATHS
        $ENV{PROGRAMFILES}/GLFW/lib
        ${GLFW_ROOT_DIR}/lib
        $ENV{GLFW_ROOT_DIR}/lib
        $ENV{GLFW_ROOT_DIR}/lib-msvc110
        DOC "The GLFW library")
ELSE(WIN32)
    FIND_PATH( GLFW_INCLUDE_PATH GL/glfw.h
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        $ENV{GLFW_ROOT_DIR}/include
        ${GLFW_ROOT_DIR}/include
        DOC "The directory where GL/glfw.h resides"
    )

  # Prefer the static library.
  FIND_LIBRARY( GLFW_LIBRARY
    NAMES libGLFW.a GLFW
    PATHS
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    $ENV{GLFW_ROOT_DIR}/lib
    ${GLFW_ROOT_DIR}/lib
    DOC "The GLFW library")
ENDIF(WIN32)

SET(GLFW_FOUND "NO")
IF(GLFW_INCLUDE_PATH AND GLFW_LIBRARY)
  SET(GLFW_LIBRARIES ${GLFW_LIBRARY})
  SET(GLFW_INCLUDE_DIR ${GLFW_INCLUDE_PATH})
  SET(GLFW_FOUND "YES")
    message(STATUS "Found GLFW.")
ENDIF(GLFW_INCLUDE_PATH AND GLFW_LIBRARY)
