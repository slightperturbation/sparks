#ifndef RAY_CAST_VOLUME_HPP
#define RAY_CAST_VOLUME_HPP

#include "Spark.hpp"

#include "Renderable.hpp"
#include "Updateable.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "VolumeData.hpp"

#include <vector>
namespace spark
{
    /// Handles rendering of a data volume using a lighted ray-cast algorithm.
    class RayCastVolume
    : public Renderable,
      public Updateable
    {
    public:
        RayCastVolume( const RenderableName& aName,
                       TextureManagerPtr tm,
                       ShaderManagerPtr sm,
                       VolumeDataPtr data );
        virtual ~RayCastVolume() {}
        
        virtual void render( const RenderCommand& rc ) const override;
        virtual void attachShaderAttributes( GLuint shaderIndex ) override;
        virtual void update( double dt ) override;
        
        /// Shader Parameters
        void setLightSamples( int num )
        {
            m_material->setShaderUniform( "u_numSamples", num );
        }
        
        void setVolumeSamples( int num )
        {
            m_material->setShaderUniform( "u_numLightSamples", num );
        }
     
        void attachVolumeData( VolumeDataPtr data ) { m_volumeData = data; }

    protected:

//            glActiveTexture( GL_TEXTURE0 );  checkOpenGLErrors();
//            glBindTexture( GL_TEXTURE_3D, m_dataTextureId );  checkOpenGLErrors();
//
//            size_t cells = m_dataTextureRGBA.size()/4;
//            for( size_t i = 0; i<cells; ++i )
//            {
//                if( vectorDataSourcePtr[0] )
//                {
//                    m_dataTextureRGBA[4*i+0] = vectorDataSourcePtr[0][i];
//                    m_dataTextureRGBA[4*i+1] = vectorDataSourcePtr[1][i];
//                    m_dataTextureRGBA[4*i+2] = vectorDataSourcePtr[2][i];
//                }
//                if( scalarDataSourcePtr )
//                {
//                    m_dataTextureRGBA[4*i+3] = scalarDataSourcePtr[i];
//                }
//            }
//
//            glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA32F,
//                m_volumeData->dimX(), 
//                m_volumeData->dimY(), 
//                m_volumeData->dimZ(), 
//                0, GL_RGBA, GL_FLOAT, 
//                &(m_dataTextureRGBA[0]) );  checkOpenGLErrors();
//
            //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
            //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
//            glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );  checkOpenGLErrors();
//            glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );  checkOpenGLErrors();
//            glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );  checkOpenGLErrors();
//            float borderColor[] = {0,0,0,0};
//            glTexParameterfv( GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor );  checkOpenGLErrors();
//
//            glGenerateMipmap( GL_TEXTURE_3D );  checkOpenGLErrors(); // required for interpolation above
//            //std::cout << "Loading 3d texture.\n";
//        }

       
    private:
        MeshPtr m_mesh;
        VolumeDataPtr m_volumeData;
        TextureManagerPtr m_textureManager;
        TextureName m_textureName;
        MaterialPtr m_material;
    };
    typedef spark::shared_ptr< RayCastVolume > RayCastVolumePtr;
}
#endif // RAY_CAST_VOLUME_HPP
