//
//  TexturedSparkRenderer.hpp
//  sparks
//
//  Created by Brian Allen on 3/20/13.
//
//

#ifndef sparks_TexturedSparkRenderer_hpp
#define sparks_TexturedSparkRenderer_hpp

#include "Mesh.hpp"
#include "LSpark.hpp"
#include "Renderable.hpp"
#include "Projection.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <memory>
namespace spark
{
    /// Renders the component spark with a textured set of triangles.
    /// Responsible for rendering details such as the width of the stroke
    /// and how Spark intensities affect drawing.
    class TexturedSparkRenderable
    : public Renderable,
      public Updateable
    {
    public:
        TexturedSparkRenderable( LSparkPtr spark );
        virtual ~TexturedSparkRenderable() {}

        /// Move the spark to the new given position and reset the
        /// intensity to starting levels.
        void reseat( const glm::vec3& fromPos, const glm::vec3& toPos,
                     float a_intensity,
                     float a_scale,
                     int   a_depth,
                     float a_forkProb );
        
        virtual void render( const RenderCommand& rc ) const override;
        virtual void attachShaderAttributes( GLuint shaderIndex ) override;
        virtual void update( double dt ) override;

        void setViewProjection( ConstProjectionPtr aCamera );
    private:
        static Eigen::Vector3f upOffset( const Eigen::Vector3f& pos, 
                                         const Eigen::Vector3f& parentPos,
                                         const Eigen::Vector3f& camDir,
                                         float halfAspectRatio );

        MeshPtr m_mesh;
        LSparkPtr m_spark;
        ConstProjectionPtr m_camera;
    };
    typedef spark::shared_ptr< TexturedSparkRenderable > TexturedSparkRenderablePtr;

    
}
#endif
