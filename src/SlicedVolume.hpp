#ifndef SLICED_VOLUME_HPP
#define SLICED_VOLUME_HPP

#include "SoftTestDeclarations.hpp"

#include "Mesh.hpp"
#include "VolumeData.hpp"

namespace spark
{
    /// Encapsulates the rendering of a VolumeData object using many 
    /// textured slices.  Relies on hardware support for 3D textures.
    /// TODO - support creating planes aligned with the eye-vector; currently 
    ///        create slices only makes axis-aligned slices.
    class SlicedVolume : public Renderable, public Updatable
    {
    public:
        SlicedVolume( TextureManagerPtr tm, ShaderManagerPtr sm, 
                     unsigned int sliceCount, VolumeDataPtr data );
        virtual ~SlicedVolume() {}
        void attachVolumeData( VolumeDataPtr data ) { m_volumeData = data; }
        virtual void render( void ) const override;
        virtual void update( float dt ) override;
        virtual void attachShaderAttributes( GLuint shaderIndex ) override;
    private:
        MeshPtr m_mesh;
        VolumeDataPtr m_volumeData;
        GLuint m_dataTextureId;
        TextureManagerPtr m_textureManager;
        TextureName m_textureName;
    };
} // end namespace spark

#endif
