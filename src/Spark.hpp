#ifndef SPARK_SPARK_HPP
#define SPARK_SPARK_HPP

//#define BOOST_ALL_NO_LIB
#define _CRT_SECURE_NO_WARNINGS

#ifdef WIN32
#  define _CRTDBG_MAP_ALLOC
#endif

#define CPPLOG_THREADING
#define CPPLOG_FILTER_LEVEL               LL_TRACE
//#define CPPLOG_FILTER_LEVEL               LL_DEBUG
//#define CPPLOG_FILTER_LEVEL               LL_INFO
//#define CPPLOG_FILTER_LEVEL               LL_WARN
//#define CPPLOG_FILTER_LEVEL               LL_ERROR
//#define CPPLOG_FILTER_LEVEL               LL_FATAL
#include "cpplog.hpp"

#include <memory>
#include <vector>
#include <queue>
#include <list>

///////////////////////////////////////////////////////////////////////////
// Definition of Global Variables


/// Logging
/// LOG_TRACE for per-frame log messages
/// LOG_DEBUG for unusual but OK cases
/// LOG_WARNING for potential problems
/// LOG_ERROR for true error conditions, possibly recoverable
/// LOG_FATAL for unrecoverable errors (always logged)
extern cpplog::BaseLogger* g_log;

///////////////////////////////////////////////////////////////////////////
// Forward Declarations


// Luabind requires boost::shared_ptr, cannot use spark::shared_ptr
// Alias either boost or std shared_ptrs, both are supported outside of luabind.
// Note that std::shared_ptr is preferred, and luabind may be fixed.
#define USE_BOOST_SHARED_PTR
#ifdef USE_BOOST_SHARED_PTR
# include <boost/shared_ptr.hpp>
# include <boost/enable_shared_from_this.hpp>
# include <boost/weak_ptr.hpp>
namespace spark 
{
    using boost::shared_ptr;
    using boost::const_pointer_cast;
    using boost::weak_ptr;
    using boost::enable_shared_from_this;
};
#else
# include <memory>
namespace spark 
{
    using std::shared_ptr;
    using std::const_pointer_cast;
    using std::weak_ptr;
    using std::enable_shared_from_this;
};
#endif

namespace spark
{
    class FileAssetFinder;
    typedef spark::shared_ptr< FileAssetFinder > FileAssetFinderPtr;
    
    class Material;
    typedef spark::shared_ptr< Material > MaterialPtr;
    typedef spark::shared_ptr< const Material > ConstMaterialPtr;
    typedef std::string MaterialName;
    typedef std::string ShaderUniformName;
    
    class Mesh;
    typedef spark::shared_ptr< Mesh > MeshPtr;
    
    class Projection;
    class PerspectiveProjection;
    class OrthogonalProjection;
    typedef spark::shared_ptr< Projection > ProjectionPtr;
    typedef spark::shared_ptr< const Projection > ConstProjectionPtr;
    typedef spark::shared_ptr< PerspectiveProjection > PerspectiveProjectionPtr;
    typedef spark::shared_ptr< const PerspectiveProjection > ConstPerspectiveProjectionPtr;
    typedef spark::shared_ptr< OrthogonalProjection > OrthogonalProjectionPtr;
    typedef spark::shared_ptr< const OrthogonalProjection > ConstOrthogonalProjectionPtr;

    class Renderable;
    typedef spark::shared_ptr< Renderable > RenderablePtr;
    typedef spark::shared_ptr< const Renderable > ConstRenderablePtr;
    typedef std::vector< RenderablePtr >  Renderables;
    typedef std::string RenderableName;
    
    class RenderCommand;
    typedef spark::shared_ptr< RenderCommand > RenderCommandPtr;
    struct RenderCommandCompare;
    typedef std::priority_queue< RenderCommand, std::vector< RenderCommand >, RenderCommandCompare > RenderCommandQueue;

    class RenderPass;
    typedef spark::shared_ptr< RenderPass > RenderPassPtr;
    typedef spark::shared_ptr< const RenderPass > ConstRenderPassPtr;
    typedef std::list< RenderPassPtr > RenderPassList;
    typedef std::string RenderPassName;

    class Scene;
    typedef spark::shared_ptr< Scene > ScenePtr;
    typedef spark::shared_ptr< const Scene > ConstScenePtr;

    class RenderTarget;
    typedef spark::shared_ptr< RenderTarget > RenderTargetPtr;
    typedef spark::shared_ptr< const RenderTarget > ConstRenderTargetPtr;

    class ShaderInstance;
    typedef spark::shared_ptr< ShaderInstance > ShaderInstancePtr;
    typedef spark::shared_ptr< const ShaderInstance > ConstShaderInstancePtr;

    class ShaderManager;
    typedef spark::shared_ptr< ShaderManager > ShaderManagerPtr;
    typedef std::string ShaderName;
    
    class SparkFacade;
    typedef spark::shared_ptr< SparkFacade > SparkFacadePtr;
    
    class TextureUnit;
    typedef spark::shared_ptr< TextureUnit > TextureUnitPtr;
    typedef spark::shared_ptr< const TextureUnit> ConstTextureUnitPtr;

    class TextureManager;
    typedef spark::shared_ptr< TextureManager > TextureManagerPtr;
    typedef std::string TextureName;
    
    class VolumeData;
    typedef spark::shared_ptr< VolumeData > VolumeDataPtr;

    // Define standard names for common render passes
    const RenderPassName g_opaqueRenderPassName = "OpaquePass";
    const RenderPassName g_transparencyRenderPassName = "TransparencyPass";

} // end namespace spark
#endif
