#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "Spark.hpp"
#include "Viewport.hpp"
namespace spark
{
    /// Display manages a collection of viewports
    class Display
    {
    public:
        Display( ProjectionPtr context );
        virtual ~Display() {}
        virtual void render( const Renderables& scene ) = 0;
        virtual void resizeWindow( int width, int height ) = 0;
    protected:
        ProjectionPtr m_context;
    };

    /// SimpleDisplay renders to one viewport.
    /// \todo rename to "SingleDisplay" to better represent the use.
    class SimpleDisplay : public Display
    {
    public:
        SimpleDisplay( ProjectionPtr context );
        virtual ~SimpleDisplay() {}
        virtual void render( const Renderables& scene );
        virtual void resizeWindow( int width, int height );
    private:
        Viewport m_viewport;
    };

    /// Handles side-by-side 3d rendering
    /// \todo abstract 3d display interface for tweaking eye params
    class SideBySideDisplay : public Display
    {
    public:
        SideBySideDisplay( ProjectionPtr context );
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
} // end namespace spark
#endif
