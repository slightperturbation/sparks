#
# Compile-in Lua and Luabind
#
# Will downlad source as needed.
#
########################################################################
# Lua
set( LUA_RELEASE 5.2.2 )
set( LUA_DIR "${CMAKE_SOURCE_DIR}/ext/lua-${LUA_RELEASE}" )
SET( LUA_SRC_DIR "${LUA_DIR}/src/lua/src" )
if( EXISTS ${LUA_SRC_DIR}/lapi.c )
    SET( LUA_SRC
        ${LUA_SRC_DIR}/lapi.c
        ${LUA_SRC_DIR}/lauxlib.c
        ${LUA_SRC_DIR}/lbaselib.c
        ${LUA_SRC_DIR}/lbitlib.c
        ${LUA_SRC_DIR}/lcode.c
        ${LUA_SRC_DIR}/lcorolib.c
        ${LUA_SRC_DIR}/lctype.c
        ${LUA_SRC_DIR}/ldblib.c
        ${LUA_SRC_DIR}/ldebug.c
        ${LUA_SRC_DIR}/ldo.c
        ${LUA_SRC_DIR}/ldump.c
        ${LUA_SRC_DIR}/lfunc.c
        ${LUA_SRC_DIR}/lgc.c
        ${LUA_SRC_DIR}/linit.c
        ${LUA_SRC_DIR}/liolib.c
        ${LUA_SRC_DIR}/llex.c
        ${LUA_SRC_DIR}/lmathlib.c
        ${LUA_SRC_DIR}/lmem.c
        ${LUA_SRC_DIR}/loadlib.c
        ${LUA_SRC_DIR}/lobject.c
        ${LUA_SRC_DIR}/lopcodes.c
        ${LUA_SRC_DIR}/loslib.c
        ${LUA_SRC_DIR}/lparser.c
        ${LUA_SRC_DIR}/lstate.c
        ${LUA_SRC_DIR}/lstring.c
        ${LUA_SRC_DIR}/lstrlib.c
        ${LUA_SRC_DIR}/ltable.c
        ${LUA_SRC_DIR}/ltablib.c
        ${LUA_SRC_DIR}/ltm.c
        # ${LUA_SRC_DIR}/lua.c -- Interpreter
        # ${LUA_SRC_DIR}/luac.c -- Compiler
        ${LUA_SRC_DIR}/lundump.c
        ${LUA_SRC_DIR}/lvm.c
        ${LUA_SRC_DIR}/lzio.c
    )
else()
    ExternalProject_Add( 
        lua
        URL "http://www.lua.org/ftp/lua-${LUA_RELEASE}.tar.gz"
        PREFIX "${LUA_DIR}"
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )
    MESSAGE( "\n\n!!!NOTE: Lua not found.  Build project to download, then re-run cmake.\n\n" )
endif()
# CFLAGS, extracted from lua's src/Makefile
#add_definitions( -DLUA_COMPAT_ALL )
if( WIN32 )
elseif( APPLE )
    #add_definitions( -DLUA_USE_MACOSX )
    set( LUA_LIBRARIES readline )
endif()
SET( LUA_INCLUDE_DIR "${LUA_SRC_DIR}" ) 
INCLUDE_DIRECTORIES( "${LUA_INCLUDE_DIR}" )
########################################################################

########################################################################
# Luabind
set( LUABIND_RELEASE 0.9.1 )
set( LUABIND_DIR "${CMAKE_SOURCE_DIR}/ext/luabind-${LUABIND_RELEASE}" )
SET( LUABIND_SRC_DIR "${LUABIND_DIR}/src/luabind/src")
SET( LUABIND_INCLUDE_DIR "${LUABIND_DIR}/src/luabind/" ) 

# Uncomment to re-download lua source
if( EXISTS ${LUABIND_SRC_DIR}/class.cpp )
    FILE( GLOB LUABIND_SRC "${LUABIND_SRC_DIR}/*.cpp" )
    # SET( LUABIND_SRC
    #     ${LUABIND_SRC_DIR}/class.cpp
    #     ${LUABIND_SRC_DIR}/class_info.cpp
    #     ${LUABIND_SRC_DIR}/class_registry.cpp
    #     ${LUABIND_SRC_DIR}/class_rep.cpp
    #     ${LUABIND_SRC_DIR}/create_class.cpp
    #     ${LUABIND_SRC_DIR}/error.cpp
    #     ${LUABIND_SRC_DIR}/exception_handler.cpp
    #     ${LUABIND_SRC_DIR}/function.cpp
    #     ${LUABIND_SRC_DIR}/inheritance.cpp
    #     ${LUABIND_SRC_DIR}/link_compatibility.cpp
    #     ${LUABIND_SRC_DIR}/object_rep.cpp
    #     ${LUABIND_SRC_DIR}/open.cpp
    #     ${LUABIND_SRC_DIR}/pcall.cpp
    #     ${LUABIND_SRC_DIR}/scope.cpp
    #     ${LUABIND_SRC_DIR}/stack_content_by_name.cpp
    #     ${LUABIND_SRC_DIR}/weak_ref.cpp
    #     ${LUABIND_SRC_DIR}/wrapper_base.cpp
    # )
    INCLUDE_DIRECTORIES( "${LUABIND_INCLUDE_DIR}" )
    message( "Including: ${LUABIND_INCLUDE_DIR}" )
else()
        ExternalProject_Add( 
        luabind
        #GIT_REPOSITORY "git://github.com/rpavlik/luabind.git"
        #GIT_REPOSITORY "git://github.com/Kazade/luabind.git"
        GIT_REPOSITORY "git@bitbucket.org:cinderblocks/luabind.git"
        PREFIX "${LUABIND_DIR}"
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )
    MESSAGE( "\n\n!!!NOTE: Luabind not found.  Build project to download, then re-run cmake.\n\n" )
endif()

########################################################################
# Applications using Lua and Luabind should add EXT_SRC
LIST( APPEND EXT_SRC ${LUA_SRC} ${LUABIND_SRC} )

