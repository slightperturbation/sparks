//
//  Scene.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_Scene_hpp
#define sparks_Scene_hpp

#include "SoftTestDeclarations.hpp"
#include "RenderPass.hpp"
#include "RenderCommand.hpp"

class Scene
{
public:
    Scene( void );
    ~Scene();
    
    void add( RenderPassPtr rp ) { m_passes.push_back( rp ); }
    void add( RenderablePtr r ) { m_renderables.push_back( r ); }

    /// Build render commands for this frame.
    void prepareRenderCommands( void );

    /// Send all queued render commands to the graphics card in-order.
    /// Render command queue is empty when this function exits.
    /// Note that for the render to display anything to the default
    /// OpenGL context, there must be at least one render pass with
    /// target set to display.
    void render( void );

private:
    RenderPassList m_passes;
    RenderCommandQueue m_commands;
    Renderables m_renderables;
};

#endif
