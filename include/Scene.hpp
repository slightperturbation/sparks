//
//  Scene.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_Scene_hpp
#define sparks_Scene_hpp

#include "Spark.hpp"

#include "RenderPass.hpp"
#include "RenderCommand.hpp"

namespace spark
{
    /// Scene manages the set of renderables to be drawn with 
    /// Renderable::render() and updated with Renderable::update().
    /// Different instances of Scene can share Renderables and RenderPasses
    class Scene
    {
    public:
        Scene( void );
        ~Scene();
        
        /// Render the given renderpass when this Scene object's render()
        /// method is called.
        void add( RenderPassPtr rp );
        /// Render the given renderable when this Scene object's render()
        /// method is called.  Note that the Renderable must also be
        /// associated with a material (see Renderable::setMaterialForPassName())
        /// for a RenderPass that has been Scene::add()'d to this Scene.
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
        void update( double dt );

        /// Update scene objects per-frame
        void fixedUpdate( double dt );

        /// Delete references to all held resources
        void reset( void );

        /// Print all passes to INFO-level log
        void logPasses( void ) const;

        /// Print all renderables known to this scene to INFO-level log.
        void logRenderables( void ) const;
        
        /// Return existing render pass with given name, if registered.
        RenderPassPtr getPass( const RenderPassName& name ) const;
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
