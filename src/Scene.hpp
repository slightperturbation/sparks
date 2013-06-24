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
namespace spark
{
    //// Scene manages the set of renderables to be drawn with render()
    class Scene
    {
    public:
        Scene( void );
        ~Scene();
        void add( RenderPassPtr rp );
        void add( RenderablePtr r );
        /// Build render commands for this frame.
        void prepareRenderCommands( void );
        /// Send all queued render commands to the graphics card in-order.
        /// Render command queue is empty when this function exits.
        /// Note that for the render to display anything to the default
        /// OpenGL context, there must be at least one render pass with
        /// target set to display.
        void render( void );
        
        /// Update scene objects per-frame
        void update( float dt );

        /// Print all passes to INFO-level log
        void logPasses( void ) const;
    private:
        RenderPassList m_passes;
        /// The current list of commands.  The first command is always
        /// the highest priority command.  Commands are ordered first
        /// by RenderPasses.
        RenderCommandQueue m_commands;
        Renderables m_renderables;
    };
} // end namespace spark

#endif
