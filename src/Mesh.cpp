#include "Mesh.hpp"
#include "Material.hpp"
#include "Utilities.hpp"

#include <assimp/Importer.hpp>
#include <assimp/PostProcess.h>
#include <assimp/Scene.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <algorithm>

spark::Mesh
::Mesh( void )
: Renderable( "Mesh" ),
  m_vertexArrayObjectId(-1),
  m_vertexBufferId(-1),
  m_elementBufferId(-1),
  m_modelTransform()
{
    LOG_DEBUG(g_log) << "\tCreating vertex arrays... ";
    GL_CHECK( glGenVertexArrays( 1, &(m_vertexArrayObjectId) ) );
    if( GL_INVALID_VALUE == m_vertexArrayObjectId )
    { 
        LOG_DEBUG(g_log) << "------- ERROR in VAO creation ------ \n"; throw; 
    }
    LOG_DEBUG(g_log) << "done.\n";
        
    LOG_DEBUG(g_log) << "\tCreating array buffer... ";
    GL_CHECK( glGenBuffers( 1, &(m_vertexBufferId) ) );
    LOG_DEBUG(g_log) << "done.\n";
    
    LOG_DEBUG(g_log) << "\tCreating element buffer... ";
    GL_CHECK( glGenBuffers( 1, &(m_elementBufferId) ) ); // Generate 1 buffer
    LOG_DEBUG(g_log) << "done.\n";
}

spark::Mesh
::~Mesh()
{
    GL_CHECK( glDeleteBuffers( 1, &m_vertexBufferId ) );
    GL_CHECK( glDeleteBuffers( 1, &m_elementBufferId ) );
    GL_CHECK( glDeleteVertexArrays( 1, &m_vertexArrayObjectId ) );
    LOG_TRACE(g_log) << "Mesh \"" << name() << "\" destroyed.";
}

//void 
//spark::Mesh
//::update( double dt )
//{
//    return;
//
//    // An example of updating the mesh dynamically  (assuming no topological changes)
//    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId ) );
//    MeshVertex* mutableVerts;
//    GL_CHECK( mutableVerts = (MeshVertex*) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE ) );
//    if( mutableVerts )
//    {
//        // number of verts is same size as m_vertexData
//        for( size_t i = 0; i < m_vertexData.size(); ++i )
//        {
//            mutableVerts[i].m_position[2] *= 1.001; // scale up the z
//            mutableVerts[i].m_diffuseColor[0] += 0.001; // make it more red
//            for( size_t gb = 1; gb < 3; gb++ )
//            {
//                mutableVerts[i].m_diffuseColor[gb] = std::max( 0.0f, mutableVerts[i].m_diffuseColor[gb] - 0.001f );
//            }
//        }
//        GLint retVal; 
//        GL_CHECK( retVal = glUnmapBuffer( GL_ARRAY_BUFFER ) );
//        if( retVal == GL_FALSE )
//        {
//            LOG_DEBUG(g_log) << "Error un-mapping vertex buffer.\n";
//        }
//    }
//    else
//    {
//        GLenum errCode = glGetError();
//        if( errCode == 0 ) LOG_DEBUG(g_log) << "NoError\n";
//        if( errCode == GL_INVALID_ENUM  ) LOG_DEBUG(g_log) << "INVALID_ENUM\n";
//        if( errCode == GL_OUT_OF_MEMORY ) LOG_DEBUG(g_log) << "OUT OF MEMORY\n";
//        if( errCode == GL_INVALID_OPERATION ) LOG_DEBUG(g_log) << "INVALID_OPERATION\n";
//    }
//    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
//}

void
spark::Mesh
::render( const RenderCommand& rc ) const
{
    // bind vertex array OBJECT (VAO)
    GL_CHECK( glBindVertexArray( m_vertexArrayObjectId ) );

    //GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId ) );
    GL_CHECK( glDrawElements( GL_TRIANGLES,
                   ((GLsizei)m_vertexIndicies.size()),
                   GL_UNSIGNED_INT,
                   nullptr ) );  // start at the beginning
    LOG_TRACE(g_log) << "Mesh \"" << name() << "\"  glDrawElements( " << m_vertexIndicies.size() << " );\n";
    GL_CHECK( glBindVertexArray( 0 ) );
}

void
spark::Mesh
::clearGeometry( void )
{
    m_vertexData.clear();
    m_vertexIndicies.clear();
}

