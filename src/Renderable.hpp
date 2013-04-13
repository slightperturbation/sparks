#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "SoftTestDeclarations.hpp"

/// Abstract class for objects to be rendered
class Renderable
{
public:
    virtual ~Renderable() {}
    virtual void render( PerspectivePtr renderContext ) = 0;
    virtual void update( float dt ) = 0;
    virtual void loadTextures() = 0;
    virtual void loadShaders() = 0;
};

#endif

