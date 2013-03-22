#ifndef RAY_CAST_VOLUME_HPP
#define RAY_CAST_VOLUME_HPP


#include "Mesh.hpp"
#include "VolumeData.hpp"

#include <vector>

/// Handles rendering of a data volume using a lighted ray-cast algorithm.
class RayCastVolume : public Mesh
{
public:
    enum ScalarDataSource { NoneScalar, DensityScalar, VorticityMagnitudeScalar };
    enum VectorDataSource { NoneVector, VelocityVector, VorticityVector, VorticityForceVector };

    RayCastVolume(VolumeDataPtr data);
    virtual ~RayCastVolume() {}
    
    //////////////////////////////////////////////////////////////////////////
    // Mesh methods
    virtual void setupRenderState( void );
    virtual void setupShaderState( const RenderContext& renderContex );
    virtual void teardownRenderState( void );
    //////////////////////////////////////////////////////////////////////////
    
    /// Shader Parameters
    void setLightSamples( int num ) { m_numLightSamples = num; }
    void setVolumeSamples( int num ) { m_numVolumeSamples = num; }
 
    void attachVolumeData( VolumeDataPtr data ) { m_volumeData = data; }
    void selectScalarDataSource( ScalarDataSource dataSource ) { m_scalarDataSource = dataSource; }
    void selectVectorDataSource( VectorDataSource dataSource ) { m_vectorDataSource = dataSource; }

    /// Give the Volume Data to the graphics card for rendering
    void update( float dt )
    {
        loadTextureData();
    }

protected:
    /// Push new density data up to graphics card
    void loadTextureData( void )
    {
        checkOpenGLErrors();

        /// Associate the scalarTexture3d with GL_TEXTURE0
        GLuint textureID = glGetUniformLocation( m_shaderProgramIndex, "fluidData3d" );  checkOpenGLErrors();
        if( textureID == GL_INVALID_VALUE || textureID == GL_INVALID_OPERATION )
        {
            std::cerr << "Warning:  fluidData3d not found in shader.\n";
            return;
        }
        
        GLuint scalarTextureEnableID = glGetUniformLocation( m_shaderProgramIndex, "scalarTextureEnabled" );  checkOpenGLErrors();
        if( scalarTextureEnableID == GL_INVALID_OPERATION || scalarTextureEnableID == GL_INVALID_VALUE )
        {
            std::cerr << "Warning:  scalarTextureEnabled not found in shader.\n";
            return;
        }
        
        GLuint vectorTextureEnableID = glGetUniformLocation( m_shaderProgramIndex, "vectorTextureEnabled" );  checkOpenGLErrors();
        if( vectorTextureEnableID == GL_INVALID_OPERATION || vectorTextureEnableID == GL_INVALID_VALUE )
        {
            std::cerr << "Warning:  vectorTextureEnable not found in shader.\n";
            return;
        }
        
        m_dataTextureRGBA.resize( 4 * m_volumeData->dimX() * m_volumeData->dimY() * m_volumeData->dimZ() );

        const float* scalarDataSourcePtr = NULL;
        const float* vectorDataSourcePtr[3] = {NULL,NULL,NULL};
        switch( m_scalarDataSource )
        {
        case DensityScalar: 
            scalarDataSourcePtr = m_volumeData->getDensityData();
            break;
        case VorticityMagnitudeScalar:
            scalarDataSourcePtr = m_volumeData->getVorticityMagnitudeData();
            break;
        }
        float scalarTextureEnableValue = (scalarDataSourcePtr ? 1.0 : 0.0);
        glUniform1f( scalarTextureEnableID, scalarTextureEnableValue );  checkOpenGLErrors();
        switch( m_vectorDataSource )
        {
        case VelocityVector:
            m_volumeData->getVelocityData( vectorDataSourcePtr[0], vectorDataSourcePtr[1], vectorDataSourcePtr[2] );
            break;
        case VorticityVector:
            m_volumeData->getVorticityData( vectorDataSourcePtr[0], vectorDataSourcePtr[1], vectorDataSourcePtr[2] );
            break;
        case VorticityForceVector:
            m_volumeData->getVorticityForceData( vectorDataSourcePtr[0], vectorDataSourcePtr[1], vectorDataSourcePtr[2] );
            break;
        }
        float vectorTextureEnableValue = (vectorDataSourcePtr[0] ? 1.0 : 0.0 ) ;
        glUniform1f( vectorTextureEnableID, vectorTextureEnableValue );  checkOpenGLErrors();

        glActiveTexture( GL_TEXTURE0 );  checkOpenGLErrors();
        glBindTexture( GL_TEXTURE_3D, m_dataTextureId );  checkOpenGLErrors();

        size_t cells = m_dataTextureRGBA.size()/4;
        for( size_t i = 0; i<cells; ++i )
        {
            if( vectorDataSourcePtr[0] )
            {
                m_dataTextureRGBA[4*i+0] = vectorDataSourcePtr[0][i];
                m_dataTextureRGBA[4*i+1] = vectorDataSourcePtr[1][i];
                m_dataTextureRGBA[4*i+2] = vectorDataSourcePtr[2][i];
            }
            if( scalarDataSourcePtr )
            {
                m_dataTextureRGBA[4*i+3] = scalarDataSourcePtr[i];
            }
        }

        glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA32F,
            m_volumeData->dimX(), 
            m_volumeData->dimY(), 
            m_volumeData->dimZ(), 
            0, GL_RGBA, GL_FLOAT, 
            &(m_dataTextureRGBA[0]) );  checkOpenGLErrors();

        //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );  checkOpenGLErrors();
        glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );  checkOpenGLErrors();
        glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );  checkOpenGLErrors();
        float borderColor[] = {0,0,0,0};
        glTexParameterfv( GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor );  checkOpenGLErrors();

        glGenerateMipmap( GL_TEXTURE_3D );  checkOpenGLErrors(); // required for interpolation above
        //std::cout << "Loading 3d texture.\n";
    }

   
private:
    VolumeDataPtr m_volumeData;
    ScalarDataSource m_scalarDataSource;
    VectorDataSource m_vectorDataSource;
    
    std::vector<float> m_dataTextureRGBA;

    // Shading Parameters
    int m_numLightSamples;
    int m_numVolumeSamples;
};

#endif // RAY_CAST_VOLUME_HPP
