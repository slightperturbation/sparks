//
//  RenderCommand.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_RenderCommand_hpp
#define sparks_RenderCommand_hpp

#include "Spark.hpp"
#include "Utilities.hpp"
#include "IlluminationModel.hpp"
#include "Material.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <functional>
#include <sstream>

namespace spark
{
    /// Command that encapsulates all the needed information
    /// to render a particular object.
    /// RenderCommands can be ordered for rendering to support
    /// effects such as transparency and to reduce the number
    /// of calls to the graphics API.
    class RenderCommand
    {
    public:
        /// Apply this render command.  Only apply changes that differ
        /// from the precedingCommand.
        void operator() ( const RenderCommand& precedingCommand );

        ConstRenderPassPtr m_pass;
        ConstProjectionPtr m_perspective;
        ConstRenderablePtr m_renderable;
        ConstMaterialPtr m_material;
        IlluminationModel m_illuminationModel;
        friend std::ostream& operator<<( std::ostream& out, const RenderCommand& rc );
    };

    /// Function object for ordering the rendering of two RenderCommands
    /// RenderCommandCompare( A, B ) returns TRUE if **B** should be rendered FIRST
    /// Used by std::priority_queue to determine rendering order.
    struct RenderCommandCompare
        : public std::binary_function< const RenderCommand&, 
                                       const RenderCommand&, bool >
    {
        bool operator()( const RenderCommand& a, const RenderCommand& b ) const;
    };
} // end namespace spark
#endif
