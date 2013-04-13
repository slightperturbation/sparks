#include "Mesh.hpp"
#include "Utilities.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>



Mesh::Mesh(const char* vertexShaderFilepath,
           const char* fragmentShaderFilepath )
: m_vertexArrayObjectId(-1),
  m_vertexBufferId(-1),
  m_elementBufferId(-1),
  m_shaderProgramIndex(0),
  m_modelTransform(),
  m_vertexShaderFilepath( vertexShaderFilepath ),
  m_fragmentShaderFilepath( fragmentShaderFilepath )
{
    LOG_INFO(g_log) << "\tCreating vertex arrays... ";
    GL_CHECK( glGenVertexArrays( 1, &(m_vertexArrayObjectId) ) );
    if( GL_INVALID_VALUE == m_vertexArrayObjectId )
    { 
        LOG_INFO(g_log) << "------- ERROR in VAO creation ------ \n"; throw; 
    }
    LOG_INFO(g_log) << "done.\n";
        
    LOG_INFO(g_log) << "\tCreating array buffer... ";
    GL_CHECK( glGenBuffers( 1, &(m_vertexBufferId) ) );
    LOG_INFO(g_log) << "done.\n";
    
    LOG_INFO(g_log) << "\tCreating element buffer... ";
    GL_CHECK( glGenBuffers( 1, &(m_elementBufferId) ) ); // Generate 1 buffer
    LOG_INFO(g_log) << "done.\n";
}

Mesh::~Mesh()
{
    GL_CHECK( glDeleteProgram( m_shaderProgramIndex ) );
    m_shaderProgramIndex = 0;
    GL_CHECK( glDeleteBuffers( 1, &m_vertexBufferId ) );
    GL_CHECK( glDeleteBuffers( 1, &m_elementBufferId ) );
    GL_CHECK( glDeleteVertexArrays( 1, &m_vertexArrayObjectId ) );
}

void Mesh::update( float dt )
{
    // An example of updating the mesh dynamically  (assuming no topological changes)
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId ) );
    MeshVertex* mutableVerts;
    GL_CHECK( mutableVerts = (MeshVertex*) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE ) );
    if( mutableVerts )
    {
        // number of verts is same size as m_vertexData
        for( size_t i = 0; i < m_vertexData.size(); ++i )
        {
            mutableVerts[i].m_position[2] *= 1.001; // scale up the z
            mutableVerts[i].m_diffuseColor[0] += 0.001; // make it more red
            for( size_t gb = 1; gb < 3; gb++ )
            {
                mutableVerts[i].m_diffuseColor[gb] = std::max( 0.0f, mutableVerts[i].m_diffuseColor[gb] - 0.001f );
            }
        }
        GLint retVal; 
        GL_CHECK( retVal = glUnmapBuffer( GL_ARRAY_BUFFER ) );
        if( retVal == GL_FALSE )
        {
            LOG_INFO(g_log) << "Error un-mapping vertex buffer.\n";
        }
    }
    else
    {
        GLenum errCode = glGetError();
        if( errCode == 0 ) LOG_INFO(g_log) << "NoError\n";
        if( errCode == GL_INVALID_ENUM  ) LOG_INFO(g_log) << "INVALID_ENUM\n";
        if( errCode == GL_OUT_OF_MEMORY ) LOG_INFO(g_log) << "OUT OF MEMORY\n";
        if( errCode == GL_INVALID_OPERATION ) LOG_INFO(g_log) << "INVALID_OPERATION\n";
    }
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
}

void Mesh::setupRenderState( void )
{
    GL_CHECK( glBindVertexArray( m_vertexArrayObjectId ) );
}

void Mesh::setupShaderState( PerspectivePtr renderContext )
{
    setShaderUniformMatrix( "projMat", renderContext->projectionMatrix() );
    setShaderUniformMatrix( "modelViewMat", renderContext->modelViewMatrix() );
    setShaderUniformVector( "g_worldLightPosition", renderContext->lightPosition() );
    setShaderUniformVector( "g_lightColor", renderContext->lightColor() );
}

