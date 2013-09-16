//
//  PointSparkRenderable.hpp
//  sparks
//
//  Created by Brian Allen on 3/19/13.
//
//

#ifndef sparks_PointSparkRenderable_hpp
#define sparks_PointSparkRenderable_hpp

#include "Spark.hpp"

#include "DBMSpark.hpp"
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
    struct PointSparkVertex
    {
        GLfloat m_position[4];
        GLfloat m_color[4];
    };

    /// Renders a spark as a series of GL_POINTS using glBegin/glEnd
    /// Note that using this class violates the OpenGL 3.2 core profile,
    /// and use should be restricted to debugging only.
    class PointSparkRenderable
    : public Renderable,
      public Updateable
    {
    public:
        PointSparkRenderable( DBMSparkPtr spark,
                              TextureManagerPtr tm,
                              ShaderManagerPtr sm );
        virtual ~PointSparkRenderable() {}
    
        virtual void render( const RenderCommand& ) const override;
        virtual void update( double dt ) override;
        virtual void loadTextures() {}
        virtual void loadShaders() {}
    private:
        DBMSparkPtr m_spark;
        std::vector< PointSparkVertex > m_pointData;
    };
    typedef spark::shared_ptr< PointSparkRenderable > PointSparkRenderablePtr;
} // end namespace spark
#endif
