# - Try to find GLUI
# Once done this will define
#
#  GLUI_FOUND - system has GLUI
#  GLUI_INCLUDES - the GLUI include directory
#  GLUI_LIBRARY - Link these to use GLUI

FIND_LIBRARY (GLUI_LIBRARY NAMES glui
    PATHS
    $ENV{PROGRAMFILES}/GLUI/lib
    ${GLUI_ROOT_DIR}/lib
    $ENV{GLUI_ROOT_DIR}/lib
    /usr/local/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /opt/local/lib
    )

FIND_PATH (GLUI_INCLUDE_DIR GL/glui.h
    ${GLUI_ROOT_DIR}/include
    $ENV{GLUI_ROOT_DIR}/include
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    )

IF(GLUI_INCLUDE_DIR AND GLUI_LIBRARY)
    SET(GLUI_FOUND TRUE)
ENDIF(GLUI_INCLUDE_DIR AND GLUI_LIBRARY)

IF(GLUI_FOUND)
  IF(NOT GLUI_FIND_QUIETLY)
    MESSAGE(STATUS "Found GLUI: ${GLUI_LIBRARIES}")
  ENDIF(NOT GLUI_FIND_QUIETLY)
ELSE(GLUI_FOUND)
  IF(GLUI_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find GLUI")
  ENDIF(GLUI_FIND_REQUIRED)
ENDIF(GLUI_FOUND)


