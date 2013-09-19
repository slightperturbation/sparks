#ifndef SPARK_LUA_INTERPRETER_HPP
#define SPARK_LUA_INTERPRETER_HPP

#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "SceneFacade.hpp"
#include "Input.hpp"

#include "lua.hpp"
#include "luabind/luabind.hpp"
#include "luabind/function.hpp"
#include "luabind/class.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <utility>
#include <sstream>


/// Helper classes to adapt boost::shared_ptrs to/from spark::shared_ptr
namespace {
    template<class SharedPointer> struct Holder {
        SharedPointer p;
        
        Holder(const SharedPointer &p) : p(p) {}
        Holder(const Holder &other) : p(other.p) {}
        Holder(Holder &&other) : p(std::move(other.p)) {}
        
        void operator () (...) const {}
    };
}

template<class T> std::shared_ptr<T> to_std_ptr(const boost::shared_ptr<T> &p) {
    typedef Holder<std::shared_ptr<T>> H;
    if(H *h = boost::get_deleter<H, T>(p)) {
        return h->p;
    } else {
        return std::shared_ptr<T>(p.get(), Holder<boost::shared_ptr<T>>(p));
    }
}

template<class T> boost::shared_ptr<T> to_boost_ptr(const std::shared_ptr<T> &p){
    typedef Holder<boost::shared_ptr<T>> H;
    if(H * h = std::get_deleter<H, T>(p)) {
        return h->p;
    } else {
        return boost::shared_ptr<T>(p.get(), Holder<std::shared_ptr<T>>(p));
    }
}

namespace spark
{
    // Forward declaration of ScriptState class which calls directly to luabind
    class ScriptState;
    
    //// Populate the error message with full-stack info.
    int reportLuaError( lua_State* L );
    void bindSceneFacade( lua_State* lua );
    void bindTextureManager( lua_State* lua );
    void bindShaderManager( lua_State* lua );
    /// Bind to lua the glm library classes (vec3, mat4, etc.)
    void bindGLM( lua_State* lua );
    void bindConstants( lua_State* lua );
    void bindInterpreter( lua_State* lua );

    /////////////////////////////////////////////////////////////////////
    // GLM

    // Pseudo member functions for lua accessors
    // Needed because we can't bind operator[]
    inline float vec2_at( glm::vec2* v, int i ) { return (*v)[i]; }
    inline float vec2_set( glm::vec2* v, int i, float x) { return (*v)[i] = x; }

    inline float vec3_at( glm::vec3* v, int i ) { return (*v)[i]; }
    inline float vec3_set( glm::vec3* v, int i, float x) { return (*v)[i] = x; }

    inline float vec4_at( glm::vec4* v, int i ) { return (*v)[i]; }
    inline float vec4_set( glm::vec4* v, int i, float x) { return (*v)[i] = x; }

    inline glm::vec3& mat3_at( glm::mat3* m, int i ) { return (*m)[i]; }
    inline float& mat3_at( glm::mat3* m, int i, int j ) { return (*m)[i][j]; }
    inline void mat3_set( glm::mat3* m, int i, int j, float x ) { (*m)[i][j] = x; }

    //inline glm::vec4& mat4_at( glm::mat4* m, int i ) { return (*m)[i]; }
    inline float mat4_at( glm::mat4& m, int i, int j ) { return m[i][j]; }
    inline void mat4_set( glm::mat4& m, int i, int j, float x ) { m[i][j] = x; }
    /////////////////////////////////////////////////////////////////////
    
    inline bool isWindows( void )
    {
#ifdef WIN32
        return true;
#else
        return false;
#endif
    }
    inline bool isApple( void )
    {
#ifdef __APPLE__
        return true;
#else
        return false;
#endif
    }
    inline bool isLinux( void )
    {
#ifdef __linux__
        return true;
#else
        return false;
#endif
    }
    
    /////////////////////////////////////////////////////////////////////
    
    /// Responsible for handling calls to and errors from the lua interpreter.
    /// Finder's paths are added to Lua's package.path at the point of
    /// construction, subsequent changes to finder will not affect lua's paths.
    /// However, runScriptFromFile (aka, in lua "interp:load()") *does*
    /// query the finder directly, so will be affected by changes to finder's
    /// paths after the creation of the LuaInterpreter.
    class LuaInterpreter
    {
    public:
        LuaInterpreter( FileAssetFinderPtr finder );
        ~LuaInterpreter();
        /// Set the asset finder, giving a set of paths for texture files.
        void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }
        FileAssetFinderPtr assetFinder( void ) { return m_finder; }
        
        /// Sets the facade and also the textureManager and shaderManager
        /// (to the facade's versions to ensure consistency)
        /// Can override if needed.
        void setFacade( SceneFacadePtr facade )
        {
            luabind::globals( m_lua )["spark"] = facade;
            setTextureManager( facade->getTextureManager() );
            setShaderManager( facade->getShaderManager() );
            setInputManager( facade->getInput() );
        }

        /// Registers a C++-owned object with Lua
        template< typename ObjectT >
        void registerObject( const std::string& luaName, ObjectT object )
        {
            luabind::globals( m_lua )[luaName.c_str()] = object;
        }
        
