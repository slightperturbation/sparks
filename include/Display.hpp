#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "Spark.hpp"
#include "Viewport.hpp"
#include "GuiEventSubscriber.hpp"

namespace spark
{
    /// Display manages a collection of viewports
    class Display : public GuiEventSubscriber
    {
    public:
        Display( void );
        virtual ~Display() {}
        virtual void render( StateManager& renderer ) = 0;
        virtual void resizeViewport( int left, int bottom,
                                    int width, int height ) override;

        virtual void setPerspective( PerspectiveProjectionPtr camera );
        virtual void setEyeTracker( ConstEyeTrackerPtr eyeTracker );
        virtual void setFrameBufferRenderTarget( FrameBufferRenderTargetPtr target );
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
        ConstEyeTrackerPtr m_eyeTracker;
        
        /// FrameBufferRenderTarget is responsible for calling glViewport
        /// during RenderTarget::preRender()
        FrameBufferRenderTargetPtr m_frameBuffer;
    };

    /// MonoDisplay renders to one viewport.
    class MonoDisplay : public Display
    {
    public:
        MonoDisplay( void );
        virtual ~MonoDisplay() {}
        
        virtual void render( StateManager& renderer ) override;
    };

    /// Allow side-by-side 3D rendering
    class SideBySideDisplay : public Display
    {
    public:
        SideBySideDisplay( );
        virtual ~SideBySideDisplay() {}
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
        virtual void render( StateManager& renderer ) override;
    };
    
} // end namespace spark
#endif
