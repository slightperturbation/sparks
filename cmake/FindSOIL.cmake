#
# Simple OpenGL Image Library
#
# On success, defines:
#  SOIL_FOUND
#  SOIL_INCLUDE_DIR
#  SOIL_LIBRARIES
#
# On failure, defines:
#  SOIL_FOUND = NO

FIND_PATH( SOIL_INCLUDE_PATH SOIL.h 
	$ENV{SOIL_ROOT_DIR}/src
	$ENV{SOIL_ROOT_DIR}
	/usr/include
	/usr/local/include
	/sw/include
	/opt/local/include
	DOC "The directory containing the SOIL header"
)

IF(WIN32)
	FIND_LIBRARY( SOIL_LIBRARY 
		NAMES SOIL SOIL_VC2010_x64
		PATHS
		$ENV{SOIL_ROOT_DIR}/lib
		$ENV{SOIL_ROOT_DIR}
		DOC "The SOIL Library"
	)
ELSE(WIN32)
	FIND_LIBRARY( SOIL_LIBRARY 
		NAMES SOIL libSOIL.a
		PATHS
		$ENV{SOIL_ROOT_DIR}/lib
		$ENV{SOIL_ROOT_DIR}
		/usr/lib
		/usr/local/lib
		/opt/local/lib
		DOC "The SOIL Library"
	)
ENDIF(WIN32) 

SET( SOIL_FOUND "NO" )

IF( SOIL_INCLUDE_PATH AND SOIL_LIBRARY )
	SET( SOIL_LIBRARIES ${SOIL_LIBRARY} )
	SET( SOIL_INCLUDE_DIR ${SOIL_INCLUDE_PATH} )
	SET( SOIL_FOUND "YES" )
	MESSAGE( STATUS "Found SOIL." )
ENDIF( SOIL_INCLUDE_PATH AND SOIL_LIBRARY )
