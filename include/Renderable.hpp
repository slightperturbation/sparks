#ifndef SPARK_RENDERABLE_HPP
#define SPARK_RENDERABLE_HPP

#include "Spark.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>

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
        /// Provides the descriptive (not necessarily unique) name
        virtual RenderableName name( void ) const;

        /// Sets a descriptive name for the Renderable
        virtual void name( const RenderableName& aName );

        /// Emits OpenGL primitives
        /// The RenderCommand provides the "context" in which the scene
        /// is being rendered.
        /// Updates are required to not change the OpenGL state.
        virtual void render( const RenderCommand& rc ) const = 0;

        /// If set to true, this Renderable will ignore RenderPass's default
        /// Materials, only rendering if a material is explicitly assigned
        /// to this material for that RenderPass.
        /// Best used for "effect" renderables like full-screen overlay quads.
        void setRequireExplicitMaterial( bool explicitMat );

        /// Returns true if this renderable will only be added to a
        /// RenderPass (using the RenderPass' default Material) when a
        /// material is explicitly assigned to that RenderPass.
        bool requiresExplicitMaterial( void ) const;
        
        /// Returns the current transform of this Renderable as a 4x4 matrix.
        const glm::mat4& getTransform( void ) const;

        /// Set the transform of this renderable to mat, ignores any previous 
        /// transformations applied.
        void setTransform( const glm::mat4& mat );

        /// Post-multiply the renderables current transform by mat.
        /// This is equivalent to:
        ///   r.setTransform( r.getTransform() * mat );
        void transform( const glm::mat4& mat );

        /// Non-uniformly scale the Renderable by scaleFactor in all three dimensions.
        /// See glm::scale()
        void scale( const glm::vec3& scaleFactor );

        /// Uniformly scale the Renderable by the scalar scaleFactor.
        /// See glm::scale()
        void scale( float scaleFactor );

        /// Translate the Renderable by the given vector.
        /// See glm::translate()
        void translate( const glm::vec3& x );

        /// Translate the Renderable by given components.
        /// See glm::translate()
        void translate( float x, float y, float z );

        /// Returns the current translation of the renderable.
        glm::vec3 getTranslation( void );

        /// Rotate the Renderable around axis by angleInDegrees.
        /// See glm::rotate()
        void rotate( float angleInDegrees, const glm::vec3& axis );
        
        /// Align this Renderable's z-axis with the given dir vector.
        void alignZAxisWithVector( const glm::vec3& dir );

        /// Returns the material used for renderPassName
        /// See setMaterialForPassName()
        ConstMaterialPtr getMaterialForPassName(
            const RenderPassName& renderPassName ) const;
        /// Returns the material used for renderPassName
        /// See setMaterialForPassName()
        MaterialPtr getMaterialForPassName(
            const RenderPassName& renderPassName ) ;
        
        /// Specifies that this Renderable should be rendered using
        /// material in the given RenderPass.  This will override the
        /// RenderPass's default material.
        /// The Material's shader must be loaded so vertex positions
        /// can be set.
        void setMaterialForPassName( const RenderPassName& renderPassName,
                                     MaterialPtr material );

        friend std::ostream& operator<<( std::ostream& out, 
            ConstRenderablePtr renderable );
        friend  std::ostream& operator<<( std::ostream& out, 
            RenderablePtr renderable );

    protected:
        Renderable( const RenderableName& name );
        virtual ~Renderable();

        /// Ties the shader's "in" variables to the channels of this
        /// Renderable's data, e.g, "in vec3 v_position" in the shader
        /// needs to point to offset 0, stride 6.
        /// Need be done only after reloading shader or re-assigning
        /// Renderable to shader, or after changing the vertex type
        /// used by the Renderable
        /// Must be called for each shader that uses this Renderable
        virtual void attachShaderAttributes( GLuint shaderIndex ) = 0;
        

        /// Debugging label for this Renderable.
        RenderableName m_name;

        /// If true, this material will not use a RenderPass's default
        /// material, instead requiring an entry in m_materials to be 
        /// Rendered in the RenderPass.
        bool m_requiresExplicitMaterial;

        /// Stores which material should be used to render this
        /// Renderable in a given RenderPass.  Note that only
        /// one material per pass is currently supported.
        std::map< const RenderPassName, MaterialPtr > m_materials;

        /// The 4x4 matrix of the current transform of the Renderable.
        glm::mat4 m_objectTransform;
    };


} // end namespace spark
#endif

