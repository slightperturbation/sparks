#ifndef SLICED_VOLUME_HPP
#define SLICED_VOLUME_HPP

#include "SoftTestDeclarations.hpp"

#include "Mesh.hpp"
#include "VolumeData.hpp"

/// Encapsulates the rendering of a VolumeData object using many 
/// textured slices.  Relies on hardware support for 3D textures.
/// TODO - support creating planes aligned with the eye-vector; currently 
///        create slices only makes axis-aligned slices.
class SlicedVolume : public Mesh
{
public:
    SlicedVolume( TextureManagerPtr tm, ShaderManagerPtr sm, 
                  unsigned int sliceCount, VolumeDataPtr data )
        : Mesh(), 
          m_volumeData( data ),
          m_dataTextureId( 0 )
    {
        ShaderName colorShaderName = name() + "_SlicedVolume_ColorShader";
        sm->loadShaderFromFiles( colorShaderName, 
//            "volumeVertexShader.glsl",
//            "volumeFragmentShader.glsl" );
"colorVertexShader.glsl",
"colorFragmentShader.glsl" );

        ShaderPtr colorShader( new Shader( colorShaderName, sm ) );
        MaterialPtr colorMaterial( new Material( colorShader ) );
        setMaterialForPassName( g_colorRenderPassName, colorMaterial );

        const size_t zSteps = sliceCount;
        const float zMin = -0.5;
        const float zMax =  0.5;
        for( size_t step = 0; step < zSteps; ++step )
        {
            float z = zMin + ((float)step) * (zMax - zMin)/((float)zSteps);
            float t = ((float)step)/(zSteps-1.0); // 0 to 1
            //LOG_DEBUG(g_log) << "slice at z = " << z << "\n";

            MeshVertex v;
            v.m_position[0] = -0.5;     v.m_position[1] =  0.5;     v.m_position[2] = z;
            v.m_normal[0]   = 0;        v.m_normal[1] = 0;          v.m_normal[2] = 1;
            v.m_diffuseColor[0] = 0.7;  v.m_diffuseColor[1] = 0.7;  v.m_diffuseColor[2] = 0.75;
            v.m_texCoord[0] = 0;        v.m_texCoord[1] = 1;        v.m_texCoord[2] = t;
            const size_t upperLeft = m_vertexData.size();
            m_vertexData.push_back( v );

            v.m_position[0] =  0.5;     v.m_position[1] =  0.5;     v.m_position[2] = z;
            v.m_texCoord[0] = 1;        v.m_texCoord[1] = 1;        v.m_texCoord[2] = t;
            size_t upperRight = m_vertexData.size();
            m_vertexData.push_back( v );

            v.m_position[0] =  0.5;     v.m_position[1] = -0.5;     v.m_position[2] = z;
            v.m_texCoord[0] = 1;        v.m_texCoord[1] = 0;        v.m_texCoord[2] = t;
            const size_t lowerRight = m_vertexData.size();
            m_vertexData.push_back( v );

            v.m_position[0] = -0.5;     v.m_position[1] = -0.5;     v.m_position[2] = z;
            v.m_texCoord[0] = 0;        v.m_texCoord[1] = 0;        v.m_texCoord[2] = t;
            const size_t lowerLeft = m_vertexData.size();
            m_vertexData.push_back( v );

            // TRIANGLES (NOT STRIP)
            //0, 1, 2
            m_vertexIndicies.push_back( upperLeft ); // 0
            m_vertexIndicies.push_back( upperRight ); // 1
            m_vertexIndicies.push_back( lowerRight ); // 2
            //2, 3, 0
            m_vertexIndicies.push_back( upperLeft ); // 0
            m_vertexIndicies.push_back( lowerRight ); // 2
            m_vertexIndicies.push_back( lowerLeft ); // 3
        }

        LOG_DEBUG(g_log) << "\tCreating vertex arrays... ";
        GL_CHECK( glGenVertexArrays( 1, &(m_vertexArrayObjectId) ) );
        GL_CHECK( glBindVertexArray( m_vertexArrayObjectId ) );
        LOG_DEBUG(g_log) << "done.\n";

        LOG_DEBUG(g_log) << "\tCreating array buffer... ";
        GL_CHECK( glGenBuffers( 1, &(m_vertexBufferId) ) );
        GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId ) );
        GL_CHECK( glBufferDataFromVector( GL_ARRAY_BUFFER, m_vertexData, GL_STATIC_DRAW ) );
        LOG_DEBUG(g_log) << "done.\n";

        LOG_DEBUG(g_log) << "\tCreating element buffer... ";
        GL_CHECK( glGenBuffers( 1, &(m_elementBufferId) ) ); 
        GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_elementBufferId ) );
        GL_CHECK( glBufferDataFromVector( GL_ELEMENT_ARRAY_BUFFER, m_vertexIndicies, GL_STATIC_DRAW ) );
        LOG_DEBUG(g_log) << "done.\n";

        GL_CHECK( glBindVertexArray( 0 ) );
        GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
        GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );
        LOG_DEBUG(g_log) << "Slices created.\n";
        load3DTextures();
        LOG_DEBUG(g_log) << "Textures loaded.\n";
    }
    virtual ~SlicedVolume() {}
    
    void attachVolumeData( VolumeDataPtr data ) { m_volumeData = data; }

    void update( float dt )
    {
        load3DTextures();
    }

    /// Push new density data up to graphics card
    void load3DTextures()
    {
        GL_CHECK( glBindTexture( GL_TEXTURE_3D, m_dataTextureId ) );
        //GL_CHECK( glTexImage3D( GL_TEXTURE_3D, 0, GL_R32F, m_N+2, m_N+2, m_N+2, 0, GL_RED, GL_FLOAT, m_density ) );
        GL_CHECK( glTexImage3D( GL_TEXTURE_3D, 0, GL_R32F, 
            m_volumeData->dimX(), 
            m_volumeData->dimY(), 
            m_volumeData->dimZ(), 
            0, GL_RED, GL_FLOAT, 
            m_volumeData->getDensityData() ) );

        //glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA16F, m_N+2, m_N+2, m_N+2, 0, GL_RGBA, GL_FLOAT, m_density );//m_velU ); //
        GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );
        GL_CHECK( glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE ) );
        GL_CHECK( glGenerateMipmap( GL_TEXTURE_3D ) );
        //LOG_DEBUG(g_log) << "Loaded 3d texture.\n";
    }

private:
    VolumeDataPtr m_volumeData;
    GLuint m_dataTextureId;
};


#endif
