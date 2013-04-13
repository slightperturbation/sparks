#ifndef SOFTTESTDECLARATIONS_HPP
#define SOFTTESTDECLARATIONS_HPP

#define BOOST_ALL_NO_LIB
#define _CRT_SECURE_NO_WARNINGS
//#define CPPLOG_THREADING
//#define CPPLOG_FILTER_LEVEL               LL_WARN
//#define CPPLOG_FILTER_LEVEL               LL_ERROR
#include "cpplog.hpp"

#include <memory>
#include <vector>
#include <queue>
#include <list>

///////////////////////////////////////////////////////////////////////////
// Definition of Global Variables

//extern cpplog::StdErrLogger g_log;
extern cpplog::FileLogger g_log;

///////////////////////////////////////////////////////////////////////////
// Forward Declarations

class Renderable;
typedef std::shared_ptr< Renderable > RenderablePtr;
typedef std::shared_ptr< const Renderable > ConstRenderablePtr;
typedef std::vector< RenderablePtr >  Renderables;

class Perspective;
typedef std::shared_ptr< Perspective > PerspectivePtr;
typedef std::shared_ptr< const Perspective > ConstPerspectivePtr;

class RenderCommand;
typedef std::shared_ptr< RenderCommand > RenderCommandPtr;
struct RenderCommandCompare;
typedef std::priority_queue< RenderCommand, std::vector< RenderCommand >, RenderCommandCompare > RenderCommandQueue;

class RenderPass;
typedef std::shared_ptr< RenderPass > RenderPassPtr;
typedef std::shared_ptr< const RenderPass > ConstRenderPassPtr;
typedef std::list< RenderPassPtr > RenderPassList;

class Shader;
typedef std::shared_ptr< Shader > ShaderPtr;
typedef std::shared_ptr< const Shader > ConstShaderPtr;

class Scene;
typedef std::shared_ptr< Scene > ScenePtr;
typedef std::shared_ptr< const Scene > ConstScenePtr;

class Texture {};
typedef std::vector< Texture > Textures;

class TextureManager;
typedef std::shared_ptr< TextureManager > TextureManagerPtr;

class RenderTarget;
typedef std::shared_ptr< RenderTarget > RenderTargetPtr;
typedef std::shared_ptr< const RenderTarget > ConstRenderTargetPtr;


#endif
