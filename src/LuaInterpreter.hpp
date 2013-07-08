#ifndef SPARK_LUA_INTERPRETER_HPP
#define SPARK_LUA_INTERPRETER_HPP

#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "SparkFacade.hpp"

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
    //// Populate the error message with full-stack info.
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
        LOG_ERROR(g_log) << "Lua Error:\n" << msg.str() << "\n";
        // Replace original message with the stack trace
        lua_pop( L, 1 );
        lua_pushstring( L, msg.str().c_str() );
        return 0;
    }
    
    void bindSparkFacade( lua_State* lua )
    {
        luabind::module( lua )
        [
            luabind::class_< Renderable, RenderablePtr >( "Renderable" )
            .def( "setRequireExplicitMaterial", 
                  &Renderable::setRequireExplicitMaterial )
            .def( "requiresExplicitMaterial", 
                  &Renderable::requiresExplicitMaterial )
            .def( "getTransform",
                  &Renderable::getTransform )
            .def( "setTransform",
                  &Renderable::setTransform )
            .def( "applyTransform",
                  &Renderable::transform )
            .def( "translate",
                  (void (Renderable::*)(float,float,float) )
                  &Renderable::translate )
            .def( "translate",
                  (void (Renderable::*)(const glm::vec3&) )
                  &Renderable::translate )
            .def( "scale",
                  (void (Renderable::*)(float))
                  &Renderable::scale )
            .def( "rotate", 
                  &Renderable::rotate )
            .def( "setMaterialForPassName", 
                  &Renderable::setMaterialForPassName ) 
        ];

        luabind::module( lua )
        [
            luabind::class_< RenderPass, RenderPassPtr >( "RenderPass" )
            .def( "name", &RenderPass::name )
            .def( "disableBlending", &RenderPass::disableBlending )
            .def( "useAdditiveBlending", &RenderPass::useAdditiveBlending )
            .def( "useInterpolatedBlending", &RenderPass::useInterpolatedBlending )
            .def( "useMaxBlending", &RenderPass::useMaxBlending )
            .def( "setDepthTest", &RenderPass::setDepthTest )
            .def( "setDepthWrite", &RenderPass::setDepthWrite )
            .def( "setColorWrite", &RenderPass::setColorWrite )
            .def( "priority", &RenderPass::priority )
            .def( "targetSize", &RenderPass::targetSize )
            .def( "targetName", &RenderPass::targetName )
            .def( "useDefaultMaterial",
                  ( void (RenderPass::*)( MaterialPtr ) )
                  &RenderPass::useDefaultMaterial )
        ];

        luabind::module( lua )
        [
            luabind::class_< RenderTarget,
                             RenderTargetPtr >( "RenderTarget" )
            .def( "setClearColor", &RenderTarget::setClearColor )
        ];

        luabind::module( lua )
        [
            luabind::class_< Projection, ProjectionPtr >( "Projection" )
        ];
        luabind::module( lua )
        [
            luabind::class_< PerspectiveProjection, 
                             Projection,
                             PerspectiveProjectionPtr >( "PerspectiveProjection" )
            // modifiers
            .def( "cameraUp", (void (PerspectiveProjection::*)(float,float,float) )&PerspectiveProjection::cameraUp )
            .def( "cameraPos", (void (PerspectiveProjection::*)(float,float,float) )&PerspectiveProjection::cameraPos )
            .def( "cameraTarget", (void (PerspectiveProjection::*)(float,float,float) )&PerspectiveProjection::cameraTarget )
            .def( "fov", (void (PerspectiveProjection::*)(float) )&PerspectiveProjection::fov )

        ];

        luabind::module( lua )
        [
         luabind::class_< SparkFacade,
                          SparkFacadePtr >( "SparkFacade" )
         .def( "createPostProcessingRenderPassAndTarget",
               &SparkFacade::createPostProcessingRenderPassAndTarget )
         .def( "createPostProcessingRenderPassAndScaledTarget", 
               &SparkFacade::createPostProcessingRenderPassAndScaledTarget )
         .def( "createPostProcessingRenderPass", 
               &SparkFacade::createPostProcessingRenderPass )
         .def( "getFrameBufferRenderTarget", 
               &SparkFacade::getFrameBufferRenderTarget )
         .def( "getCamera", 
               &SparkFacade::getCamera )
         .def( "createScaledTextureRenderTarget", 
               &SparkFacade::createScaledTextureRenderTarget )
         .def( "createTextureRenderTarget",
               &SparkFacade::createTextureRenderTarget )
         .def( "createRenderPass",
               &SparkFacade::createRenderPass )
         .def( "createRenderPassWithProjection", 
               &SparkFacade::createRenderPassWithProjection )
         .def( "setMainRenderTarget", 
               &SparkFacade::setMainRenderTarget )
         .def( "getMainRenderTarget", 
               &SparkFacade::getMainRenderTarget )
         .def( "createMaterial", 
               &SparkFacade::createMaterial )
         .def( "loadMesh", 
               &SparkFacade::loadMesh )
         .def( "createCube",
               &SparkFacade::createCube )
         .def( "createLSpark", 
               &SparkFacade::createLSpark )
        ];
 
        luabind::module( lua )
        [
            luabind::class_< Material,
                             MaterialPtr >( "Material" )
//            .property( "name", (std::string& (Material::* const)(void) )&Material::name,
//                               (void (Material::*)(const std::string&) )&Material::name )
            .def( "setFloat", &Material::setShaderUniform<float> )
            .def( "setVec2", &Material::setShaderUniform<glm::vec2> )
            .def( "setVec3", &Material::setShaderUniform<glm::vec3> )
            .def( "setVec4", &Material::setShaderUniform<glm::vec4> )
            .def( "addTexture", &Material::addTexture )
            .def( "dumpShaderUniforms", &Material::dumpShaderUniforms )
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
    /////////////////////////////////////////////////////////////////////
    // GLM

    // Pseudo member functions for lua accessors
    // Needed because we can't bind operator[]
    float vec2_at( glm::vec2* v, int i ) { return (*v)[i]; }
    float vec2_set( glm::vec2* v, int i, float x) { return (*v)[i] = x; }

    float vec3_at( glm::vec3* v, int i ) { return (*v)[i]; }
    float vec3_set( glm::vec3* v, int i, float x) { return (*v)[i] = x; }

    float vec4_at( glm::vec4* v, int i ) { return (*v)[i]; }
    float vec4_set( glm::vec4* v, int i, float x) { return (*v)[i] = x; }

    glm::vec3& mat3_at( glm::mat3* m, int i ) { return (*m)[i]; }
    float& mat3_at( glm::mat3* m, int i, int j ) { return (*m)[i][j]; }
    void mat3_set( glm::mat3* m, int i, int j, float x ) { (*m)[i][j] = x; }

    glm::vec4& mat4_at( glm::mat4* m, int i ) { return (*m)[i]; }
    float& mat4_at( glm::mat4* m, int i, int j ) { return (*m)[i][j]; }
    void mat4_set( glm::mat4* m, int i, int j, float x ) { (*m)[i][j] = x; }
    
    /// Bind to lua the glm library classes (vec3, mat4, etc.)
    void bindGLM( lua_State* lua )
    {
        luabind::module( lua )
        [
            luabind::class_< glm::vec2 >( "vec2" )
            .def( luabind::constructor<>() )
            .def( luabind::constructor<float,float>() )
            .def_readwrite( "x", &glm::vec2::x )
            .def_readwrite( "y", &glm::vec2::y )
            .def_readwrite( "r", &glm::vec2::r )
            .def_readwrite( "g", &glm::vec2::g )
            .def_readwrite( "s", &glm::vec2::s )
            .def_readwrite( "t", &glm::vec2::t )
            .def( "at", &vec2_at )
            .def( "set", &vec2_set )
        ];

        luabind::module( lua )
        [
            luabind::class_< glm::vec3 >( "vec3" )
            .def( luabind::constructor<>() )
            .def( luabind::constructor<float>() )
            .def( luabind::constructor<float,float,float>() )
            .def_readwrite( "x", &glm::vec3::x )
            .def_readwrite( "y", &glm::vec3::y )
            .def_readwrite( "z", &glm::vec3::z )
            .def_readwrite( "r", &glm::vec3::r )
            .def_readwrite( "g", &glm::vec3::g )
            .def_readwrite( "b", &glm::vec3::b )
            .def( "at", &vec3_at )
            .def( "set", &vec3_set )
        ];
        luabind::module( lua )
        [
            luabind::class_< glm::vec4 >( "vec4" )
            .def( luabind::constructor<>() )
            .def( luabind::constructor<float>() )
            .def( luabind::constructor<float,float,float,float>() )
            .def_readwrite( "x", &glm::vec4::x )
            .def_readwrite( "y", &glm::vec4::y )
            .def_readwrite( "z", &glm::vec4::z )
            .def_readwrite( "w", &glm::vec4::w )
            .def_readwrite( "r", &glm::vec4::r )
            .def_readwrite( "g", &glm::vec4::g )
            .def_readwrite( "b", &glm::vec4::b )
            .def_readwrite( "a", &glm::vec4::a )
            .def( "at", &vec4_at )
            .def( "set", &vec4_set )
        ];

        luabind::module( lua )
        [
         luabind::class_< glm::mat3 >( "mat3" )
         .def( luabind::constructor<>() )
         .def( luabind::constructor<float>() )
         .def( luabind::constructor<glm::vec3,glm::vec3,glm::vec3>() )
         .def( "at", (glm::vec3& (*)(glm::mat3*, int))&mat3_at )
         .def( "at", (float& (*)(glm::mat3*, int, int))&mat3_at )
         ];

        luabind::module( lua )
        [
            luabind::class_< glm::mat4 >( "mat4" )
            .def( luabind::constructor<>() )
            .def( luabind::constructor<float>() )
            .def( luabind::constructor<glm::vec4,glm::vec4,glm::vec4,glm::vec4>() )
            .def( "at", (glm::vec4& (*)(glm::mat4*, int))&mat4_at )
            .def( "at", (float& (*)(glm::mat4*, int, int))&mat4_at )
            .def( "set", &mat4_set )
        ];
    }
    
    void bindInterpreter( lua_State* lua );
    
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
                bindGLM( m_lua );
                bindTextureManager( m_lua );
                bindShaderManager( m_lua );
                bindSparkFacade( m_lua );
                bindInterpreter( m_lua );
            }
            catch( luabind::error& err )
            {
                LOG_ERROR(g_log) << "Error in Lua bindings: " << err.what();
            }
            
            luabind::globals( m_lua )["interp"] = this;
            
            /// Add m_finder's paths to lua's package.path for finding
            /// modules.
            std::vector< std::string >  paths = m_finder->getSearchPaths();
            std::stringstream cmd;
            for( auto pathIter = paths.begin(); pathIter != paths.end(); ++pathIter )
            {
                const std::string& path = *pathIter;
                // Do we want to allow arbitrary extensions?  No for now.
                //cmd << "package.path = package.path .. '"
                //    << path << "\\\\?;'\n";
                cmd << "package.path = package.path .. '"
                    << path << "\\?.lua;'\n";
            }
            std::string cmdClean = cmd.str();
            // replace back-slashes with forward slashes
            boost::replace_all( cmdClean, "\\", "/" );
            // get rid of double forward slashes
            boost::replace_all( cmdClean, "//", "/" );

            LOG_DEBUG(g_log) << "Adding paths:\n" << cmdClean;
            runScriptFromString( cmdClean );
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
        
        /// Excute the given script in the current lua context.
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
                          << errType;
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
    private:
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
                std::cerr << "Lua runtime error: " << errMsg << '\n';
                LOG_ERROR(g_log) << "Lua runtime error: " << errMsg;
                lua_pop( m_lua, 1 ); // pop error off stack
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

    void bindInterpreter( lua_State* lua )
    {
        luabind::module( lua )
        [
         luabind::class_< LuaInterpreter >( "LuaInterpreter" )
         .def( "load", &LuaInterpreter::runScriptFromFile )
         ];
    }

}


#endif
