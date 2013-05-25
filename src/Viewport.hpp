#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include "SoftTestDeclarations.hpp"

namespace spark
{
    /// Viewport is a single view into the scene.
    /// The projection and other details of rendering are handled by the 
    /// contained RenderContext.
    class Viewport
    {
    public:
        Viewport( ) : m_left(0), m_bottom(0), m_width(800), m_height( 600 ) {}
        void render( const Renderables& scene, PerspectivePtr context ) const;
        void setExtents( int left, int bottom, int width, int height );

        int m_left;
        int m_bottom;
        int m_width;
        int m_height;
    };
} // end namespace spark
#endif

