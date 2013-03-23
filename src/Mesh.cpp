#include "Mesh.hpp"

#include <iostream>


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


//////////////////////////////////////////////////////////////////////////
// Utility Functions  -- TODO -- MOVE OUT OF MESH

/// 
void checkOpenGLErrors( void )
{
#ifdef DEBUG
    GLenum errCode;
    const GLubyte *errString;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        std::cerr << "OpenGL Error: " << errString << "\n";
    }
#endif
}


std::string readFileToString( const char* filename )
{
    using namespace std;
    ifstream srcFile(filename, std::ios::in );
    if( srcFile )
    {
        ostringstream contents;
        contents << srcFile.rdbuf();
        srcFile.close();
        return contents.str();
    }
    std::cerr << "---------------------------------------\nError.  Unable to read file \"" << filename << "\"\n---------------------------------------\n";
    throw(errno);
}

GLuint createShaderWithErrorHandling( GLuint shaderType, const std::string& shaderSource )
{
    //////////////////
    // Load the Shader with error handling
    GLuint shader = glCreateShader( shaderType );
    {
        const char* shaderSourceCStr = shaderSource.c_str();
        glShaderSource( shader, 1, &(shaderSourceCStr), NULL );  checkOpenGLErrors();
        glCompileShader( shader );  checkOpenGLErrors();
        GLint shaderStatus;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &shaderStatus );  checkOpenGLErrors();
        const int buffSize = 1024;
        char buff[buffSize+1];
        glGetShaderInfoLog( shader, buffSize, NULL, buff );  checkOpenGLErrors();
        if( strnlen(buff, buffSize) )
        {
            std::cerr << "Shader Compilation message:\n------------\n" 
                << buff << "\n------------\n";
        }
        if( shaderStatus != GL_TRUE )
        {
            std::cerr << "Failed to compile shader:\n-------------\n" 
                << shaderSource << "\n-------------\n";
            //throw "Error during shader compilation.\n";
        }
        else
        {
            std::cerr << "Shader loaded successfully.\n";
        }
    }
    return shader;
}

GLuint loadShaderFromFile( const char* vertexShaderFilepath, const char* fragmentShaderFilepath )
{
    // Read file
    std::string vertexShaderString = readFileToString( vertexShaderFilepath );
    GLuint vertexShader = createShaderWithErrorHandling( GL_VERTEX_SHADER, vertexShaderString );
    std::string fragmentShaderString = readFileToString( fragmentShaderFilepath );
    GLuint fragmentShader = createShaderWithErrorHandling( GL_FRAGMENT_SHADER, fragmentShaderString );

    GLuint shaderProgram = glCreateProgram();  checkOpenGLErrors();
    glAttachShader( shaderProgram, vertexShader );  checkOpenGLErrors();
    glAttachShader( shaderProgram, fragmentShader );  checkOpenGLErrors();
    glBindFragDataLocation( shaderProgram, 0, "outColor" );  checkOpenGLErrors();  // define the output for color buffer-0
    glLinkProgram( shaderProgram );  checkOpenGLErrors();
    return shaderProgram;
}
//////////////////////////////////////////////////////////////////////////



Mesh::Mesh(const char* vertexShaderFilepath,
           const char* fragmentShaderFilepath )
: m_vertexArrayObjectId(-1),
m_dataTextureId(0),
m_vertexBufferId(-1),
m_elementBufferId(-1),
m_shaderProgramIndex(-1),
m_modelTransform(),
m_vertexShaderFilepath( vertexShaderFilepath ),
m_fragmentShaderFilepath( fragmentShaderFilepath )
{
    loadShaders();
}

Mesh::~Mesh()
{
    glDeleteTextures( 1, &m_dataTextureId );
    glDeleteProgram( m_shaderProgramIndex );
    glDeleteBuffers( 1, &m_vertexBufferId );
    glDeleteBuffers( 1, &m_elementBufferId );
    glDeleteVertexArrays( 1, &m_vertexArrayObjectId );
}

