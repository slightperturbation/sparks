#ifndef SOFTTESTDECLARATIONS_HPP
#define SOFTTESTDECLARATIONS_HPP


#include <memory>
#include <vector>


// Forward Declarations
class Renderable;
typedef std::shared_ptr< Renderable > RenderablePtr;
typedef std::vector< RenderablePtr > Renderables;
class RenderContext;
typedef std::shared_ptr< RenderContext > RenderContextPtr;
typedef std::shared_ptr< const RenderContext > ConstRenderContextPtr;

void checkOpenGLErrors( void );

#endif
