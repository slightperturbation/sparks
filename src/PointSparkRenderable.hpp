//
//  PointSparkRenderable.hpp
//  sparks
//
//  Created by Brian Allen on 3/19/13.
//
//

#ifndef sparks_PointSparkRenderable_hpp
#define sparks_PointSparkRenderable_hpp

#include "SoftTestDeclarations.hpp"

#include "Spark.hpp"
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

struct PointSparkVertex
{
    GLfloat m_position[4];
    GLfloat m_color[4];
};

class PointSparkRenderable : public Renderable
{
public:
    PointSparkRenderable( SparkPtr spark,
                          TextureManagerPtr tm,
                          ShaderManagerPtr sm );
    virtual ~PointSparkRenderable() {}
    
    virtual void render( void ) const;
    virtual void update( float dt );
    virtual void loadTextures() {}
    virtual void loadShaders() {}
private:
    SparkPtr m_spark;
    std::vector< PointSparkVertex > m_pointData;
};
typedef std::shared_ptr< PointSparkRenderable > PointSparkRenderablePtr;


#endif
