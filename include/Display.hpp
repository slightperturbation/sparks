#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "Spark.hpp"
#include "GuiEventSubscriber.hpp"
#include "Utilities.hpp"

namespace spark
{
    /// Display manages a collection of viewports
    class Display : public GuiEventSubscriber
    {
    public:
        Display( void );
        virtual ~Display() {}
        virtual void activate() = 0;
        virtual void render( StateManager& renderer ) = 0;
        virtual void resizeViewport( int left, int bottom,
                                    int width, int height ) override;

        void setPerspective( PerspectiveProjectionPtr camera );
        void setEyeTracker( EyeTrackerPtr eyeTracker );
        void setFrameBufferRenderTarget( FrameBufferRenderTargetPtr target );
        void setWindow( OpenGLWindow* window );

    protected:
        /// Store the current window's dimensions and location
        /// Note this is not a "viewport", but the total window
        /// we are drawing into.  The difference is important for
        /// Side-by-Side stereo displays.
        int m_windowLeft;
        int m_windowBottom;
        int m_windowWidth;
        int m_windowHeight;

        PerspectiveProjectionPtr m_camera;
        EyeTrackerPtr m_eyeTracker;
        
        /// FrameBufferRenderTarget is responsible for calling glViewport
        /// during RenderTarget::preRender()
        FrameBufferRenderTargetPtr m_frameBuffer;

        OpenGLWindow* m_window;
    };

    /// MonoDisplay renders to one viewport.
    class MonoDisplay : public Display
    {
    public:
        MonoDisplay( void );
        virtual ~MonoDisplay() {}
        
        virtual void activate() override;
        virtual void render( StateManager& renderer ) override;
    };

    /// Allow side-by-side 3D rendering
    class SideBySideDisplay : public Display
    {
    public:
        SideBySideDisplay( );
        virtual ~SideBySideDisplay() {}
        virtual void activate() override;
        virtual void render( StateManager& renderer ) override;

        float getEyeSeparation( void ) const { return m_eyeSeparationDistance; }
        void setEyeSeparation( float distance ) { m_eyeSeparationDistance = distance; }
        
        void enableOculusDistortion( void );
        void disableOculusDistoration( void );
    private:
        float m_eyeSeparationDistance;
    };
    
    class QuadBufferStereoDisplay : public MonoDisplay
    {
    public:
        QuadBufferStereoDisplay( );
        virtual ~QuadBufferStereoDisplay() {}

        virtual void activate( ) override;
        virtual void render( StateManager& renderer ) override;
    };
    
} // end namespace spark
#endif
