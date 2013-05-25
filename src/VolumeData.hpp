
#ifndef VOLUME_DATA_HPP
#define VOLUME_DATA_HPP

#include "SoftTestDeclarations.hpp"

#include <memory>

namespace spark
{
    /// Abstract superclass for 3D volume data governed by a density field
    class VolumeData
    {
    public:
        virtual ~VolumeData() {}
        /// Simulate the fluid flow: add sources, diffuse, advect
        virtual void update( float dt ) = 0;

        /// Returns the size of the fluid in number of voxels on x dimension
        virtual size_t dimX( void ) const = 0;
        /// Returns the size of the fluid in number of voxels on y dimension
        virtual size_t dimY( void ) const = 0;
        /// Returns the size of the fluid in number of voxels on z dimension
        virtual size_t dimZ( void ) const = 0;
        /// Returns a raw pointer to the density scalar field
        virtual const float* const getDensityData() const = 0;
        virtual const float* const getVorticityMagnitudeData() const = 0;
        virtual void getVelocityData( const float*& outVelX, const float*& outVelY, const float*& outVelZ ) const = 0;
        virtual void getVorticityData( const float*& outVorticityX, const float*& outVorticityY, const float*& outVorticityZ ) const = 0;
        virtual void getVorticityForceData( const float*& outVorticityForceX, const float*& outVorticityForceY, const float*& outVorticityForceZ ) const = 0;
        /// Returns the amount of light absorption per unit density
        virtual float absorption( void ) const = 0;
    };
    typedef std::shared_ptr< VolumeData > VolumeDataPtr;
}

#endif
