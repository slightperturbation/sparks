#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "SoftTestDeclarations.hpp"


/// Abstract class for objects to be rendered
class Renderable
{
public:
    virtual ~Renderable() {}
    virtual void render( const RenderContext& renderContext ) = 0;
    virtual void update( float dt ) = 0;
    virtual void loadTextures() = 0;
    virtual void loadShaders() = 0;
};
typedef std::shared_ptr< Renderable > RenderablePtr;
typedef std::vector< RenderablePtr >  Renderables;

#endif

