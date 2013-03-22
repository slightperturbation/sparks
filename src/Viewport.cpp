#include "Viewport.hpp"
#include "Renderable.hpp"
#include "RenderContext.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

void Viewport::render( const Renderables& scene, RenderContext& context ) const
{
    glViewport( (GLsizei)m_left, (GLsizei)m_bottom, (GLsizei)m_width, (GLsizei)m_height );
    
    for( auto r = scene.begin(); r != scene.end(); ++r )
    {
        (*r)->render( context );
    }
}

void Viewport::setExtents( int left, int bottom, int width, int height )
{
    m_width = width; m_height = height; m_bottom = bottom; m_left = left;
}