        /// Execute the given script in the current lua context.
        /// Example:
        /// lua.runScriptFromString( "print('DONE--  at(testVec, 1) = ' .. testVec:at(1) );" );
        void runScriptFromString( const std::string& script )
        {
            // Push std lib debugging function
            lua_getglobal( m_lua, "debug");
            lua_getfield( m_lua, -1,"traceback" );
            // assuming string is one chunk on stack
            int errorFuncStack = -2; 
            lua_remove( m_lua, errorFuncStack );

            // Load chunk
            int luaError = luaL_loadstring( m_lua, script.c_str() );
            if( !luaError )
            {
                callChunkOnStack( errorFuncStack );
            }
            else
            {
                LOG_ERROR(g_log) << "Unable to load string: " << script;
            } 
        }

        /// Find a lua script with aScriptFilename using the current
        /// FileAssetFinder.
        /// Runs the script in the current lua context.
        void runScriptFromFile( const char* aScriptFilename )
        {
            LOG_DEBUG(g_log) << "Searching for lua script file \""
                             << aScriptFilename << "\".";
            std::string filePath;
            if( !m_finder->findFile( aScriptFilename, filePath ) )
            {
                LOG_ERROR(g_log) << "FAILED to find texture file \""
                                 << aScriptFilename << "\" in search paths.";
                assert( false );
                return;
            }
            // Push debugging function
            lua_getglobal( m_lua, "debug");
            lua_getfield( m_lua, -1,"traceback" );
            // assuming load file puts it's one chunk on stack
            int errorFuncStack = -2; 
            lua_remove( m_lua, errorFuncStack );
            // load and compile lua
            int fileLoadError = luaL_loadfile( m_lua, filePath.c_str() );
            if( fileLoadError ) 
            {
                std::string errType( "Unknown error" );
                switch( fileLoadError )
                {
                case LUA_ERRSYNTAX:
                    errType = "Syntax error";
                    break;
                case LUA_ERRMEM:
                    errType = "Memory allocation error";
                    break;
                case LUA_ERRFILE: 
                    errType = "Cannot open/read file";
                    break;
                }
                std::cerr << "Unable to load lua script from file \""
                          << filePath << "\":  "
                          << errType << "\n\n";
                LOG_ERROR(g_log) << "Unable to load lua script from file \""
                                 << filePath << "\":  "
                                 << errType;
            }
            else
            {
                callChunkOnStack( errorFuncStack );
            }
            return;
        }
        friend class ScriptState;
    private:
        void setTextureManager( TextureManagerPtr tm )
        {
            luabind::globals( m_lua )["textureManager"] = tm;
        }
        void setShaderManager( ShaderManagerPtr sm )
        {
            luabind::globals( m_lua )["shaderManager"] = sm;
        }
        void setInputManager( InputPtr input )
        {
            luabind::globals( m_lua )["input"] = input;
        }

        void runScriptFromFile_NoErrorStack( const char* aScriptFilename )
        {
            LOG_DEBUG(g_log) << "Searching for lua script file \""
                             << aScriptFilename << "\".";
            std::string filePath;
            if( !m_finder->findFile( aScriptFilename, filePath ) )
            {
                LOG_ERROR(g_log) << "FAILED to find script file \""
                                 << aScriptFilename << "\" in search paths.";
                assert( false );
                return;
            }
            int status = luaL_dofile( m_lua, filePath.c_str() );
            if (status) {
                //error message is at the top of the stack
                std::string err( lua_tostring( m_lua, -1 ) );
                std::cerr << "Lua error in \"" << filePath << "\":\n\t" << err;
                LOG_ERROR(g_log) << "Lua script error in file \""
                                 << filePath << "\":  "
                                 << err;
                return;
            }
            LOG_INFO(g_log) << "Lua script \"" << filePath
                            << "\" executed successfully.";
        }
    private:
        /// errorFuncStack is the stack index of the error function 
        /// that has already been pushed onto the lua stack (0 if none)
        /// See:  http://www.gamedev.net/topic/600886-lua-error-handling/
        /// and: http://www.gamedev.net/topic/446781-lua-more-detailed-error-information/page__view__findpost__p__3960907
        int callChunkOnStack( int errorFuncStack = 0 )
        {
            int err = 0;
            try
            {
                //err = lua_pcall( m_lua, nargs, nresults, errorFunc );
                err = lua_pcall( m_lua, 0, 0, errorFuncStack );
            }
            catch( std::exception& e )
            {
                std::cerr << "Lua exception caught: " << e.what() << '\n';
                LOG_ERROR(g_log) << "Lua exception caught: " << e.what();
            }
            if( err )
            {
                // Copy error, but leave on stack for caller
                std::string errMsg( lua_tostring( m_lua, -1 ) );
                std::cerr << "Lua runtime error:\n" << errMsg << '\n';
                LOG_ERROR(g_log) << "Lua runtime error: " << errMsg;
                lua_pop( m_lua, 1 ); // pop error off stack
                assert( false );
            }
            if( errorFuncStack )
            {
                lua_remove( m_lua, errorFuncStack ); // pop error func
            }
            return err;
        }

        FileAssetFinderPtr m_finder;
        lua_State* m_lua;
    };
    typedef spark::shared_ptr<LuaInterpreter> LuaInterpreterPtr;



}


#endif
