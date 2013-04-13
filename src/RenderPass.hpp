//
//  RenderPass.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_RenderPass_hpp
#define sparks_RenderPass_hpp

#include "SoftTestDeclarations.hpp"

RenderCommand createRenderCommand( ConstRenderPassPtr aRenderPass, 
                                   ConstRenderablePtr aRenderable );

/// Encapsulates a single rendering pass over all of the renderables in
/// a given scene.  A scene with only a single pass will likely render
/// directly to the display and render every renderable.  Multiple
/// passes can be used to implement shadow maps, reflections, full-screen
/// overlay effects, etc.  In those cases, the target is generally an
/// OpenGL FrameBufferObject that will be used by subsequent passes.
/// A RenderPass's role is only to create RenderCommands, not to do 
/// any rendering itself.
class RenderPass
{
public:
    RenderPass( void );

    void initialize( RenderTargetPtr aTarget, 
                     PerspectivePtr aPerspective )
    { m_target = aTarget; m_perspective = aPerspective; }

    void initialize( RenderTargetPtr aTarget, 
                     PerspectivePtr aPerspective,
                     float aPriority )
    { m_target = aTarget; m_perspective = aPerspective; m_priority = aPriority; }

    /// Sets OpenGL state to draw to the render target 
    /// (e.g., display device or render-to-texture)
    void preRender( void ) const;
    void postRender( void ) const;

    float priority( void ) const 
    { return m_priority; }
    
    //TODO make RenderPass const
    friend RenderCommand createRenderCommand( ConstRenderPassPtr aRenderPass, 
                                              ConstRenderablePtr aRenderable );
private:
    RenderTargetPtr m_target;
    PerspectivePtr m_perspective;
    // m_target
    float m_priority;
};

bool renderPassCompareByPriority( ConstRenderPassPtr a, 
                                  ConstRenderPassPtr b );
RenderCommand createRenderCommand( ConstRenderPassPtr aRenderPass, 
                                   ConstRenderablePtr aRenderable );

#endif