void Mesh::update( float dt )
{
    // An example of updating the mesh dynamically  (assuming no topological changes)
    glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId );
    MeshVertex* mutableVerts = (MeshVertex*) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
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
        if( glUnmapBuffer( GL_ARRAY_BUFFER ) == GL_FALSE )
        {
            std::cerr << "Error un-mapping vertex buffer.\n";
        }
    }
    else
    {
        GLenum errCode = glGetError();
        if( errCode == 0 ) std::cerr << "NoError\n";
        if( errCode == GL_INVALID_ENUM  ) std::cerr << "INVALID_ENUM\n";
        if( errCode == GL_OUT_OF_MEMORY ) std::cerr << "OUT OF MEMORY\n";
        if( errCode == GL_INVALID_OPERATION ) std::cerr << "INVALID_OPERATION\n";
    }
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void Mesh::setupRenderState( void )
{
    glDisable( GL_DEPTH_TEST );
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    //        glBlendFunc( GL_ONE_MINUS_SRC_ALPHA, GL_ONE ); // need to update shader to premultiply alpha
}

void Mesh::setupShaderState( const RenderContext& renderContext )
{
    // SETUP UNIFORM SHADER DATA
    std::string modelViewProjName = "modelViewProj";  // Depends on the name in the shader program
    GLint mvpm = glGetUniformLocation( m_shaderProgramIndex, modelViewProjName.c_str() );  checkOpenGLErrors();
    if( mvpm == -1 )
    {
        std::cerr << "Renderable::render | Failed to find uniform shader attribute of name \""
        << modelViewProjName << "\" in shader #" << m_shaderProgramIndex << "\n";
    }
    else
    {
        glUniformMatrix4fv( mvpm, 1, GL_FALSE, glm::value_ptr( renderContext.getModelViewProjMatrix() ) );  checkOpenGLErrors();
        //std::cerr << "xform: " << glm::to_string(xform) << "\n";
    }
}

void Mesh::teardownRenderState( void )
{
    glUseProgram( 0 );  checkOpenGLErrors();
    glBindVertexArray( 0 );  checkOpenGLErrors();
    glBindBuffer( GL_ARRAY_BUFFER, 0 );  checkOpenGLErrors();
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );  checkOpenGLErrors();
}

void Mesh::render( const RenderContext& renderContext )
{
    setupRenderState();
    glUseProgram( m_shaderProgramIndex );  checkOpenGLErrors();
    // bind vertex array OBJECT (VAO)
    glBindVertexArray( m_vertexArrayObjectId );  checkOpenGLErrors(); 
    
    setupShaderState( renderContext );
    
    glDrawElements(GL_TRIANGLES,
                   ((GLsizei)m_vertexIndicies.size()),
                   GL_UNSIGNED_INT,
                   nullptr);  // start at the beginning
    checkOpenGLErrors();

    teardownRenderState();
}

void Mesh::loadShaders()
{
    m_shaderProgramIndex = loadShaderFromFile( m_vertexShaderFilepath.c_str(),
                                              m_fragmentShaderFilepath.c_str() );
}
void Mesh::clearGeometry( void )
{
    m_vertexData.clear();
    m_vertexIndicies.clear();
}

