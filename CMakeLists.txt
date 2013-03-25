cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_MODULE_PATH "./cmake" "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if( APPLE )
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
else( APPLE )
  if( UNIX )
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
  endif( UNIX )
endif( APPLE )


# Bug in some versions of visual studio create duplicate manifest entries
# Avoiding manifests entirely seems to be one fix. (http://www.cmake.org/Bug/print_bug_page.php?bug_id=8386)
if( WINDOWS )
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
endif( WINDOWS )

set(PROJECT_NAME "sparks")
project(${PROJECT_NAME})

set(CMAKE_MODULE_PATH 
  "${CMAKE_SOURCE_DIR}/cmake/" 
  ${CMAKE_MODULE_PATH}
)

#####################################################################
# BOOST
#####################################################################
# On windows, looks for env var: BOOST_ROOT
MESSAGE( "BOOST: " $ENV{BOOST_ROOT})
set(BOOST_MIN_VERSION "1.44")
set( Boost_LIB_DIAGNOSTIC_DEFINITIONS  )
if( WINDOWS )
  #set(Boost_USE_STATIC_LIBS ON)
  #set(Boost_USE_MULTITHREADED ON) 
  #set(Boost_USE_STATIC_RUNTIME ON)
endif( WINDOWS )

list(APPEND PROJECT_BOOST_COMPONENTS system filesystem thread program_options) 
FIND_PACKAGE(Boost ${BOOST_MIN_VERSION} COMPONENTS ${PROJECT_BOOST_COMPONENTS})

if( Boost_FOUND )
  if( WINDOWS )
    #add_definitions( -DBOOST_ALL_NO_LIB )
    #add_definitions( -DBOOST_ALL_DYN_LINK )
  endif( WINDOWS )
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  ADD_DEFINITIONS( ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} )
  ADD_DEFINITIONS( "-DHAS_BOOST" )
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
else()
  message(SEND_ERROR "Failed to find boost!")
endif()


#####################################################################
# Eigen3
#####################################################################
FIND_PATH( EIGEN3_INCLUDE_DIR signature_of_eigen3_matrix_library
          HINTS 
            /usr/local/include/
            /usr/include/
            "C:/libs/"
            "C:/libs/eigen-3.1.2"
          PATH_SUFFIXES eigen3 )
INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}/unsupported/" )

#####################################################################
# GUI Libraries : OpenGL and GLFW (not required)
#####################################################################
FIND_PACKAGE(OpenGL REQUIRED)
#MESSAGE(STATUS "Opengl libraries: ${OPENGL_LIBRARIES}")
#MESSAGE(STATUS "Opengl libraries: ${OPENGL_gl_LIBRARY}")
#MESSAGE(STATUS "Opengl input dir: ${OPENGL_INCLUDE_DIR}")

# GLFW -- on Windows: depends on env var: GLFW_ROOT_DIR
FIND_PACKAGE(glfw REQUIRED)
IF( GLFW_FOUND )
  INCLUDE_DIRECTORIES (${GLFW_INCLUDE_DIR})
ELSE( GLFW_FOUND )
  MESSAGE(STATUS "GLFW NOT FOUND --------------------------")
ENDIF( GLFW_FOUND )

# GLEW -- on Windows: depends on env var: GLEW_ROOT_DIR
FIND_PACKAGE(glew REQUIRED)
IF( GLEW_FOUND )
  INCLUDE_DIRECTORIES (${GLEW_INCLUDE_DIR})
ELSE( GLEW_FOUND )
  MESSAGE( STATUS "GLEW NOT FOUND -------------------------")
ENDIF( GLEW_FOUND )

# GLM -- on Windows: depends on env var: GLM_ROOT_DIR
FIND_PACKAGE(glm REQUIRED)
IF( GLM_FOUND )
  INCLUDE_DIRECTORIES( ${GLM_INCLUDE_DIR} )
ELSE( GLM_FOUND )
  MESSAGE( STATUS "Failed to find GLM ----------------------")
ENDIF( GLM_FOUND )
if(OPENGL_FOUND AND APPLE)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(IOKIT_LIBRARY IOKit)
endif()

# AntTweakBar -- on windows, depends on env var: ANT_TWEAK_BAR_ROOT
#FIND_PACKAGE(AntTweakBar REQUIRED)
#IF( ANT_TWEAK_BAR_FOUND )
#  INCLUDE_DIRECTORIES( ${ANT_TWEAK_BAR_INCLUDE_PATH} )
#ELSE( ANT_TWEAK_BAR_FOUND )
#  MESSAGE( STATUS "Failed to find AntTweakBar ----------------------")
#ENDIF( ANT_TWEAK_BAR_FOUND )


set( HDRS
  ./src/Mesh.hpp
  ./src/VolumeData.hpp
  ./src/Display.hpp
  ./src/Renderable.hpp
  ./src/RenderContext.hpp
  ./src/SoftTestDeclarations.hpp
  ./src/Viewport.hpp
  ./src/Spark.hpp
  ./src/PointSparkRenderable.hpp
  ./src/LSpark.hpp
  ./src/TexturedSparkRenderable.hpp
)

set( SRCS
  ./src/main.cpp
  ./src/Mesh.cpp
  ./src/Display.cpp
  ./src/Viewport.cpp
  ./src/Spark.cpp
  ./src/PointSparkRenderable.cpp
  ./src/LSpark.cpp
  ./src/TexturedSparkRenderable.cpp
)


#####################################################################
# Data dir config
#####################################################################
# Relies on variable name g_dataPath
SET( g_dataPath "${CMAKE_SOURCE_DIR}/data" )
configure_file( src/config.hpp.in ${CMAKE_BINARY_DIR}/config/config.hpp )
# The config directory should be added as a include-searchpath
include_directories( ${CMAKE_BINARY_DIR}/config/ )

#####################################################################
# Executables
#####################################################################
add_executable( ${PROJECT_NAME} 
  ${HDRS} ${SRCS} 
)
target_link_libraries(${PROJECT_NAME} 
	${Boost_LIBRARIES}
	${GLFW_LIBRARY}
	${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
)

# For OpenGL on Apple, need to link with Cocoa too
if(APPLE)
	target_link_libraries(${PROJECT_NAME}
		${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
	)
endif(APPLE)



