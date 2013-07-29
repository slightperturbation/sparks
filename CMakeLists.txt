cmake_minimum_required(VERSION 2.8.9)
include(ExternalProject)

set(CMAKE_MODULE_PATH "./cmake" "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})


# SANITY CHECK -- what variables are set?
IF(WIN32)
  MESSAGE( STATUS "VARIABLES --- WIN32" )
ENDIF(WIN32)
IF(MSVC)
  MESSAGE( STATUS "VARIABLES --- MSVC" )
ENDIF(MSVC)
IF(UNIX)
  MESSAGE( STATUS "VARIABLES --- UNIX" )
ENDIF(UNIX)
IF(APPLE)
  MESSAGE( STATUS "VARIABLES --- APPLE" )
ENDIF(APPLE)

# Use our included patch executable because windows doesn't come with one.
if( WIN32 )
  set( PATCH_CMD "${CMAKE_SOURCE_DIR}/bin-win32/patch.exe" )
else( WIN32 )
  set( PATCH_CMD /usr/bin/patch )
endif( WIN32 )
MESSAGE( STATUS "Using Patch command: ${PATCH_CMD}")

# Prefer static libraries, so prepend common static library suffixes
# See: http://stackoverflow.com/questions/3762057/cmake-how-to-produce-binaries-as-static-as-possible
IF(WIN32)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ELSE(WIN32)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(WIN32)

# Use C++11 
# Note that on Windows, this requires the Intel C++ Compiler
# which can't be setup or enforced by cmake.
if( APPLE )
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
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
SET( FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIR_ft2build} ${FREETYPE_INCLUDE_DIR_freetype2} )
    # enabled. Specify /EHsc 
    # MSVC needs /EHsc flag, which I included in root CMakeLists.txt, but 
    # above SET(CMAKE_CXX_FLAGS "") erases it.
    SET(CMAKE_CXX_FLAGS "/EHsc")  
  endif( MSVC )
endif( WINDOWS )


set(PROJECT_NAME "sparkGui")
project(${PROJECT_NAME})

set(CMAKE_MODULE_PATH 
  "${CMAKE_SOURCE_DIR}/cmake/" 
  ${CMAKE_MODULE_PATH}
)

#####################################################################
# BOOST
#####################################################################
# On windows, looks for env var: BOOST_ROOT
MESSAGE( STATUS "BOOST ENV{BOOST_ROOT}: " $ENV{BOOST_ROOT})
set(BOOST_MIN_VERSION "1.53")
set( Boost_LIB_DIAGNOSTIC_DEFINITIONS  )
if( UNIX )
  #MESSAGE( STATUS "BOOST: Boost_USE_STATIC_LIBS Boost_USE_STATIC_RUNTIME" )
  #set(Boost_USE_STATIC_LIBS OFF)
  #set(Boost_USE_MULTITHREADED ON) 
  #set(Boost_USE_STATIC_RUNTIME OFF)
endif( UNIX )
if( WIN32 )
  MESSAGE( STATUS "Boost on windows" )
  set(Boost_USE_STATIC_LIBS OFF)
  set(Boost_USE_MULTITHREADED ON) 
  set(Boost_USE_STATIC_RUNTIME OFF)
endif( WIN32 )

list(APPEND PROJECT_BOOST_COMPONENTS 
  date_time
  unit_test_framework 
  filesystem 
  system 
  thread 
  ) 
FIND_PACKAGE(Boost ${BOOST_MIN_VERSION} COMPONENTS ${PROJECT_BOOST_COMPONENTS})
MESSAGE( "Using boost from " ${Boost_LIBRARY_DIRS} )
if( Boost_FOUND )
  if( WIN32 )
    # Define WIN32_LEAN_AND_MEAN here to make sure it's defined before WinSock.h is included
    # Needed for ASIO lib
    add_definitions( -DWIN32_LEAN_AND_MEAN )
  endif( WIN32 )
    add_definitions( -DBOOST_ALL_NO_LIB ) # Don't auto-link in windows
    #############add_definitions( -DBOOST_ALL_DYN_LINK )
  INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )
  ADD_DEFINITIONS( ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} )
  ADD_DEFINITIONS( "-DHAS_BOOST" )
  LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )
else()
  message(SEND_ERROR "Failed to find boost!")
