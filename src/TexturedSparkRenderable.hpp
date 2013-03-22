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
#include "RenderContext.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <memory>

class TexturedSparkRenderable : public Renderable
{
public:
    TexturedSparkRenderable( LSparkPtr spark );
    virtual ~TexturedSparkRenderable() {}
    
    virtual void render( const RenderContext& renderContext );
    virtual void update( float dt );
    virtual void loadTextures();
    virtual void loadShaders();
    virtual void setTextureState( const RenderContext& renderContext );
private:
    MeshPtr m_mesh;
    LSparkPtr m_spark;
    GLuint m_textureId;
    GLuint m_textureShaderUnit;

    std::string m_fragmentShaderFilename;
    std::string m_vertexShaderFilename;
};
typedef std::shared_ptr< TexturedSparkRenderable > TexturedSparkRenderablePtr;



#endif
