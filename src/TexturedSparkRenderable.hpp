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

class TexturedSparkRenderable : public Renderable, public Updatable
{
public:
    TexturedSparkRenderable( LSparkPtr spark, TextureManagerPtr tm, ShaderManagerPtr sm  );
    virtual ~TexturedSparkRenderable() {}
    
    virtual void render( void ) const;
    virtual void update( float dt );
    virtual void loadTextures();
    virtual void setTextureState( PerspectivePtr renderContext );
private:
    static Eigen::Vector3f upOffset( const Eigen::Vector3f& pos, 
                                     const Eigen::Vector3f& parentPos,
                                     const Eigen::Vector3f& camPos,
                                     float halfAspectRatio );

    MeshPtr m_mesh;
    LSparkPtr m_spark;

    std::string m_fragmentShaderFilename;
    std::string m_vertexShaderFilename;
};
typedef std::shared_ptr< TexturedSparkRenderable > TexturedSparkRenderablePtr;



#endif