void Mesh::setShaderUniformMatrix( const char* uniformName, const glm::mat4& mat)
{
    GLint uniId;
    GL_CHECK( uniId = glGetUniformLocation( m_shaderProgramIndex, uniformName ) );
    if( uniId == GL_INVALID_VALUE || uniId == GL_INVALID_OPERATION )
    {
        LOG_INFO(g_log) << "Renderable::render | Failed to find uniform shader attribute of name \""
            << uniformName << "\" in shader #" << m_shaderProgramIndex << "\n";
    }
    else
    {
        if( glewIsSupported( "ARB_separate_shader_objects" ) )
        {
            GL_CHECK( glProgramUniformMatrix4fv( m_shaderProgramIndex, uniId, 1, GL_FALSE, glm::value_ptr(mat) ) );
        }
        else
        {
            GLint curShader;
            GL_CHECK( glGetIntegerv( GL_CURRENT_PROGRAM, &curShader ) );
            GL_CHECK( glUseProgram( m_shaderProgramIndex ) );
            GL_CHECK( glUniformMatrix4fv( uniId, 1, GL_FALSE, glm::value_ptr(mat) ) );
            GL_CHECK( glUseProgram( curShader ) );
        }
    }
}

void Mesh::setShaderUniformVector( const char* uniformName, const glm::vec3& vec )
{
    GLint uniId;
    GL_CHECK( uniId = glGetUniformLocation( m_shaderProgramIndex, uniformName ) );
    if( uniId == GL_INVALID_VALUE || uniId == GL_INVALID_OPERATION )
    {
        LOG_INFO(g_log) << "Renderable::render | Failed to find uniform shader attribute of name \"" 
            <<  uniformName << "\" in shader #" << m_shaderProgramIndex << "\n";
    }
    else
    {        
        if( glewIsSupported( "ARB_separate_shader_objects" ) )
        {
            GL_CHECK( glProgramUniform3fv( m_shaderProgramIndex, uniId, 1, glm::value_ptr(vec) ) );
        }
        else
        {
            GLint curShader;
            GL_CHECK( glGetIntegerv( GL_CURRENT_PROGRAM, &curShader ) );
            GL_CHECK( glUseProgram( m_shaderProgramIndex ) );
            GL_CHECK( glUniform3fv( uniId, 1, glm::value_ptr(vec) ) );
            GL_CHECK( glUseProgram( curShader ) );
        }
    }
}

void Mesh::setShaderUniformInt( const char* uniformName, GLint val )
{
    GLint uniId;
    GL_CHECK( uniId = glGetUniformLocation( m_shaderProgramIndex, uniformName ) );
    if( uniId == GL_INVALID_VALUE || uniId == GL_INVALID_OPERATION )
    {
        LOG_INFO(g_log) << "Renderable::render | Failed to find uniform shader attribute of name \""
            << uniformName << "\" in shader #" << m_shaderProgramIndex << "\n";
    }
    else
    {
        if( glewIsSupported( "ARB_separate_shader_objects" ) )
        {
            GL_CHECK( glProgramUniform1i( m_shaderProgramIndex, uniId, val ) );
        }
        else
        {
            GLint curShader;
            GL_CHECK( glGetIntegerv( GL_CURRENT_PROGRAM, &curShader ) );
            GL_CHECK( glUseProgram( m_shaderProgramIndex ) );
            GL_CHECK( glUniform1i( uniId, val ) );
            GL_CHECK( glUseProgram( curShader ) );
        }
    }
}

void Mesh::teardownRenderState( void )
{
    //GL_CHECK( glUseProgram( 0 ) );
    //GL_CHECK( glBindVertexArray( 0 ) );
    //glBindBuffer( GL_ARRAY_BUFFER, 0 );  checkOpenGLErrors();
    //glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );  checkOpenGLErrors();
}

void Mesh::render( PerspectivePtr renderContext )
{
    setupRenderState();
    GL_CHECK( glUseProgram( m_shaderProgramIndex ) );
    setupShaderState( renderContext );

    // bind vertex array OBJECT (VAO)
    GL_CHECK( glBindVertexArray( m_vertexArrayObjectId ) );
    GL_CHECK( glDrawElements( GL_TRIANGLES,
                   ((GLsizei)m_vertexIndicies.size()),
                   GL_UNSIGNED_INT,
                   nullptr ) );  // start at the beginning
    //LOG_INFO(g_log) << "glDrawElements( " << m_vertexIndicies.size() << ");\n";

    teardownRenderState();
}

