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

#include <functional>
#include "Material.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <sstream>
namespace spark
{
    class Light
    {
    public:
        virtual ~Light() {}
        virtual std::string name( void ) const = 0;
        virtual void setShaderUniforms( MaterialPtr material ) = 0;
    };
    typedef spark::shared_ptr< Light > LightPtr;
    typedef spark::shared_ptr< const Light > ConstLightPtr;

    class AmbientLight : public Light
    {
        virtual std::string name( void ) const 
        {
            std::stringstream ss;
            ss << "Ambient[color=" << m_color << "]";
            return ss.str();
        }
        virtual void setShaderUniforms( MaterialPtr material )
        {
            material->setShaderUniform( "u_lightAmbientColor", m_color );
        }
    private:
        glm::vec4 m_color;
    };

    class PointLight : public Light
    {
        virtual std::string name( void ) const 
        {
            std::stringstream ss;
            ss << "u_pointLight[" << m_index << ", pos=" << m_pos << "]";
            return ss.str();
        }
        virtual void setShaderUniforms( MaterialPtr material )
        {
            material->setShaderUniform( name() + ".pos", m_pos );
            material->setShaderUniform( name() + ".color", m_color );
        }
    private:
        std::string uniformName( void ) const 
        {
            std::stringstream ss;
            ss << "u_pointLight[" << m_index << "]";
            return ss.str();
        }
        unsigned int m_index;
        glm::vec3 m_pos;
        glm::vec4 m_color;
    };

    class Illumination
    {
    public:
        virtual std::string name( void ) const 
        { 
            std::stringstream out;
            out << "{";
            int counter = 0;
            for( auto i = m_lights.begin(); i != m_lights.end(); ++i )
            {
                out << "(Light " << ++counter << "=" << (*i)->name() << ")";
            }
            out << "}";
            return out.str();
        }
        void setShaderUniforms( MaterialPtr material )
        {
            for( auto i = m_lights.begin(); i != m_lights.end(); ++i )
            {
                (*i)->setShaderUniforms( material );
            }
        }
    private:
        std::vector< LightPtr > m_lights;
    };
    typedef spark::shared_ptr< Illumination > IlluminationPtr;
    typedef spark::shared_ptr< const Illumination > ConstIlluminationPtr;
    ///////////

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
        ConstIlluminationPtr m_illumination;
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
