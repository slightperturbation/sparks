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
  if( MSVC )
    # warning C4530: C++ exception handler used, but unwind semantics are not 
    # enabled. Specify /EHsc 
    # MSVC needs /EHsc flag, which I included in root CMakeLists.txt, but 
    # above SET(CMAKE_CXX_FLAGS "") erases it.
    SET(CMAKE_CXX_FLAGS "/EHsc")  
  endif( MSVC )
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
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON) 
  set(Boost_USE_STATIC_RUNTIME ON)
endif( WINDOWS )

list(APPEND PROJECT_BOOST_COMPONENTS system filesystem thread program_options unit_test_framework ) 
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
IF( OPENGL_FOUND AND APPLE )
    FIND_LIBRARY( COCOA_LIBRARY Cocoa )
    FIND_LIBRARY( IOKIT_LIBRARY IOKit )
ENDIF()

## DEVIL -- image loading library
IF( MSVC )
    MESSAGE( STATUS "-- MSVC ")
    SET( CMAKE_PREFIX_PATH $ENV{SOFMIS_DEVIL} )
    MESSAGE( STATUS "-- CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH} ")
    FIND_PACKAGE( DevIL REQUIRED )
    IF( IL_FOUND )
      MESSAGE( STATUS "-- MSVC -- Think we've found IL ")
      INCLUDE_DIRECTORIES( "${IL_INCLUDE_DIR}/../" )
      # Note addition of ../ in include dir is a hack around 
      # a buggy (on windows) standard cmake file for DevIL.
      # See http://www.cmake.org/pipermail/cmake-developers/2011-August/001960.html
      MESSAGE( STATUS "Found DevIL. (Headers at " ${IL_INCLUDE_DIR}/../ " -- note hack.)")
    ELSE( IL_FOUND )
      MESSAGE( STATUS "Failed to find DevIL ------------------")
    ENDIF( IL_FOUND )
ELSE( MSVC )
  FIND_PACKAGE( DevIL REQUIRED )
  IF( IL_FOUND )
      INCLUDE_DIRECTORIES( ${IL_INCLUDE_DIR} )
      MESSAGE( STATUS "Found DevIL. (Headers at " ${IL_INCLUDE_DIR} ")")
  ELSE( IL_FOUND )
      MESSAGE( STATUS "Failed to find DevIL ------------------")
  ENDIF( IL_FOUND )  
ENDIF( MSVC )  

# AntTweakBar -- on windows, depends on env var: ANT_TWEAK_BAR_ROOT
#FIND_PACKAGE(AntTweakBar REQUIRED)
#IF( ANT_TWEAK_BAR_FOUND )
#  INCLUDE_DIRECTORIES( ${ANT_TWEAK_BAR_INCLUDE_PATH} )
#ELSE( ANT_TWEAK_BAR_FOUND )
#  MESSAGE( STATUS "Failed to find AntTweakBar ----------------------")
#ENDIF( ANT_TWEAK_BAR_FOUND )


set( HDRS
  ./src/Mesh.hpp
  ./src/SoftTestDeclarations.hpp
  ./src/VolumeData.hpp
  ./src/Display.hpp
  ./src/Renderable.hpp
  ./src/Perspective.hpp
  ./src/Utilities.hpp
  ./src/TextureManager.hpp
  ./src/ShaderManager.hpp
  ./src/Viewport.hpp
  ./src/Spark.hpp
  ./src/PointSparkRenderable.hpp
  ./src/LSpark.hpp
  ./src/TexturedSparkRenderable.hpp
  ./src/VelocityFieldInterface.hpp
  ./src/Fluid.hpp
  ./src/SlicedVolume.hpp
  ./src/VertexAttribute.hpp
  ./src/Shader.hpp
  ./src/ShaderUniform.hpp
  ./src/Material.hpp
  ./src/Scene.hpp
  ./src/RenderPass.hpp
  ./src/RenderCommand.hpp
  ./src/FileAssetFinder.hpp
)

set( SRCS
  ./src/Mesh.cpp
  ./src/Display.cpp
  ./src/Viewport.cpp
  ./src/Perspective.cpp
  ./src/Utilities.cpp
  ./src/TextureManager.cpp
  ./src/Spark.cpp
  ./src/PointSparkRenderable.cpp
  ./src/LSpark.cpp
  ./src/TexturedSparkRenderable.cpp
  ./src/Fluid.cpp
  ./src/Scene.cpp
  ./src/RenderPass.cpp
  ./src/RenderCommand.cpp
  ./src/ShaderUniform.cpp
  ./src/FileAssetFinder.cpp
)
set( GUI_SRCS 
  ./src/main.cpp
)

set( UNIT_TEST_SRCS
	./src/tests/UnitTests.cpp
)
source_group( "Unit Tests" FILES ${UNIT_TEST_SRCS} )

set( CPPLOG_HDRS
	./ext/cpplog/cpplog.hpp
	./ext/cpplog/outputdebugstream.hpp
	./ext/cpplog/concurrent_queue.hpp
	./ext/cpplog/scribestream.hpp
)
include_directories( ./ext/cpplog/ )
source_group( "External\\CppLog" FILES ${CPPLOG_HDRS} )

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
  ${HDRS} ${SRCS} ${GUI_SRCS} ${CPPLOG_HDRS}
)
target_link_libraries(${PROJECT_NAME} 
	${Boost_LIBRARIES}
	${GLFW_LIBRARY}
	${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${IL_LIBRARIES}
)
add_executable( ${PROJECT_NAME}_test 
  ${UNIT_TEST_SRCS} ${HDRS} ${SRCS} ${CPPLOG_HDRS}
)
target_link_libraries(${PROJECT_NAME}_test
	${Boost_LIBRARIES}
	${GLFW_LIBRARY}
	${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${IL_LIBRARIES}
)
# For OpenGL on Apple, need to link with Cocoa too
if(APPLE)
	target_link_libraries(${PROJECT_NAME}
		${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
	)
endif(APPLE)