void Mesh::loadShaders()
{
    m_shaderProgramIndex = loadShaderFromFile( m_vertexShaderFilepath.c_str(),
                                               m_fragmentShaderFilepath.c_str() );
    
    // Acquire shader
    GL_CHECK( glUseProgram( m_shaderProgramIndex ) );
    MeshVertex::addVertexAttributes( m_attributes );
    attachShaderAttributes();
}

void Mesh::loadTextures()
{
}

void Mesh::clearGeometry( void )
{
    m_vertexData.clear();
    m_vertexIndicies.clear();
}


void Mesh::resizeVertexArray( size_t newSize )
{
    m_vertexData.resize( newSize );
}
void Mesh::setVertex( size_t i, const Eigen::Vector3f& a, 
    const Eigen::Vector2f& textureCoords, 
    const Eigen::Vector4f color, 
    const Eigen::Vector3f norm )
{
    if( i > m_vertexData.size()-1 )
    {
        LOG_INFO(g_log) << "Call to Mesh::setVertex( " << i << ", ... ) where "
            << i << " is larger than number of pre-allocated vertices (" 
            << m_vertexData.size() << ").\n";
        assert( false );
    }
    MeshVertex& v = m_vertexData[i];
    for( size_t i=0; i<3; ++i ) v.m_position[i] = a(i);
    v.m_position[3] = 0;
    v.m_texCoord[0] = textureCoords(0);    v.m_texCoord[1] = textureCoords(1);
    v.m_texCoord[2] = 0;
    for( size_t i=0; i<4; ++i ) v.m_diffuseColor[i] = color(i);
    for( size_t i=0; i<3; ++i ) v.m_normal[i] = norm(i);
}

size_t Mesh::addVertex( const Eigen::Vector3f& a, 
                        const Eigen::Vector2f& aCoord,
                        const Eigen::Vector4f aColor,
                        const Eigen::Vector3f aNorm )
{
    MeshVertex v;
    v.m_position[3] = 0;
    for( size_t i=0; i<3; ++i ) v.m_normal[i] = aNorm(i);
    for( size_t i=0; i<3; ++i ) v.m_position[i] = a(i);
    v.m_texCoord[0] = aCoord(0);    v.m_texCoord[1] = aCoord(1);
    v.m_texCoord[2] = 0;
    for( size_t i=0; i<4; ++i ) v.m_diffuseColor[i] = aColor(i);
    m_vertexData.push_back( v );
    return m_vertexData.size() - 1;
}

void Mesh::addTriangleByIndex( unsigned int a, unsigned int b, unsigned int c )
{
    m_vertexIndicies.push_back( a ); 
    m_vertexIndicies.push_back( b ); 
    m_vertexIndicies.push_back( c ); 
}

