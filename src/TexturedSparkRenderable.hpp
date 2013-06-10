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
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <memory>
namespace spark
{
    /// Renders the component spark with a textured set of triangles.
    class TexturedSparkRenderable : public Renderable, public Updatable
    {
    public:
        TexturedSparkRenderable( LSparkPtr spark );
        virtual ~TexturedSparkRenderable() {}
    
        // cam pos is used to orient the spark billboards toward the camera
        void setCameraPosition( const glm::vec3& cameraPos )
        { m_cameraPos = cameraPos; }
        virtual void render( void ) const override;
        virtual void attachShaderAttributes( GLuint shaderIndex ) override;
        virtual void update( float dt ) override;
    private:
        static Eigen::Vector3f upOffset( const Eigen::Vector3f& pos, 
                                         const Eigen::Vector3f& parentPos,
                                         const Eigen::Vector3f& camPos,
                                         float halfAspectRatio );

        MeshPtr m_mesh;
        LSparkPtr m_spark;
        glm::vec3 m_cameraPos;
    };
    typedef std::shared_ptr< TexturedSparkRenderable > TexturedSparkRenderablePtr;
}
#endif