void 
spark::Mesh
::resizeVertexArray( size_t newSize )
{
    m_vertexData.resize( newSize );
}

void 
spark::Mesh
::setVertex( size_t i, 
             const Eigen::Vector3f& a, 
             const Eigen::Vector2f& textureCoords, 
             const Eigen::Vector4f& color, 
             const Eigen::Vector3f& norm )
{
    if( i > m_vertexData.size()-1 )
    {
        LOG_DEBUG(g_log) << "Call to Mesh::setVertex( " << i << ", ... ) where "
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

size_t 
spark::Mesh
::addVertex( const Eigen::Vector3f& a, 
             const Eigen::Vector2f& aCoord,
             const Eigen::Vector4f& aColor,
             const Eigen::Vector3f& aNorm )
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

size_t 
spark::Mesh
::addVertex( const MeshVertex& v )
{
    m_vertexData.push_back( v );
    return m_vertexData.size() - 1;
}

void 
spark::Mesh
::addTriangleByIndex( unsigned int a, unsigned int b, unsigned int c )
{
    m_vertexIndicies.push_back( a ); 
    m_vertexIndicies.push_back( b ); 
    m_vertexIndicies.push_back( c ); 
}

void
spark::Mesh
::addQuad( const glm::vec3& a, const glm::vec2& aCoord, 
    const glm::vec3& b, const glm::vec2& bCoord, 
    const glm::vec3& c, const glm::vec2& cCoord, 
    const glm::vec3& d, const glm::vec2& dCoord, 
    const glm::vec3& norm )
{
    addQuad(Eigen::Vector3f( a.x, a.y, a.z ),  Eigen::Vector2f( aCoord.x, aCoord.y ), 
            Eigen::Vector3f( b.x, b.y, b.z ),  Eigen::Vector2f( bCoord.x, bCoord.y ), 
            Eigen::Vector3f( c.x, c.y, c.z ),  Eigen::Vector2f( cCoord.x, cCoord.y ), 
            Eigen::Vector3f( d.x, d.y, d.z ),  Eigen::Vector2f( dCoord.x, dCoord.y ),
            Eigen::Vector3f( norm.x, norm.y, norm.z ) );
}

void
spark::Mesh
::addQuad( const Eigen::Vector3f& a, const Eigen::Vector2f& aCoord, 
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

    bindDataToBuffers();
}

void 
spark::Mesh
::unitCube( void )
{
   cube( 1.0f );
}

void 
spark::Mesh
::cube( float scale )
{
    LOG_DEBUG(g_log) << "Creating new Unit Cube\n";
    float third = 1.0f/1.73205080757f;
    
    MeshVertex v;
    //0
    v.m_position[0]     =  0;     v.m_position[1]     =  scale; v.m_position[2]     =  0;
    v.m_texCoord[0]     =  0;     v.m_texCoord[1]     =  1;     v.m_texCoord[2]     =  0;
    v.m_normal[0]       = -third; v.m_normal[1]       =  third; v.m_normal[2]       = -third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  0.0;   v.m_diffuseColor[2] =  0.0;
    m_vertexData.push_back( v );
    //1
    v.m_position[0]     =  scale; v.m_position[1]     =  scale; v.m_position[2]     =  0;
    v.m_texCoord[0]     =  1;     v.m_texCoord[1]     =  1;     v.m_texCoord[2]     =  0;
    v.m_normal[0]       =  third; v.m_normal[1]       =  third; v.m_normal[2]       = -third;
    v.m_diffuseColor[0] =  0.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  0.0;
    m_vertexData.push_back( v );
    //2
    v.m_position[0]     =  scale; v.m_position[1]     =  0;     v.m_position[2]     =  0;
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
    v.m_position[0]     =  0;     v.m_position[1]     =  scale; v.m_position[2]     =  scale;
    v.m_texCoord[0]     =  0;     v.m_texCoord[1]     =  1;     v.m_texCoord[2]     =  1;
    v.m_normal[0]       = -third; v.m_normal[1]       =  third; v.m_normal[2]       =  third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    //5
    v.m_position[0]     =  scale; v.m_position[1]     =  scale; v.m_position[2]     =  scale;
    v.m_texCoord[0]     =  1;     v.m_texCoord[1]     =  1;     v.m_texCoord[2]     =  1;
    v.m_normal[0]       =  third; v.m_normal[1]       =  third; v.m_normal[2]       =  third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    //6
    v.m_position[0]     =  scale; v.m_position[1]     =  0;     v.m_position[2]     =  scale;
    v.m_texCoord[0]     =  1;     v.m_texCoord[1]     =  0;     v.m_texCoord[2]     =  1;
    v.m_normal[0]       =  third; v.m_normal[1]       = -third; v.m_normal[2]       =  third;
    v.m_diffuseColor[0] =  1.0;   v.m_diffuseColor[1] =  1.0;   v.m_diffuseColor[2] =  1.0;
    m_vertexData.push_back( v );
    //7
    v.m_position[0]     =  0;     v.m_position[1]     =  0;     v.m_position[2]     =  scale;
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

void
spark::Mesh
::plane( const glm::vec3& center,
         const glm::vec2& scale,
         const glm::ivec2& subdivisions )
{
    glm::vec3 basePos = center - glm::vec3( 0.5f*scale.x, 0.5f*scale.y, 0.0f );
    float stepx = scale[0] / subdivisions[0];
    float stepz = scale[1] / subdivisions[1];
    for( int x = 0; x < subdivisions[0]; ++x )
    {
        float prevtexx = (float)(x-1)/subdivisions[0];
        float texx = (float)x/subdivisions[0];
        for( int z = 1; z < subdivisions[1]; ++z )
        {
            float prevtexz = (float)(z-1)/subdivisions[1];
            float texz = (float)z/subdivisions[1];
            // 0,1 1,1
            //  c   d
            //  *---*
            //  |\  |
            //  + \ +  (begin->end)
            //  |  \|
            //  *---*
            //  a   b
            // 0,0 1,0
            //0
            glm::vec3 a = basePos;
            a[0]     += x*stepx;
            a[2]     += (z-1)*stepz;
            glm::vec2 aCoord( texx, prevtexz );

            //1
            glm::vec3 b = basePos;
            b[0]     += (x-1)*stepx;
            b[2]     += (z-1)*stepz;
            glm::vec2 bCoord( prevtexx, prevtexz );

            //2
            glm::vec3 c = basePos;
            c[0]     += x*stepx;
            c[2]     += z*stepz;
            glm::vec2 cCoord( texx, texz );

            //3
            glm::vec3 d = basePos;
            d[0]     += (x-1)*stepx;
            d[2]     += z*stepz;
            glm::vec2 dCoord( prevtexx, texz );

            glm::vec3 normal( 0, 1, 0 );
            
            addQuad( a, aCoord,
                     b, bCoord,
                     c, cCoord,
                     d, dCoord,
                     normal );
        }
    }
}


void
spark::Mesh
::bindDataToBuffers( void )
{
    const bool explicitLogging = false;
    if( explicitLogging )
    {
        int count = 0;
        LOG_TRACE(g_log) << "++++\n";
        for( auto idxIter = m_vertexIndicies.begin(); idxIter != m_vertexIndicies.end(); ++idxIter )
        {
            count++;
            size_t idx = *idxIter;
            LOG_TRACE(g_log) << idx ;
            for( int i=0;i<3;++i ) LOG_TRACE(g_log) << "\t" 
                << m_vertexData[idx].m_position[i];
            if( !(count % 3) ) LOG_TRACE(g_log) << "----";
        }
        LOG_TRACE(g_log) << "++++\n";
    }
    GL_CHECK( glBindVertexArray( m_vertexArrayObjectId ) );
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId ) );
    GL_CHECK( glBufferDataFromVector( GL_ARRAY_BUFFER, m_vertexData, GL_STATIC_DRAW ) );
    GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_elementBufferId ) );
    GL_CHECK( glBufferDataFromVector( GL_ELEMENT_ARRAY_BUFFER, m_vertexIndicies, GL_STATIC_DRAW ) );

    GL_CHECK( glBindVertexArray( 0 ) );
    //GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
    //GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );
}