void Mesh::addQuad( const Eigen::Vector3f& a, const Eigen::Vector2f& aCoord, 
                    const Eigen::Vector3f& b, const Eigen::Vector2f& bCoord, 
                    const Eigen::Vector3f& c, const Eigen::Vector2f& cCoord, 
                    const Eigen::Vector3f& d, const Eigen::Vector2f& dCoord, 
                    const Eigen::Vector3f& norm )
{
    // 0,1 1,1
    //  c   d
    //  *---*
    //  |\  |
    //  + \ +  (begin->end)
    //  |  \|
    //  *---*
    //  a   b
    // 0,0 1,0
    //
    //  a  b  c  d
    // -4 -3 -2 -1
    //
    // Indexes (CCW): (abc) (cbd)
    
    // TODO v1.m_normal
    
    MeshVertex v;
    v.m_position[3] = 0;
    for( size_t i=0; i<3; ++i ) v.m_normal[i] = norm[i];
    for( size_t i=0; i<3; ++i ) v.m_position[i] = a(i);
    v.m_texCoord[0] = aCoord[0];    v.m_texCoord[1] = aCoord[1];
    v.m_texCoord[2] = 0;
    v.m_diffuseColor[0] = 1; v.m_diffuseColor[1] = 0; v.m_diffuseColor[2] = 0; v.m_diffuseColor[3] = 0;
    size_t aIdx = m_vertexData.size();
    m_vertexData.push_back( v );
    // b
    for( size_t i=0; i<3; ++i ) v.m_position[i] = b(i);
    v.m_texCoord[0] = bCoord[0];    v.m_texCoord[1] = bCoord[1];
    v.m_diffuseColor[0] = 0; v.m_diffuseColor[1] = 1; v.m_diffuseColor[2] = 0; v.m_diffuseColor[3] = 0;
    size_t bIdx = m_vertexData.size();
    m_vertexData.push_back( v );
    // c
    for( size_t i=0; i<3; ++i ) v.m_position[i] = c(i);
    v.m_texCoord[0] = cCoord[0];    v.m_texCoord[1] = cCoord[1];
    v.m_diffuseColor[0] = 0; v.m_diffuseColor[1] = 0; v.m_diffuseColor[2] = 1; v.m_diffuseColor[3] = 0;
    size_t cIdx = m_vertexData.size();
    m_vertexData.push_back( v );
    // d
    for( size_t i=0; i<3; ++i ) v.m_position[i] = d(i);
    v.m_texCoord[0] = dCoord[0];    v.m_texCoord[1] = dCoord[1];
    v.m_diffuseColor[0] = 1; v.m_diffuseColor[1] = 1; v.m_diffuseColor[2] = 1; v.m_diffuseColor[3] = 0;
    size_t dIdx = m_vertexData.size();
    m_vertexData.push_back( v );
    
    // Three vertices per triangle (*not* tri-strips)
    m_vertexIndicies.push_back( aIdx );
    m_vertexIndicies.push_back( bIdx );
    m_vertexIndicies.push_back( cIdx );
    
    m_vertexIndicies.push_back( cIdx );
    m_vertexIndicies.push_back( bIdx );
    m_vertexIndicies.push_back( dIdx );
}


void Mesh::unitCube()
{
    LOG_INFO(g_log) << "Creating new Unit Cube\n";
    float third = 1.0f/1.73205080757f;
    
    MeshVertex v;
    //0
    v.m_position[0]     =  0;     v.m_position[1]     =  1;     v.m_position[2]     =  0;
    v.m_texCoord[0]     =  0;     v.m_texCoord[1]     =  1;     v.m_texCoord[2]     =  0;
    v.m_normal[0]       = -third; v.m_normal[1]       =  third; v.m_normal[2]       = -third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  0.0;   v.m_diffuseColor[2] =  0.0;
    m_vertexData.push_back( v );
    //1
    v.m_position[0]     =  1;     v.m_position[1]     =  1;     v.m_position[2]     =  0;
    v.m_texCoord[0]     =  1;     v.m_texCoord[1]     =  1;     v.m_texCoord[2]     =  0;
    v.m_normal[0]       =  third; v.m_normal[1]       =  third; v.m_normal[2]       = -third;
    v.m_diffuseColor[0] =  0.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  0.0;
    m_vertexData.push_back( v );
    //2
    v.m_position[0]     =  1;     v.m_position[1]     =  0;     v.m_position[2]     =  0;
    v.m_texCoord[0]     =  1;     v.m_texCoord[1]     =  0;     v.m_texCoord[2]     =  0;
    v.m_normal[0]       =  third; v.m_normal[1]       = -third; v.m_normal[2]       = -third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  0.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    //3
    v.m_position[0]     =  0;     v.m_position[1]     =  0;     v.m_position[2]     =  0;
    v.m_texCoord[0]     =  0;     v.m_texCoord[1]     =  0;     v.m_texCoord[2]     =  0;
    v.m_normal[0]       = -third; v.m_normal[1]       = -third; v.m_normal[2]       = -third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    
    //4
    v.m_position[0]     =  0;     v.m_position[1]     =  1;     v.m_position[2]     =  1;
    v.m_texCoord[0]     =  0;     v.m_texCoord[1]     =  1;     v.m_texCoord[2]     =  1;
    v.m_normal[0]       = -third; v.m_normal[1]       =  third; v.m_normal[2]       =  third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    //5
    v.m_position[0]     =  1;     v.m_position[1]     =  1;     v.m_position[2]     =  1;
    v.m_texCoord[0]     =  1;     v.m_texCoord[1]     =  1;     v.m_texCoord[2]     =  1;
    v.m_normal[0]       =  third; v.m_normal[1]       =  third; v.m_normal[2]       =  third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    //6
    v.m_position[0]     =  1;     v.m_position[1]     =  0;     v.m_position[2]     =  1;
    v.m_texCoord[0]     =  1;     v.m_texCoord[1]     =  0;     v.m_texCoord[2]     =  1;
    v.m_normal[0]       =  third; v.m_normal[1]       = -third; v.m_normal[2]       =  third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    //7
    v.m_position[0]     =  0;     v.m_position[1]     =  0;     v.m_position[2]     =  1;
    v.m_texCoord[0]     =  0;     v.m_texCoord[1]     =  0;     v.m_texCoord[2]     =  1;
    v.m_normal[0]       = -third; v.m_normal[1]       = -third; v.m_normal[2]       =  third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    
    GLuint vertexIndicies[] = {
        2, 1, 0,
        2, 0, 3, // Bottom
        
        4, 5, 6,
        6, 7, 4, // Top
        
        0, 4, 3,
        3, 4, 7, //
        
        5, 0, 1,
        0, 5, 4, //
        
        7, 2, 3,
        2, 7, 6,
        
        5, 1, 6,
        6, 1, 2,
    };
    for( size_t i = 0; i < sizeof(vertexIndicies)/sizeof(GLuint); ++i )
    {
        m_vertexIndicies.push_back(vertexIndicies[i]);
    }
    bindDataToBuffers();
}

