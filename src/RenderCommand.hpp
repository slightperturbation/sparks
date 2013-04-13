//
//  RenderCommand.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_RenderCommand_hpp
#define sparks_RenderCommand_hpp

#include "SoftTestDeclarations.hpp"

#include "Shader.hpp"
//#include "Texture.hpp"

#include <vector>
#include <functional>
#include <queue>


class RenderCommand
{
public:
    /// Apply this render command.
    void operator() ( void ) const;

    ConstRenderPassPtr m_pass;
    ConstShaderPtr m_shader;
    ConstPerspectivePtr m_perspective;
    ConstRenderablePtr m_renderable;
    Textures m_textures;
};

struct RenderCommandCompare 
    : public std::binary_function< const RenderCommand&, const RenderCommand&, bool >
{
    bool operator()( const RenderCommand& a, const RenderCommand& b ) const;
};


#endif
