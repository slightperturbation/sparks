#
# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_DIR
# GLEW_LIBRARY
# 
# First look for GLEW_ROOT_DIR
# On windows use environment variable GLEW_ROOT_DIR

SET(GLEW_FOUND "NO")

IF(WIN32)

	FIND_PATH( GLEW_INCLUDE_DIR GL/glew.h
		PATHS
			${GLEW_ROOT_DIR}
			$ENV{GLEW_ROOT_DIR}
		PATH_SUFFIXES
			include
		DOC "The directory where GL/glew.h resides")

    FIND_LIBRARY( GLEW_RELEASE_LIBRARY
        NAMES glew64s  # Only accept the 64-bit, static build 
		PATHS
			${GLEW_ROOT_DIR}
		    $ENV{GLEW_ROOT_DIR}/lib
        PATH_SUFFIXES
			lib 
			libs
			bin
		DOC "The GLEW Release library")
	set( GLEW_LIBRARY ${GLEW_RELEASE_LIBRARY} )

    FIND_LIBRARY( GLEW_DEBUG_LIBRARY
        NAMES glew64sd  # Only accept the 64-bit, static build 
		PATHS
			${GLEW_ROOT_DIR}
		    $ENV{GLEW_ROOT_DIR}/lib
        PATH_SUFFIXES
			lib 
			libs
			bin
        DOC "The GLEW Debug library")

        IF( GLEW_INCLUDE_DIR AND GLEW_RELEASE_LIBRARY AND GLEW_DEBUG_LIBRARY )
			SET(GLEW_FOUND "YES")
			message(STATUS "Found GLEW.")
		ENDIF( GLEW_INCLUDE_DIR AND GLEW_RELEASE_LIBRARY AND GLEW_DEBUG_LIBRARY )

ELSE(WIN32)

	FIND_PATH( GLEW_INCLUDE_DIR GL/glew.h
		/usr/include
		/usr/local/include
		/sw/include
        /opt/local/include
		${GLEW_ROOT_DIR}/include
		$ENV{GLEW_ROOT_DIR}/include
		DOC "The directory where GL/glew.h resides")

	# Prefer the static library.
	FIND_LIBRARY( GLEW_LIBRARY
		NAMES libGLEW.a GLEW
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		${GLEW_ROOT_DIR}/lib
		$ENV{GLEW_ROOT_DIR}/lib
		DOC "The GLEW library")

	IF(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
		SET(GLEW_LIBRARIES ${GLEW_LIBRARY})
		SET(GLEW_FOUND "YES")
	    message(STATUS "Found GLEW.")
	ENDIF(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)

ENDIF(WIN32)