void Mesh::bindDataToBuffers( void )
{
    //int count = 0;
    //LOG_INFO(g_log) << "\n";
    //for( auto idxIter = m_vertexIndicies.begin(); idxIter != m_vertexIndicies.end(); ++idxIter )
    //{
    //    count++;
    //    unsigned int idx = *idxIter;
    //    LOG_INFO(g_log) << idx << "\t";
    //    for( int i=0;i<3;++i ) LOG_INFO(g_log) << m_vertexData[idx].m_position[i] << " \t ";
    //    LOG_INFO(g_log) << "\n";
    //    
    //    if( !(count % 3) ) LOG_INFO(g_log) << "----\n";
    //}
    //LOG_INFO(g_log) << "++++\n";
    GL_CHECK( glBindVertexArray( m_vertexArrayObjectId ) );
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId ) );
    GL_CHECK( glBufferDataFromVector( GL_ARRAY_BUFFER, m_vertexData, GL_STATIC_DRAW ) );
    GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_elementBufferId ) );
    GL_CHECK( glBufferDataFromVector( GL_ELEMENT_ARRAY_BUFFER, m_vertexIndicies, GL_STATIC_DRAW ) );
}

void Mesh::attachShaderAttributes()
{
    // Must bind VAO & VBO to set attributes
    LOG_INFO(g_log) << "Binding vertex array object: " << m_vertexArrayObjectId << "\n";
    GL_CHECK( glBindVertexArray( m_vertexArrayObjectId ) );
    GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId ) );

    LOG_INFO(g_log) << "\tInitializing: # of attributes = " << m_attributes.size() << "\n";
    //for( auto attrib : m_attributes )
    for( auto attribIter = m_attributes.begin(); attribIter != m_attributes.end(); ++attribIter )
    {
        auto attrib = *attribIter;
        LOG_INFO(g_log) << "\t\tdefining shader attribute \"" << attrib->m_name << "\"...\n";
        attrib->defineByNameInShader( m_shaderProgramIndex );
        LOG_INFO(g_log) << "\t\tenabling shader attribute \"" << attrib->m_name << "\"...\n";
        attrib->enableByNameInShader( m_shaderProgramIndex );
    }
    GL_CHECK( glBindVertexArray( 0 ) );
}

RenderablePtr Mesh::createBox( void )
{
    Mesh* box = new Mesh();
    if( !box ) return RenderablePtr(nullptr);
    box->unitCube();  // Build geometry and setup VAO
    box->loadShaders(); // load shaders and attach to VAO
    return RenderablePtr( box );
}