endif()
LIST( APPEND PROJECT_LINK_LIBRARIES
  ${Boost_LIBRARIES}
)


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
# Intel Thread Building Blocks (TBB)
#####################################################################
set( USING_TBB OFF )
if( USING_TBB )
  MESSAGE( STATUS "\tTBB ON")
  if( WIN32 )
    set( TBB_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/ext/tbb" )
    set( TBB_BIN_URL "http://threadingbuildingblocks.org/sites/default/files/software_releases/windows/tbb41_20130314oss_win.zip"
         CACHE STRING "URL pointing to the ZIP archive of TBB binary bundle. (See http://threadingbuildingblocks.org/download#stable-releases )" )
    ExternalProject_Add(
        TBB
        PREFIX ${TBB_PREFIX}
        URL ${TBB_BIN_URL}
        URL_MD5 eb73c00f4b40fb0eba8f631bb311ecc5 # Version 4.1 Update 3
        UPDATE_COMMAND    ""
        BUILD_IN_SOURCE   1
        INSTALL_COMMAND   ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
    )
    set( TBB_LIB_DIR "${TBB_PREFIX}/src/TBB/lib/intel64/vc10" )
    set( TBB_LIBRARIES "${TBB_LIB_DIR}/tbb.lib" )
    set( TBB_INCLUDE_DIRS "${TBB_PREFIX}/src/TBB/include" )
    set( TBB_DLLS "${TBB_PREFIX}/src/TBB/bin/intel64/vc10")
  endif( WIN32 )

  if( APPLE )
    # Use BREW to stay up-to-date
    FIND_PACKAGE( TBB REQUIRED )
    if(NOT TBB_FOUND) 
     MESSAGE( SEND_ERROR "Failed to find TBB." )  
    endif(NOT TBB_FOUND)
    INCLUDE_DIRECTORIES( "${TBB_INCLUDE_DIRS}" )
  endif( APPLE )
  LIST( APPEND PROJECT_LINK_LIBRARIES
    ${TBB_LIBRARIES}
  )
endif( USING_TBB )

#####################################################################
# Freetype-GL
# https://code.google.com/p/freetype-gl/
#####################################################################

set( FREETYPEGL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/ext/FreeTypeGL" )
set( FREETYPEGL_SVN_URL "http://freetype-gl.googlecode.com/svn/trunk/"
     CACHE STRING "SVN Repository for FreeType-GL library." )

if( WIN32 )
  # Fix bugs in REV 226 of FreeTypeGL's build on windows
  # Need to stick this in a temporary batch file because
  # the patch command can easily be too long for widows CMD 
  SET( PATCH_BATCH_FILE ${FREETYPEGL_PREFIX}/freetypegl_patch_wrap.bat )
  SET( FREETYPEGL_PATCH_GUARDFILE ${FREETYPEGL_PREFIX}/freetypegl_patch_guardfile.txt )

  # Batch file uses a guard file to avoid patching multiple times (e.g., for Debug and Release builds)
  FILE( WRITE  ${PATCH_BATCH_FILE} "echo FreeTypeGL patching...\n" )
  FILE( APPEND ${PATCH_BATCH_FILE} "@ECHO OFF\n" )
  FILE( APPEND ${PATCH_BATCH_FILE} ":BEGIN\n" )
  FILE( APPEND ${PATCH_BATCH_FILE} "if exist \"${FREETYPEGL_PATCH_GUARDFILE}\" GOTO HASPATCHED\n" )
  FILE( APPEND ${PATCH_BATCH_FILE} "  \"${PATCH_CMD}\" -d \"${FREETYPEGL_PREFIX}/src/FREETYPEGL/\" -p0 -t -i \"${CMAKE_SOURCE_DIR}/cmake/freetype-gl-patch-rev226.patch\"\n" )
  FILE( APPEND ${PATCH_BATCH_FILE} "  copy /y nul \"${FREETYPEGL_PATCH_GUARDFILE}\"\n" )
  FILE( APPEND ${PATCH_BATCH_FILE} "  echo File patched.\n" )
  FILE( APPEND ${PATCH_BATCH_FILE} ":HASPATCHED\n" )
  FILE( APPEND ${PATCH_BATCH_FILE} "echo Done.\n" )

  set( FREETYPEGL_PATCH_CMD "${PATCH_BATCH_FILE}" )
else( WIN32 )
  # Batch for patch needed on unix-style platforms
  #set( FREETYPEGL_PATCH_CMD "${PATCH_CMD} -d \"${FREETYPEGL_PREFIX}/src/FREETYPEGL/\" -p0 -t -i \"${CMAKE_SOURCE_DIR}/cmake/freetype-gl-patch-rev226.patch\"" )
endif( WIN32 )

ExternalProject_Add(
    FREETYPEGL
    PREFIX ${FREETYPEGL_PREFIX}
    SVN_REPOSITORY ${FREETYPEGL_SVN_URL}
    #SVN_REVISION
    # Force th output paths for the libs to help find them afterwards
    CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
               -Dfreetype-gl_BUILD_DEMOS=OFF
    # Disable install step
    INSTALL_COMMAND ""
    PATCH_COMMAND ${FREETYPEGL_PATCH_CMD}
    # Log all errors 
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)
set( FREETYPEGL_INCLUDE_DIRS "${FREETYPEGL_PREFIX}/src/FREETYPEGL" )
INCLUDE_DIRECTORIES( "${FREETYPEGL_INCLUDE_DIRS}" )

set( FREETYPEGL_LIB_DIR "${FREETYPEGL_PREFIX}/src/FREETYPEGL-build" )
# FreetypeGL compiles to a static library.  Windows uses .lib for both static and dynamic libs.
# note that we can't use find_library() (and so CMAKE_FIND_LIBRARY_SUFFIXES)
# because this is an external project.
if( WIN32 )
  set( FREETYPE_GL_LIB_PREFIX "" )
  set( FREETYPE_GL_LIB_SUFFIX .lib )
else( WIN32 )
  set( FREETYPE_GL_LIB_PREFIX lib )
  set( FREETYPE_GL_LIB_SUFFIX .a )
endif( WIN32 )
set( FREETYPEGL_DEBUG_LIB     ${FREETYPEGL_LIB_DIR}/DebugLibs/${FREETYPE_GL_LIB_PREFIX}freetype-gl${FREETYPE_GL_LIB_SUFFIX} )
set( FREETYPEGL_OPTIMIZED_LIB ${FREETYPEGL_LIB_DIR}/ReleaseLibs/${FREETYPE_GL_LIB_PREFIX}freetype-gl${FREETYPE_GL_LIB_SUFFIX} )

list( APPEND PROJECT_LINK_LIBRARIES      debug ${FREETYPEGL_DEBUG_LIB} )
list( APPEND PROJECT_LINK_LIBRARIES      optimized ${FREETYPEGL_OPTIMIZED_LIB} )

# Add source dependency for EDTAA (required for distance-field rendering)
LIST( APPEND EXT_SRC 
  ./ext/edtaa/edtaa3func.c
)


#####################################################################
# FreeType
#####################################################################
if( WIN32 )
  # freetype2 isn't distributed to build for x64, so a pre-built binary is included in project for windows
  list( APPEND PROJECT_LINK_LIBRARIES      debug ${CMAKE_SOURCE_DIR}/ext/freetype2/freetype250MT_D.lib )
  list( APPEND PROJECT_LINK_LIBRARIES      optimized ${CMAKE_SOURCE_DIR}/ext/freetype2/freetype250MT.lib )
else( WIN32 )
  FIND_PACKAGE( Freetype REQUIRED )
  LIST( APPEND PROJECT_LINK_LIBRARIES ${FREETYPE_LIBRARY} )
endif( WIN32 )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/ext/freetype2/include )

#####################################################################
# Lua and Luabind
#####################################################################
INCLUDE( LuaBind )
LIST( APPEND PROJECT_LINK_LIBRARIES ${LUA_LIBRARIES} )
source_group( "External Source" FILES ${EXT_SRC} )

#####################################################################
# OpenGL 
#####################################################################
FIND_PACKAGE(OpenGL REQUIRED)

#####################################################################
# GLFW 3.01
#####################################################################
set( GLFW3_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/ext/GLFW3" )
set( GLFW3_URL "http://sourceforge.net/projects/glfw/files/glfw/3.0.1/glfw-3.0.1.zip/download")
ExternalProject_Add(
    GLFW3
    PREFIX ${GLFW3_PREFIX}
    URL ${GLFW3_URL}
    # Force th output paths for the libs to help find them afterwards
    CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
               -DGLFW_BUILD_EXAMPLES=OFF
               -DGLFW_BUILD_TESTS=OFF
               -DGLFW_INSTALL=OFF
               -DBUILD_SHARED_LIBS=OFF
               -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON
    # Disable install step
    INSTALL_COMMAND ""
    # Log all errors 
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)
set( GLFW3_INCLUDE_DIRS "${GLFW3_PREFIX}/src/GLFW3/include" )
INCLUDE_DIRECTORIES( "${GLFW3_INCLUDE_DIRS}" )
set( GLFW3_LIBRARY_DIR_PREFIX "${GLFW3_PREFIX}/src/GLFW3-build/src" )
if( WIN32 )
  set( GLFW3_DEBUG_LIBRARY ${GLFW3_LIBRARY_DIR_PREFIX}/DebugLibs/glfw3.lib )
  set( GLFW3_RELEASE_LIBRARY ${GLFW3_LIBRARY_DIR_PREFIX}/ReleaseLibs/glfw3.lib )
else( WIN32 )
  set( GLFW3_DEBUG_LIBRARY ${GLFW3_LIBRARY_DIR_PREFIX}/DebugLibs/libglfw3.a )
  set( GLFW3_RELEASE_LIBRARY ${GLFW3_LIBRARY_DIR_PREFIX}/ReleaseLibs/libglfw3.a )
endif( WIN32 )
list( APPEND PROJECT_LINK_LIBRARIES      debug ${GLFW3_DEBUG_LIBRARY} )
list( APPEND PROJECT_LINK_LIBRARIES      optimized ${GLFW3_RELEASE_LIBRARY} )

set( GLFW_LIBRARY "" )


# GLFW -- on Windows: depends on env var: GLFW_ROOT_DIR
#FIND_PACKAGE(glfw REQUIRED)
#IF( GLFW_FOUND )
#  INCLUDE_DIRECTORIES (${GLFW_INCLUDE_DIR})
#ELSE( GLFW_FOUND )
#  MESSAGE(STATUS "GLFW NOT FOUND --------------------------")
#ENDIF( GLFW_FOUND )

#####################################################################
# GLEW 1.9.0
#####################################################################
# GLEW -- on Windows: depends on env var: GLEW_ROOT_DIR
set( GLEW_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext/glew-1.9.0" )
FIND_PACKAGE(glew REQUIRED)
IF( GLEW_FOUND )
  INCLUDE_DIRECTORIES (${GLEW_INCLUDE_DIR})
  if( WIN32 )
    LIST( APPEND PROJECT_LINK_LIBRARIES debug ${GLEW_DEBUG_LIBRARY} )
    LIST( APPEND PROJECT_LINK_LIBRARIES optimized ${GLEW_RELEASE_LIBRARY} )
  else( WIN32 )
    LIST( APPEND PROJECT_LINK_LIBRARIES ${GLEW_LIBRARY} )
  endif( WIN32 )
ELSE( GLEW_FOUND )
  MESSAGE( STATUS "GLEW NOT FOUND -------------------------")
ENDIF( GLEW_FOUND )


#####################################################################
# GLM
#####################################################################
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

## AssImp -- Asset Importer
FIND_PACKAGE( assimp REQUIRED )
IF( ASSIMP_FOUND )
  MESSAGE( STATUS "Found ASSIMP.  Headers at " ${ASSIMP_INCLUDE_DIR} )
  INCLUDE_DIRECTORIES( ${ASSIMP_INCLUDE_DIR} )
ELSE( ASSIMP_FOUND )
  MESSAGE( STATUS "Failed to find ASSIMP -----------------")
ENDIF( ASSIMP_FOUND )

# AntTweakBar -- on windows, depends on env var: ANT_TWEAK_BAR_ROOT
#FIND_PACKAGE(AntTweakBar REQUIRED)
#IF( ANT_TWEAK_BAR_FOUND )
#  INCLUDE_DIRECTORIES( ${ANT_TWEAK_BAR_INCLUDE_PATH} )
#ELSE( ANT_TWEAK_BAR_FOUND )
#  MESSAGE( STATUS "Failed to find AntTweakBar ----------------------")
#ENDIF( ANT_TWEAK_BAR_FOUND )


LIST( APPEND PROJECT_LINK_LIBRARIES 
  ${GLFW_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${IL_LIBRARIES}
  ${ILU_LIBRARIES}
  ${ASSIMP_LIBRARIES}
)



###########################################################################
set( HDRS
  ./src/ArcBall.hpp
  ./src/DBMSpark.hpp
  ./src/Display.hpp
  ./src/LSpark.hpp
  ./src/LuaInterpreter.hpp
  ./src/Exceptions.hpp
  ./src/EyeTracker.hpp
  ./src/Fluid.hpp
  ./src/FileAssetFinder.hpp
  ./src/GuiEventSubscriber.hpp
  ./src/GuiEventPublisher.hpp
  ./src/FontManager.hpp
  ./src/Material.hpp
  ./src/Mesh.hpp
  ./src/PointSparkRenderable.hpp
  ./src/Projection.hpp
  ./src/RayCastVolume.hpp
  ./src/Renderable.hpp
  ./src/RenderPass.hpp
  ./src/RenderCommand.hpp
  ./src/RenderTarget.hpp
  ./src/Scene.hpp
  ./src/SlicedVolume.hpp
  ./src/ShaderManager.hpp
  ./src/ShaderInstance.hpp
  ./src/ShaderUniform.hpp
  ./src/Simulation.hpp
  ./src/Spark.hpp
  ./src/SparkFacade.hpp
  ./src/Task.hpp
  ./src/TextRenderable.hpp
  ./src/TextureManager.hpp
  ./src/TexturedSparkRenderable.hpp
  ./src/Utilities.hpp
  ./src/Viewport.hpp
  ./src/VolumeData.hpp
  ./src/VertexAttribute.hpp
  ./src/VelocityFieldInterface.hpp
)
include_directories( ./src/ )  # Move headers to ./include/

set( SRCS
  ./src/ArcBall.cpp
  ./src/DBMSpark.cpp
  ./src/Display.cpp
  ./src/LSpark.cpp
  ./src/Exceptions.cpp
  ./src/EyeTracker.cpp
  ./src/Fluid.cpp
  ./src/FileAssetFinder.cpp
  ./src/FontManager.cpp
  ./src/Material.cpp
  ./src/Mesh.cpp
  ./src/PointSparkRenderable.cpp
  ./src/Projection.cpp
  ./src/RayCastVolume.cpp
  ./src/Renderable.cpp
  ./src/RenderCommand.cpp
  ./src/RenderPass.cpp
  ./src/RenderTarget.cpp
  ./src/Scene.cpp
  ./src/SlicedVolume.cpp
  ./src/ShaderUniform.cpp
  ./src/ShaderManager.cpp
  ./src/Simulation.cpp
  ./src/TextRenderable.cpp
  ./src/TexturedSparkRenderable.cpp
  ./src/TextureManager.cpp
  ./src/Utilities.cpp
  ./src/Viewport.cpp
 )
set( GUI_SRCS 
  ./src/main.cpp
)

set( UNIT_TEST_SRCS
	./src/tests/UnitTests.cpp
	./src/tests/RenderTests.cpp
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
SET( g_dataPath "${CMAKE_SOURCE_DIR}/data/" )
configure_file( src/config.hpp.in ${CMAKE_BINARY_DIR}/config/config.hpp )
# The config directory should be added as a include-searchpath
include_directories( ${CMAKE_BINARY_DIR}/config/ )
# Alternatively, could use resources, e.g.:
# SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES RESOURCE "${RESOURCES}")

#####################################################################
# Executables
#####################################################################
add_executable( ${PROJECT_NAME} 
  ${HDRS} ${SRCS} ${GUI_SRCS} ${CPPLOG_HDRS} ${EXT_SRC}
)
#add_dependencies( ${PROJECT_NAME} TBB )
add_dependencies( ${PROJECT_NAME} FREETYPEGL )
add_dependencies( ${PROJECT_NAME} GLFW3 )

target_link_libraries(${PROJECT_NAME} 
	${PROJECT_LINK_LIBRARIES}
)


# For OpenGL on Apple, need to link with Cocoa too
if(APPLE)
	target_link_libraries(${PROJECT_NAME}
		${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
	)
#  target_link_libraries(${PROJECT_NAME}_test
#    ${COCOA_LIBRARY}
#    ${IOKIT_LIBRARY}
#  )
endif(APPLE)



if(WIN32)
  MESSAGE( "Executables will require dependency DLLs to be in the path.  E.g., C:/dev_dlls/" )
endif()