#include "LuaInterpreter.hpp"

int
spark
::reportLuaError( lua_State* L )
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

void
spark
::bindSparkFacade( lua_State* lua )
{
    // Renderable
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
    
#if 0
    // TextRenderable
    luabind::module( lua )
    [
     luabind::class_< TextRenderable, Renderable, TextRenderablePtr >( "TextRenderable" )
     .def( "initialize", &TextRenderable::initialize )
     .def( "setText", &TextRenderable::setText )
     ];
#endif
    // RenderPass
    luabind::module( lua )
    [
     luabind::class_< RenderPass, RenderPassPtr >( "RenderPass" )
     .def( "name", &RenderPass::name )
     .def( "disableBlending", &RenderPass::disableBlending )
     .def( "useAdditiveBlending", &RenderPass::useAdditiveBlending )
     .def( "useInterpolatedBlending", &RenderPass::useInterpolatedBlending )
     .def( "useMaxBlending", &RenderPass::useMaxBlending )
     .def( "setBlending",
          (void (RenderPass::*)(GLenum, GLenum) )
          &RenderPass::setBlending )
     .def( "setBlending",
          (void (RenderPass::*)(GLenum, GLenum, GLenum) )
          &RenderPass::setBlending )
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
    
    // RenderTarget
    luabind::module( lua )
    [
     luabind::class_< RenderTarget,
     RenderTargetPtr >( "RenderTarget" )
     .def( "setClearColor", &RenderTarget::setClearColor )
     ];
    // Projection
    luabind::module( lua )
    [
     luabind::class_< Projection, ProjectionPtr >( "Projection" )
     ];
    // PerspectiveProjection
    luabind::module( lua )
    [
     // Note! The luabind docs explicitly say to bind with the base-class's
     // smart pointer.  This is *wrong*, bind to the concrete class's
     // smart pointer.
     luabind::class_< PerspectiveProjection,
     Projection,
     PerspectiveProjectionPtr >( "PerspectiveProjection" )
     // modifiers
     .def( "cameraUp", (void (PerspectiveProjection::*)(float,float,float) )&PerspectiveProjection::cameraUp )
     .def( "cameraPos", (void (PerspectiveProjection::*)(float,float,float) )&PerspectiveProjection::cameraPos )
     .def( "cameraTarget", (void (PerspectiveProjection::*)(float,float,float) )&PerspectiveProjection::cameraTarget )
     .def( "fov", (void (PerspectiveProjection::*)(float) )&PerspectiveProjection::fov )
     
     ];
    
    // OrthogonalProjection
    luabind::module( lua )
    [
     luabind::class_< OrthogonalProjection,
     Projection,
     OrthogonalProjectionPtr >( "OrthogonalProjection" )
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
          (RenderPassPtr (SparkFacade::*)(float,
                                          const RenderPassName&,
                                          RenderTargetPtr,
                                          MaterialPtr ))
          &SparkFacade::createPostProcessingRenderPass )
     .def( "createPostProcessingRenderPass",
          (RenderPassPtr (SparkFacade::*)(float,
                                          const RenderPassName&,
                                          const TextureName&,
                                          RenderTargetPtr,
                                          const ShaderName& ))
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
     .def( "createOverlayRenderPass",
          &SparkFacade::createOverlayRenderPass )
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
     .def( "createQuad",
          &SparkFacade::createQuad )
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

void
spark
::bindTextureManager( lua_State* lua )
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

void
spark
::bindShaderManager( lua_State* lua )
{
    luabind::module( lua )
    [
     luabind::class_< ShaderManager,
     ShaderManagerPtr >( "ShaderManager" )
     .def( "loadShaderFromFiles", &ShaderManager::loadShaderFromFiles)
     .def( "reloadAllShaders", &ShaderManager::reloadAllShaders )
     .def( "releaseAll", &ShaderManager::releaseAll )
     ];
}

void
spark
::bindGLM( lua_State* lua )
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

void
spark
::bindConstants( lua_State* lua )
{
    luabind::globals( lua )["GL_ONE"] = GL_ONE;
    luabind::globals( lua )["GL_ZERO"] = GL_ZERO;
    luabind::globals( lua )["GL_SRC_COLOR"] = GL_SRC_COLOR;
    luabind::globals( lua )["GL_DST_COLOR"] = GL_DST_COLOR;
    luabind::globals( lua )["GL_SRC_ALPHA"] = GL_SRC_ALPHA;
    luabind::globals( lua )["GL_DST_ALPHA"] = GL_DST_ALPHA;
    luabind::globals( lua )["GL_ONE_MINUS_SRC_ALPHA"] = GL_ONE_MINUS_SRC_ALPHA;
    luabind::globals( lua )["GL_ONE_MINUS_DST_ALPHA"] = GL_ONE_MINUS_DST_ALPHA;
    luabind::globals( lua )["GL_ONE_MINUS_SRC_COLOR"] = GL_ONE_MINUS_SRC_COLOR;
    luabind::globals( lua )["GL_ONE_MINUS_DST_COLOR"] = GL_ONE_MINUS_DST_COLOR;
    luabind::globals( lua )["GL_CONSTANT_COLOR"] = GL_CONSTANT_COLOR;
    luabind::globals( lua )["GL_CONSTANT_ALPHA"] = GL_CONSTANT_ALPHA;
    
    // BlendEquations
    // http://www.opengl.org/sdk/docs/man/xhtml/glBlendEquation.xml
    luabind::globals( lua )["GL_FUNC_ADD"] = GL_FUNC_ADD;
    luabind::globals( lua )["GL_FUNC_SUBTRACT​"] = GL_FUNC_SUBTRACT;
    luabind::globals( lua )["GL_FUNC_REVERSE_SUBTRACT"] = GL_FUNC_REVERSE_SUBTRACT;
    luabind::globals( lua )["GL_MIN"] = GL_MIN;
    luabind::globals( lua )["GL_MAX"] = GL_MAX;
    
}

void
spark
::bindInterpreter( lua_State* lua )
{
    luabind::module( lua )
    [
     luabind::class_< LuaInterpreter >( "LuaInterpreter" )
     .def( "load", &LuaInterpreter::runScriptFromFile )
     ];
}


//////////////////////////////////////////////////////////////////////////////

spark::LuaInterpreter
::LuaInterpreter( FileAssetFinderPtr finder )
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
        bindConstants( m_lua );
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

spark::LuaInterpreter
::~LuaInterpreter()
{
    lua_close( m_lua );
}








