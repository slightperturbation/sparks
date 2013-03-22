#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "SoftTestDeclarations.hpp"
#include "Viewport.hpp"

/// Display manages a collection of viewports
class Display
{
public:
    Display(  RenderContext& context );
    virtual ~Display() {}
    virtual void render( const Renderables& scene ) = 0;
    virtual void resizeWindow( int width, int height ) = 0;
protected:
    RenderContext& m_context;
};

// TODO rename to SingleDisplay
class SimpleDisplay : public Display
{
public:
    SimpleDisplay( RenderContext& context );
    virtual ~SimpleDisplay() {}
    virtual void render( const Renderables& scene );
    virtual void resizeWindow( int width, int height );
private:
    Viewport m_viewport;
};

/// Handles side-by-side 3d rendering
//TODO abstract 3d display interface for tweaking eye params
class SideBySideDisplay : public Display
{
public:
    SideBySideDisplay( RenderContext& context );
    virtual ~SideBySideDisplay() {}
    virtual void render( const Renderables& scene );
    virtual void resizeWindow( int width, int height );
    float getEyeSeparation( void ) const { return m_eyeSeparationDistance; }
    void setEyeSeparation( float distance ) { m_eyeSeparationDistance = distance; }
private:
    Viewport m_rightViewport;
    Viewport m_leftViewport;
    float m_eyeSeparationDistance;
};


#endif
