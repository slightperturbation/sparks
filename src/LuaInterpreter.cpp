#include "LuaInterpreter.hpp"

#include "TextRenderable.hpp"
#include "TissueMesh.hpp"
#include "SlicedVolume.hpp"
#include "Fluid.hpp"
#include "Utilities.hpp" // for glm operator<< functions

#include <luabind/operator.hpp>

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
::bindSceneFacade( lua_State* lua )
{
    //////////////////////////////////////////////////////////// Renderable
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
     .def( "scale",
          (void (Renderable::*)(const glm::vec3&) )
          &Renderable::scale )
     .def( "rotate",
          &Renderable::rotate )
     .def( "setMaterialForPassName",
          &Renderable::setMaterialForPassName )
     ];
    
    //////////////////////////////////////////////////////// TextRenderable
    luabind::module( lua )
    [
     luabind::class_< TextRenderable, Renderable, TextRenderablePtr >( "TextRenderable" )
     .def( "initialize", &TextRenderable::initialize )
     .def( "setText", &TextRenderable::setText )
     .def( "getSizeInPixels", &TextRenderable::getSizeInPixels )
     ];

    //////////////////////////////////////////////////////////// TissueMesh
    luabind::module( lua )
    [
     luabind::class_< TissueMesh, Renderable, TissueMeshPtr >( "TissueMesh" )
     .def( "accumulateHeat", &TissueMesh::accumulateHeat )
     .def( "getTempMapTextureName", &TissueMesh::getTempMapTextureName )
     .def( "getConditionMapTextureName", &TissueMesh::getConditionMapTextureName )
     ];

    ///////////////////////////////////////////////////////////SlicedVolume
    luabind::module( lua )
    [
        luabind::class_< SlicedVolume, Renderable, SlicedVolumePtr >( "SlicedVolume" )
        .def( "setCameraDirection", &SlicedVolume::setCameraDirection )
    ];

    /////////////////////////////////////////////////////////// Fluid
    luabind::module( lua )
    [
        luabind::class_< Fluid, FluidPtr >( "Fluid" )
        .def( "setViscosity", &Fluid::setViscosity )
        .def( "setDiffusion", &Fluid::setDiffusion )
        .def( "setVorticity", &Fluid::setVorticity )
        .def( "setAbsorption", &Fluid::setAbsorption )
        .def( "setGravityFactor", &Fluid::setGravityFactor )
        .def( "setSolverIterations", &Fluid::setSolverIterations )
        .def( "reset", &Fluid::reset )
    ];

    /////////////////////////////////////////////////////////// FontManager
    luabind::module( lua )
    [
     luabind::class_< FontManager, FontManagerPtr >( "FontManager" )
     .def( "addFont", &FontManager::addFont )
     .def( "getFontAtlasTextureName", &FontManager::getFontAtlasTextureName )
    ];

    //////////////////////////////////////////////////////////// RenderPass
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
    
    ////////////////////////////////////////////////////////// RenderTarget
    luabind::module( lua )
    [
     luabind::class_< RenderTarget,
     RenderTargetPtr >( "RenderTarget" )
     .def( "setClearColor", &RenderTarget::setClearColor )
     ];

    //////////////////////////////////////////////////////////// Projection
    luabind::module( lua )
    [
     luabind::class_< Projection, ProjectionPtr >( "Projection" )
     ];
    ///////////////////////////////////////////////// PerspectiveProjection
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
    
    ///////////////////////////////////////////////////////////////// Input
    luabind::module( lua )
    [
     luabind::class_< Input, InputPtr >( "Input" )
     .def( "isKeyDown", &Input::isKeyDown )
     .def( "getTransform", &Input::getTransform )
     .def( "getPosition", &Input::getPosition )
     .def( "isButtonPressed", &Input::isButtonPressed )
     .def( "getPositionRange", &Input::getPositionRange )
     ];
    
    ////////////////////////////////////////////////// OrthogonalProjection
    luabind::module( lua )
    [
     luabind::class_< OrthogonalProjection,
     Projection,
     OrthogonalProjectionPtr >( "OrthogonalProjection" )
     ];

    /////////////////////////////////////////////////////////// SceneFacade
    luabind::module( lua )
    [
     luabind::class_< SceneFacade,
     SceneFacadePtr >( "SceneFacade" )
     .def( "createPostProcessingRenderPassAndTarget",
          &SceneFacade::createPostProcessingRenderPassAndTarget )
     .def( "createPostProcessingRenderPassAndScaledTarget",
          &SceneFacade::createPostProcessingRenderPassAndScaledTarget )
     .def( "createPostProcessingRenderPass",
          (RenderPassPtr (SceneFacade::*)(float,
                                          const RenderPassName&,
                                          RenderTargetPtr,
                                          MaterialPtr ))
          &SceneFacade::createPostProcessingRenderPass )
     .def( "createPostProcessingRenderPass",
          (RenderPassPtr (SceneFacade::*)(float,
                                          const RenderPassName&,
                                          const TextureName&,
                                          RenderTargetPtr,
                                          const ShaderName& ))
          &SceneFacade::createPostProcessingRenderPass )
     .def( "getFrameBufferRenderTarget",
          &SceneFacade::getFrameBufferRenderTarget )
     .def( "getCamera",
          &SceneFacade::getCamera )
     .def( "createScaledTextureRenderTarget",
          &SceneFacade::createScaledTextureRenderTarget )
     .def( "createTextureRenderTarget",
          &SceneFacade::createTextureRenderTarget )
     .def( "createRenderPass",
          &SceneFacade::createRenderPass )
     .def( "createRenderPassWithProjection",
          &SceneFacade::createRenderPassWithProjection )
     .def( "createOverlayRenderPass",
          &SceneFacade::createOverlayRenderPass )
     .def( "setMainRenderTarget",
          &SceneFacade::setMainRenderTarget )
     .def( "getMainRenderTarget",
          &SceneFacade::getMainRenderTarget )
     .def( "createMaterial",
          &SceneFacade::createMaterial )
     .def( "loadMesh",
          &SceneFacade::loadMesh )
     .def( "createCube",
          &SceneFacade::createCube )
     .def( "createQuad",
          &SceneFacade::createQuad )
     .def( "createTissue",
          &SceneFacade::createTissue )
     .def( "createLSpark",
          &SceneFacade::createLSpark )
     .def( "createText",
          &SceneFacade::createText )
     .def( "getFontManager",
          &SceneFacade::getFontManager )
     .def( "getRenderPassByName",
          &SceneFacade::getRenderPassByName )
     ];

    ////////////////////////////////////////////////////////////// Material
    luabind::module( lua )
    [
     luabind::class_< Material,
     MaterialPtr >( "Material" )
     .property( "name", (const std::string& (Material::*)(void) const )&Material::name,
                        (void (Material::*)(const std::string&) )&Material::name )
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
     .def( "setTextureParameteri", &TextureManager::setTextureParameteri )
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
     //.def_readwrite( "r", &glm::vec2::r )
     //.def_readwrite( "g", &glm::vec2::g )
     //.def_readwrite( "s", &glm::vec2::s )
     //.def_readwrite( "t", &glm::vec2::t )
     .def( "at", &vec2_at )
     .def( "set", &vec2_set )
     .def( luabind::const_self + luabind::const_self )
     .def( luabind::const_self - luabind::const_self )
     .def( luabind::const_self * float() )
     .def( luabind::const_self * luabind::other< glm::mat2 >() )
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
     // Union bindings not supported under Intel complier
     //.def_readwrite( "r", &glm::vec3::r )
     //.def_readwrite( "g", &glm::vec3::g )
     //.def_readwrite( "b", &glm::vec3::b )
     .def( "at", &vec3_at )
     .def( "set", &vec3_set )
     .def( luabind::const_self + luabind::const_self )
     .def( luabind::const_self - luabind::const_self )
     .def( luabind::const_self * float() )
     .def( luabind::const_self * luabind::other< glm::mat3 >() )
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
     // Union bindings not supported under Intel complier
     //.def_readwrite( "r", &glm::vec4::r )
     //.def_readwrite( "g", &glm::vec4::g )
     //.def_readwrite( "b", &glm::vec4::b )
     //.def_readwrite( "a", &glm::vec4::a )
     .def( "at", &vec4_at )
     .def( "set", &vec4_set )
     .def( luabind::const_self + luabind::const_self )
     .def( luabind::const_self - luabind::const_self )
     .def( luabind::const_self * float() )
     .def( luabind::const_self * luabind::other< glm::mat4 >() )
     ];
    
    luabind::module( lua )
    [
     luabind::class_< glm::mat3 >( "mat3" )
     .def( luabind::constructor<>() )
     .def( luabind::constructor<float>() )
     .def( luabind::constructor<glm::vec3,glm::vec3,glm::vec3>() )
     .def( "at", (glm::vec3& (*)(glm::mat3*, int))&mat3_at )
     .def( "at", (float& (*)(glm::mat3*, int, int))&mat3_at )
     .def( luabind::const_self + luabind::const_self )
     .def( luabind::const_self - luabind::const_self )
     .def( luabind::const_self * luabind::const_self )
     .def( luabind::const_self * luabind::other< glm::vec3 >() )
     ];
    
    luabind::module( lua )
    [
     luabind::class_< glm::mat4 >( "mat4" )
     .def( luabind::constructor<>() )
     .def( luabind::constructor<float>() )
     .def( luabind::constructor<glm::vec4,glm::vec4,glm::vec4,glm::vec4>() )
     .def( "at", (glm::vec4& (*)(glm::mat4*, int))&mat4_at )
     .def( "at", (float& (*)(glm::mat4*, int, int))&mat4_at )
     .def( luabind::const_self + luabind::const_self )
     .def( luabind::const_self - luabind::const_self )
     .def( luabind::const_self * luabind::const_self )
     .def( luabind::const_self * luabind::other< glm::vec4 >() )
     ];

    luabind::module( lua )
    [
        luabind::def( "mat4_set", &spark::mat4_set ),
        luabind::def( "mat4_at", &spark::mat4_at )
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
    
    luabind::module( lua )
    [
        luabind::def( "isWindows", &spark::isWindows ),
        luabind::def( "isApple", &spark::isApple ),
        luabind::def( "isLinux", &spark::isLinux )
    ];
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
        bindSceneFacade( m_lua );
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