void Mesh::addQuad( const Eigen::Vector3f& a,
                    const Eigen::Vector3f& b,
                    const Eigen::Vector3f& c,
                    const Eigen::Vector3f& d,
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
    for( int i=0; i<3; ++i ) v.m_normal[i] = norm[i];
    for( int i=0; i<3; ++i ) v.m_position[i] = a(i);
    v.m_texCoord[0] = 0;    v.m_texCoord[1] = 0;
    v.m_texCoord[2] = 0;
    size_t aIdx = m_vertexData.size();
    m_vertexData.push_back( v );
    // b
    for( int i=0; i<3; ++i ) v.m_position[i] = b(i);
    v.m_texCoord[0] = 1;    v.m_texCoord[1] = 0;
    size_t bIdx = m_vertexData.size();
    m_vertexData.push_back( v );
    // c
    for( int i=0; i<3; ++i ) v.m_position[i] = c(i);
    v.m_texCoord[0] = 0;    v.m_texCoord[1] = 1;
    size_t cIdx = m_vertexData.size();
    m_vertexData.push_back( v );
    // d
    for( int i=0; i<3; ++i ) v.m_position[i] = d(i);
    v.m_texCoord[0] = 1;    v.m_texCoord[1] = 1;
    size_t dIdx = m_vertexData.size();
    m_vertexData.push_back( v );
    
    
    m_vertexIndicies.push_back( aIdx );
    m_vertexIndicies.push_back( bIdx );
    m_vertexIndicies.push_back( cIdx );
    
    m_vertexIndicies.push_back( cIdx );
    m_vertexIndicies.push_back( bIdx );
    m_vertexIndicies.push_back( dIdx );
}

void Mesh::unitCube()
{
    checkOpenGLErrors();
    std::cerr << "Creating new Unit Cube\n";
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
    
    std::cerr << "\tCreating vertex arrays... ";
    glGenVertexArrays( 1, &(m_vertexArrayObjectId) );  checkOpenGLErrors();
    if( GL_INVALID_VALUE == m_vertexArrayObjectId ) { std::cerr << "------- ERROR in VAO creation ------ \n"; throw; }
    glBindVertexArray( m_vertexArrayObjectId );  checkOpenGLErrors();
    std::cerr << "done.\n";
    
    std::cerr << "\tCreating array buffer... ";
    glGenBuffers( 1, &(m_vertexBufferId) );  checkOpenGLErrors();
    glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId );  checkOpenGLErrors();
    glBufferDataFromVector( GL_ARRAY_BUFFER, m_vertexData, GL_STATIC_DRAW );  checkOpenGLErrors();
    std::cerr << "done.\n";
    
    std::cerr << "\tCreating element buffer... ";
    glGenBuffers( 1, &(m_elementBufferId) ); // Generate 1 buffer
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_elementBufferId );  checkOpenGLErrors();
    glBufferDataFromVector( GL_ELEMENT_ARRAY_BUFFER, m_vertexIndicies, GL_STATIC_DRAW );  checkOpenGLErrors();
    std::cerr << "done.\n";
    
    std::cerr << "\tCreating shader program... ";
    loadShaders();
    std::cerr << "done.\n";
    
    MeshVertex::addShaderAttributes( m_attributes );
    
    // ATTACH TO SHADER
    std::cerr << "\tInitializing: # of attributes = " << m_attributes.size() << "\n";
    glUseProgram( m_shaderProgramIndex );  checkOpenGLErrors();
    //for( auto attrib : m_attributes )
    for( auto attribIter = m_attributes.begin(); attribIter != m_attributes.end(); ++attribIter )
    {
        auto attrib = *attribIter;
        std::cerr << "\t\tenabling shader attribute \"" << attrib->m_name << "\"...\n";
        attrib->enableByNameInShader( m_shaderProgramIndex );
        std::cerr << "\t\tdefining shader attribute \"" << attrib->m_name << "\"...\n";
        attrib->defineByNameInShader( m_shaderProgramIndex );
    }
    
    glBindVertexArray( 0 );  checkOpenGLErrors();
    glBindBuffer( GL_ARRAY_BUFFER, 0 );  checkOpenGLErrors();
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );  checkOpenGLErrors();
    std::cerr << "Unit Cube created.\n";
}

RenderablePtr Mesh::createBox( void )
{
    Mesh* box = new Mesh();
    if( !box ) return RenderablePtr(nullptr);
    box->unitCube();
    return RenderablePtr( box );
}
