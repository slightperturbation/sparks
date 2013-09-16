#ifndef SLICED_VOLUME_HPP
#define SLICED_VOLUME_HPP

#include "Spark.hpp"
#include "Updateable.hpp"
#include "Mesh.hpp"
#include "VolumeData.hpp"

namespace spark
{
    /// Encapsulates the rendering of a VolumeData object using many 
    /// textured slices.  Relies on hardware support for 3D textures.
    class SlicedVolume : public Renderable, public Updateable
    {
    public:
        SlicedVolume( TextureManagerPtr tm, ShaderManagerPtr sm,
                     const RenderPassName& renderPassName,
                     unsigned int sliceCount, VolumeDataPtr data );
        virtual ~SlicedVolume() {}
        void attachVolumeData( VolumeDataPtr data ) { m_volumeData = data; }

        // From Renderable
        virtual void render( const RenderCommand& rc ) const override;
        // From Updateable
        virtual void update( double dt ) override;

        virtual void attachShaderAttributes( GLuint shaderIndex ) override;
        void setCameraDirection( const glm::vec3& dir );
    private:
        MeshPtr m_mesh;
        VolumeDataPtr m_volumeData;
        TextureManagerPtr m_textureManager;
        TextureName m_textureName;
        glm::vec3 m_cameraDir;
    };
    typedef spark::shared_ptr< SlicedVolume > SlicedVolumePtr;
} // end namespace spark

#endif