void 
spark::Mesh
::attachShaderAttributes( GLuint aShaderProgramIndex )
{
    // TODO -- if mesh always uses a particular vertex type, could replace 
    // with simple opengl calls
    // glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId );
    // glVertexAttribPointer( ... );
    // glEnableVertexAttribArray( position_loc_in_shader );

    MeshVertex::addVertexAttributes( m_attributes );

    LOG_TRACE(g_log) << "Binding vertex array object: " << m_vertexArrayObjectId ;
    // Must bind VAO & VBO to set attributes
    GL_CHECK( glBindVertexArray( m_vertexArrayObjectId ) );
    GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId ) );

    LOG_TRACE(g_log) << "\tInitializing: # of attributes = " << m_attributes.size();
    for( auto attribIter = m_attributes.begin(); attribIter != m_attributes.end(); ++attribIter )
    {
        auto attrib = *attribIter;
        LOG_TRACE(g_log) << "\t\tdefining shader attribute \"" << attrib->m_name << "\".";
        attrib->defineByNameInShader( aShaderProgramIndex );
        LOG_TRACE(g_log) << "\t\tenabling shader attribute \"" << attrib->m_name << "\".";
        attrib->enableByNameInShader( aShaderProgramIndex );
    }
    GL_CHECK( glBindVertexArray( 0 ) );
}

