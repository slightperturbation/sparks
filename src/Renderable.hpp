#ifndef SPARK_RENDERABLE_HPP
#define SPARK_RENDERABLE_HPP

#include "Spark.hpp"

#include <glm/glm.hpp>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

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
        Renderable( const RenderableName& name );
        virtual ~Renderable();
        
        /// Update between renderings.  Used to swap computed data in to be
        /// rendered on next frame.
        virtual void update( float dt ) {}

        /// Provides the descriptive (not necessarily unique) name
        virtual RenderableName name( void ) const;
        virtual void name( const RenderableName& aName );

        /// Emits OpenGL primitives
        /// The RenderCommand provides the "context" in which the scene
        /// is being rendered.
        virtual void render( const RenderCommand& rc ) const = 0;
        /// Ties the shader's "in" variables to the channels of this 
        /// Renderable's data, e.g, "in vec3 v_position" in the shader 
        /// needs to point to offset 0, stride 6.
        /// Need be done only after reloading shader or re-assigning 
        /// Renderable to shader, or after changing the vertex type 
        /// used by the Renderable
        /// Must be called for each shader that uses this Renderable
        virtual void attachShaderAttributes( GLuint shaderIndex ) = 0;
        
        /// If set to true, this Renderable will ignore RenderPass's default
        /// Materials, only rendering if a material is explicitly assigned
        /// to this material for that RenderPass.
        /// Best used for "effect" renderables like full-screen overlay quads.
        void setRequireExplicitMaterial( bool explicitMat );
        bool requiresExplicitMaterial( void ) const;
        
        const glm::mat4& getTransform( void ) const;
        void setTransform( const glm::mat4& mat );
        void transform( const glm::mat4& mat );
        void scale( const glm::vec3& scaleFactor );
        void scale( float scaleFactor );
        void translate( const glm::vec3& x );
        void translate( float x, float y, float z );
        
        void rotate( float angleInDegrees, const glm::vec3& axis );
        void alignZAxisWithVector( const glm::vec3& dir );

        ConstMaterialPtr getMaterialForPassName( const RenderPassName& renderPassName ) const;
        void setMaterialForPassName( const RenderPassName& renderPassName, 
                                     MaterialPtr material );

        friend std::ostream& operator<<( std::ostream& out, 
            ConstRenderablePtr renderable );
        friend  std::ostream& operator<<( std::ostream& out, 
            RenderablePtr renderable );

    protected:
        RenderableName m_name;
        bool m_requiresExplicitMaterial;
        std::map< const RenderPassName, MaterialPtr > m_materials;
        glm::mat4 m_objectTransform;
    };


} // end namespace spark
#endif

