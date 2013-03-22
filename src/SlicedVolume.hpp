#ifndef SLICED_VOLUME_HPP
#define SLICED_VOLUME_HPP

#include "Mesh.hpp"
#include "VolumeData.hpp"

/// Encapsulates the rendering of a VolumeData object using many 
/// textured slices.  Relies on hardware support for 3D textures.
/// TODO - support creating planes aligned with the eye-vector; currently 
///        create slices only makes axis-aligned slices.
class SlicedVolume : public Mesh
{
public:
    SlicedVolume( unsigned int sliceCount, VolumeDataPtr data )
        : m_volumeData( data )
    {
        const size_t zSteps = sliceCount;
        const float zMin = -0.5;
        const float zMax =  0.5;
        for( size_t step = 0; step < zSteps; ++step )
        {
            float z = zMin + ((float)step) * (zMax - zMin)/((float)zSteps);
            float t = ((float)step)/(zSteps-1.0); // 0 to 1
            //std::cerr << "slice at z = " << z << "\n";

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

        std::cerr << "\tCreating vertex arrays... ";
        glGenVertexArrays( 1, &(m_vertexArrayObjectId) );
        glBindVertexArray( m_vertexArrayObjectId );
        std::cerr << "done.\n";

        std::cerr << "\tCreating array buffer... ";
        glGenBuffers( 1, &(m_vertexBufferId) );
        glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId );
        glBufferDataFromVector( GL_ARRAY_BUFFER, m_vertexData, GL_STATIC_DRAW );
        std::cerr << "done.\n";

        std::cerr << "\tCreating element buffer... ";
        glGenBuffers( 1, &(m_elementBufferId) ); // Generate 1 buffer
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_elementBufferId );
        glBufferDataFromVector( GL_ELEMENT_ARRAY_BUFFER, m_vertexIndicies, GL_STATIC_DRAW );
        std::cerr << "done.\n";

        std::cerr << "\tCreating shader program... ";
        loadShaders();
        std::cerr << "done.\n";

        MeshVertex::addShaderAttributes( m_attributes );

        // ATTACH TO SHADER
        std::cerr << "\tInitializing: # of attributes = " << m_attributes.size() << "... ";
        glUseProgram( m_shaderProgramIndex );
        //for( auto attrib : box->m_attributes )
        for( auto attribIter = m_attributes.begin(); attribIter != m_attributes.end(); ++attribIter )
        {
            auto attrib = *attribIter;
            //std::cerr << "\t\tenabling shader attribute \"" << attrib->m_name << "\"... ";
            attrib->enableByNameInShader( m_shaderProgramIndex );
            //std::cerr << "\t\tdefining shader attribute \"" << attrib->m_name << "\"... ";
            attrib->defineByNameInShader( m_shaderProgramIndex );
        }
        std::cerr << "done.\n";
        glBindVertexArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        std::cerr << "Slices created.\n";
        load3DTextures();
        std::cerr << "Textures loaded.\n";
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
        glBindTexture( GL_TEXTURE_3D, m_dataTextureId );
        //glTexImage3D( GL_TEXTURE_3D, 0, GL_R32F, m_N+2, m_N+2, m_N+2, 0, GL_RED, GL_FLOAT, m_density );
        glTexImage3D( GL_TEXTURE_3D, 0, GL_R32F, 
            m_volumeData->dimX(), 
            m_volumeData->dimY(), 
            m_volumeData->dimZ(), 
            0, GL_RED, GL_FLOAT, 
            m_volumeData->getDensityData() );

        //glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA16F, m_N+2, m_N+2, m_N+2, 0, GL_RGBA, GL_FLOAT, m_density );//m_velU ); //
        glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );
        glGenerateMipmap( GL_TEXTURE_3D );
        //std::cout << "Loading 3d texture.\n";
    }

private:
    VolumeDataPtr m_volumeData;
};


#endif
