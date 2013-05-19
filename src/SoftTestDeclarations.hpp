#ifndef SOFTTESTDECLARATIONS_HPP
#define SOFTTESTDECLARATIONS_HPP

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

class Renderable;
typedef std::shared_ptr< Renderable > RenderablePtr;
typedef std::shared_ptr< const Renderable > ConstRenderablePtr;
typedef std::vector< RenderablePtr >  Renderables;
typedef std::string RenderableName;

class Projection;
class PerspectiveProjection;
class OrthogonalProjection;
typedef std::shared_ptr< Projection > PerspectivePtr;
typedef std::shared_ptr< const Projection > ConstPerspectivePtr;
typedef std::shared_ptr< PerspectiveProjection > PerspectiveProjectionPtr;
typedef std::shared_ptr< const PerspectiveProjection > ConstPerspectiveProjectionPtr;
typedef std::shared_ptr< OrthogonalProjection > OrthogonalProjectionPtr;
typedef std::shared_ptr< const OrthogonalProjection > ConstOrthogonalProjectionPtr;


class RenderCommand;
typedef std::shared_ptr< RenderCommand > RenderCommandPtr;
struct RenderCommandCompare;
typedef std::priority_queue< RenderCommand, std::vector< RenderCommand >, RenderCommandCompare > RenderCommandQueue;

class RenderPass;
typedef std::shared_ptr< RenderPass > RenderPassPtr;
typedef std::shared_ptr< const RenderPass > ConstRenderPassPtr;
typedef std::list< RenderPassPtr > RenderPassList;
typedef std::string RenderPassName;

class Scene;
typedef std::shared_ptr< Scene > ScenePtr;
typedef std::shared_ptr< const Scene > ConstScenePtr;

class RenderTarget;
typedef std::shared_ptr< RenderTarget > RenderTargetPtr;
typedef std::shared_ptr< const RenderTarget > ConstRenderTargetPtr;

class ShaderInstance;
typedef std::shared_ptr< ShaderInstance > ShaderInstancePtr;
typedef std::shared_ptr< const ShaderInstance > ConstShaderInstancePtr;

class Material;
typedef std::shared_ptr< Material > MaterialPtr;
typedef std::shared_ptr< const Material > ConstMaterialPtr;
typedef std::string MaterialName;
typedef std::string ShaderUniformName;

class TextureUnit;
typedef std::shared_ptr< TextureUnit > TextureUnitPtr;
typedef std::shared_ptr< const TextureUnit> ConstTextureUnitPtr;

class TextureManager;
typedef std::shared_ptr< TextureManager > TextureManagerPtr;
typedef std::string TextureName;

class ShaderManager;
typedef std::shared_ptr< ShaderManager > ShaderManagerPtr;
typedef std::string ShaderName;

class FileAssetFinder;
typedef std::shared_ptr< FileAssetFinder > FileAssetFinderPtr;

class Mesh;
typedef std::shared_ptr< Mesh > MeshPtr;

// Define standard names for common render passes
const RenderPassName g_colorRenderPassName = "ColorPass";

#endif
