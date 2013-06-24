#ifndef SPARK_LUA_INTERPRETER_HPP
#define SPARK_LUA_INTERPRETER_HPP

#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "SparkFacade.hpp"

#include "lua.hpp"
#include "luabind/luabind.hpp"
#include "luabind/function.hpp"
#include "luabind/class.hpp"

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
    // Populate the error message with full-stack info.
    int reportLuaError( lua_State* L )
    {
        std::string err = lua_tostring( L, -1 );
        lua_Debug d;
        int level = 0;
        std::stringstream msg;
        while( lua_getstack( L, level, &d ) )
        {
            if( lua_getinfo( L, "Sln", &d ) )
            {
                msg << "{" << level << "} ";
                msg << d.short_src << ":" << d.currentline;
                if( d.name != 0 )
                {
                    msg << "(" << d.namewhat << " " << d.name << ")";
                }
                msg << " --> " << err << "\n";
            }
            level++;
        }
        std::cerr << "Lua Error: " << msg.str() << "\n";
        lua_pop( L, 1 );
        lua_pushstring( L, msg.str().c_str() );
        return 0;
    }
    
    void bindSparkFacade( lua_State* lua )
    {
        luabind::module( lua )
        [
         luabind::class_< SparkFacade,
                          SparkFacadePtr >( "SparkFacade" )
         ];
    }
    
    void bindTextureManager( lua_State* lua )
    {
        luabind::module( lua )
        [
         luabind::class_< TextureManager, 
                          TextureManagerPtr >( "TextureManager" )
         .def( "loadTextureFromImageFile", &TextureManager::loadTextureFromImageFile )
         .def( "loadCheckerTexture", &TextureManager::loadCheckerTexture )
         .def( "loadTestTexture", &TextureManager::loadTestTexture )
         
         .def( "releaseAll", &TextureManager::releaseAll )
         .def( "deleteTexture", &TextureManager::deleteTexture )
         .def( "exists", &TextureManager::exists )
         .def( "logTextures", &TextureManager::logTextures )
         ];
    }
    void bindShaderManager( lua_State* lua )
    {
        luabind::module( lua )
        [
         luabind::class_< ShaderManager, 
                          ShaderManagerPtr >( "ShaderManager" )
         .def( "loadShaderFromFiles", &ShaderManager::loadShaderFromFiles)
         .def( "reloadShader", &ShaderManager::reloadShader )
         .def( "reloadAllShaders", &ShaderManager::reloadAllShaders )
         .def( "releaseAll", &ShaderManager::releaseAll )
        ];
    }
    
    /// Responsible for handling calls to and errors from the lua interpreter.
    class LuaInterpreter
    {
    public:
        LuaInterpreter( FileAssetFinderPtr finder )
        : m_finder( finder )
        {
            m_lua = luaL_newstate();
            // Load Lua standard libraries, e.g. print function
            luaL_openlibs( m_lua );
            // Connect LuaBind to this lua state
            luabind::open( m_lua );
            // Setup error class
            luabind::set_pcall_callback( reportLuaError );
            // Bind classes
            try
            {
                bindSparkFacade( m_lua );
                bindTextureManager( m_lua );
                bindShaderManager( m_lua );
            }
            catch( luabind::error& err )
            {
                LOG_ERROR(g_log) << "Error in Lua bindings: " << err.what();
            }
        }
        ~LuaInterpreter()
        {
            lua_close( m_lua );
        }

        /// Set the asset finder, giving a set of paths for texture files.
        void setAssetFinder( FileAssetFinderPtr finder ) { m_finder = finder; }
        FileAssetFinderPtr assetFinder( void ) { return m_finder; }
        
        void setFacade( SparkFacadePtr facade )
        {
            luabind::globals( m_lua )["spark"] = facade;
        }
        void setTextureManager( TextureManagerPtr tm )
        {
            luabind::globals( m_lua )["textureManager"] = tm;
        }
        void setShaderManager( ShaderManagerPtr sm )
        {
            luabind::globals( m_lua )["shaderManager"] = sm;
        }

        void runScriptFromString( const std::string& script )
        {
            // Load chunk
            int luaError = luaL_loadstring( m_lua, script.c_str() );
            if( !luaError )
            {
                callChunkOnStack();
            }
            else
            {
                LOG_ERROR(g_log) << "Unable to load string: " << script;
            } 
        }

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
                LOG_ERROR(g_log) << "Unable to load lua script from file \""
                                 << filePath << "\":  "
                                 << errType;
            }
            else
            {
                callChunkOnStack();
            }
            return;
        }

        void runScriptFromFile_NoErrorStack( const char* aScriptFilename )
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
        int callChunkOnStack( void )
        {
            // Instance chunk on stack
            // Add error-reporting function
            int nargs = 0;
            int nresults = 0; // LUA_MULTRET
            int en = 0;
            int top = lua_gettop( m_lua );
            int base = top - nargs;
            lua_pushcfunction( m_lua, reportLuaError );
            lua_insert( m_lua, base );  // push pcall_callback under chunk and args
            en = base;
            int error = lua_pcall( m_lua, nargs, nresults, en);
            if( error )
            {
                // Copy error, but leave on stack for caller
                std::string err( lua_tostring( m_lua, -1 ) );
                LOG_ERROR(g_log) << "Lua compilation error: "
                                 << err;
            }
            if ( en )
            {
                lua_remove( m_lua, en );  // remove pcall_callback
            }
            //int nresults = lua_gettop( m_lua ) - top;
            return error;
        }

        FileAssetFinderPtr m_finder;
        lua_State* m_lua;
    };
}


#endif
