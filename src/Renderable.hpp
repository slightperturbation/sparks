#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "SoftTestDeclarations.hpp"
#include "Material.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace spark
{
    /// Interface for objects to be rendered.
    /// Exposes methods for changing shader and material properties.
    /// Note that a given Renderable has a mapping of render pass
    /// to material.  The linked material is applied for the current
    /// render pass.
    class Renderable
    {
    public:
        Renderable( const RenderableName& name )
        : m_name( name )
        { }
        virtual ~Renderable() { }

        virtual RenderableName name( void ) const { return m_name; }
        virtual void name( const RenderableName& aName ) { m_name = aName; }

        /// Emits GL primitives
        virtual void render( void ) const = 0;
        /// Ties the shader's "in" variables to the channels of this Renderable's data,
        /// E.g, "in vec3 v_position" in the shader needs to point to offset 0, stride 6.
        /// Need be done only after reloading shader or re-assigning Renderable to shader,
        /// or after changing the vertex type used by the Renderable
        /// Must be called for each shader that uses this Renderable
        virtual void attachShaderAttributes( GLuint shaderIndex ) = 0;
    
        const glm::mat4& getTransform( void ) const { return m_objectTransform; }
        
        void setTransform( const glm::mat4& mat ) { m_objectTransform = mat; }
        
        void transform( const glm::mat4& mat )
        { m_objectTransform = mat * m_objectTransform; }
        
        void scale( const glm::vec3& scaleFactor )
        {
            m_objectTransform = glm::scale( m_objectTransform, scaleFactor );
        }
        void scale( float scaleFactor )
        {
            m_objectTransform = glm::scale( m_objectTransform,
                                            scaleFactor,
                                            scaleFactor,
                                            scaleFactor );
        }
        void translate( const glm::vec3& x )
        {
            m_objectTransform = glm::translate( m_objectTransform, x );
        }
        void translate( float x, float y, float z )
        {
            m_objectTransform = glm::translate( m_objectTransform,
                                                glm::vec3(x,y,z) );
        }
        
        void rotate( float angleInDegrees, const glm::vec3& axis )
        {
            m_objectTransform = glm::rotate( m_objectTransform,
                                             angleInDegrees,
                                             axis );
        }
        void alignZAxisWithVector( const glm::vec3& dir )
        {
            glm::mat4 invModel = glm::inverse( m_objectTransform );
            glm::vec4 dirN4 = glm::normalize( invModel * glm::vec4(dir, 0) );
            glm::vec3 dirN( dirN4.x, dirN4.y, dirN4.z );
            float angle = std::acos( glm::dot( dirN, glm::vec3(0,0,1) ) );
            const float epsilon = 1e-20f;
            if( std::fabs( angle ) > epsilon )
            {
                glm::vec3 axis = glm::cross( dirN, glm::vec3(0,0,1 ) ) ;
                axis = glm::normalize( axis );
                this->rotate( glm::degrees(angle), axis );
            }
        }

        ConstMaterialPtr getMaterialForPassName( const RenderPassName& renderPassName ) const
        { 
            auto itr = m_materials.find( renderPassName );
            if( itr != m_materials.end() )
            {
                return (*itr).second;
            }
            else
            {
                LOG_TRACE(g_log) << "No material for pass \""
                << renderPassName << "\" for renderable \"" << m_name << "\".";
                return ConstMaterialPtr( NULL );
            }
        }
        void setMaterialForPassName( const RenderPassName& renderPassName, 
                                     MaterialPtr material )
        {
            LOG_DEBUG(g_log) << "Assigning material \"" << material->name() 
                << "\" to renderable \"" << name() << "\" for pass \"" 
                << renderPassName << "\".";
            m_materials[ renderPassName ] = material;
            GLuint shaderId = material->getGLShaderIndex();
            attachShaderAttributes( shaderId );
        }

    protected:
        RenderableName m_name;
        std::map< const RenderPassName, MaterialPtr > m_materials;
        glm::mat4 m_objectTransform;
    };

    /// Abstract class for objects that will be updated periodically.
    class Updatable
    {
    public:
        virtual ~Updatable() {}

        /// Update this object by dtSeconds.
        /// dtSeconds is the wall-clock time difference between updates. 
        /// Subclasses are free to ignore it and use a fixed time-step.
        virtual void update( float dtSeconds ) = 0;
    };
} // end namespace spark
#endif