spark::RenderablePtr 
spark::Mesh
::createBox( TextureManagerPtr tm, 
             ShaderManagerPtr sm, 
             const RenderPassName& renderPassName  )
{
    Mesh* box = new Mesh();
    if( !box ) return RenderablePtr();
    box->unitCube();  // Build geometry and setup VAO
    // Build materials for needed passes
    ShaderName colorShaderName = box->name() + "_Box_ColorShader";
    sm->loadShaderFromFiles( colorShaderName, 
        "color.vert",
        "color.frag" );
    ShaderInstancePtr colorShader( new ShaderInstance( colorShaderName, sm ) );
    MaterialPtr colorMaterial( new Material( tm, colorShader ) );
    box->setMaterialForPassName( renderPassName, colorMaterial );
    return RenderablePtr( box );
}

spark::MeshPtr 
spark::Mesh
::createMeshFromAiMesh( const aiMesh* meshNode, float scale )
{
    assert( meshNode );
    
    MeshPtr mesh( new Mesh );
    mesh->name( meshNode->mName.C_Str() );

    // Load vertices
    MeshVertex v;
    std::map< size_t, size_t > aiVertexIndex_to_meshVertexIndex;
    for( size_t vertexIndex=0; vertexIndex < meshNode->mNumVertices; ++vertexIndex )
    {
        for( size_t i=0; i<3; ++i )
        {
            v.m_position[i] = meshNode->mVertices[vertexIndex][i];
            v.m_normal[i] = meshNode->mNormals[vertexIndex][i];
        }
        // Only get the first channel of texture coords for now
        if( meshNode->HasTextureCoords(0) )
        {
            for( size_t i=0; i< std::min<unsigned int>( 3, meshNode->mNumUVComponents[0] ); ++i )
            {
                v.m_texCoord[i] = meshNode->mTextureCoords[0][vertexIndex][i];
            }
        }
//        for( size_t channelIndex=0;
//             channelIndex < meshNode->GetNumUVChannels();
//             ++channelIndex )
//        {
//            for( size_t i=0; i< std::min( 3, meshNode->mNumUVComponents[0] ); ++i )
//            {
//                v.m_texCoords[channelIndex][i] = meshNode->mTextureCoords[0][vertexIndex][i]
//            }
//        }
        if( meshNode->HasVertexColors(0) )
        {
            for( size_t i=0; i<3; ++i )
            {
                v.m_diffuseColor[i] = meshNode->mColors[0][vertexIndex][i];
            }
        }
        aiVertexIndex_to_meshVertexIndex[vertexIndex] = mesh->m_vertexData.size();
        mesh->m_vertexData.push_back( v );
    }

    for( size_t faceIdx=0; faceIdx < meshNode->mNumFaces; ++faceIdx )
    {
        const aiFace face = meshNode->mFaces[faceIdx];
        if( face.mNumIndices != 3 )
        {
            LOG_WARN(g_log) << "Ignoring face with " 
                << face.mNumIndices << " indices while loading mesh node \""
                << mesh->name() << "\" because can only handle triangles.";
            continue;
        }
        for( size_t i=0; i < face.mNumIndices; ++i )
        {
            mesh->m_vertexIndicies
              .push_back( aiVertexIndex_to_meshVertexIndex[ face.mIndices[i] ] );
        }
    }
    mesh->bindDataToBuffers();
    return mesh;
}







