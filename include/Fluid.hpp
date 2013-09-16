#ifndef SOFTIE_FLUID_HPP
#define SOFTIE_FLUID_HPP

#include "VelocityFieldInterface.hpp"

#include "Mesh.hpp"
#include "VolumeData.hpp"

#include <ostream>

namespace spark
{

    /// Implementation of Jos Stam's Stable Fluids
    class Fluid : public VolumeData
    {
    public:
        /// Create and initialize a fluid volume, with all-equal dimensions.
        Fluid( int size = 4 );
        virtual ~Fluid();

        void setSize( int size );
        int getSize( void );

        /// Save the fluid to a file.
        /// \see loadFromFile
        void saveToFile( const char* filename );

        /// Read the state of the fluid from a file.
        /// \see saveToFile
        void loadFromFile( const char* filename );

        //////////////////////////////////////////////////////////////////////////
        // Methods from VolumeData
        virtual void update( double dt ) override;
        virtual size_t dimX( void ) const { return m_N+2; }
        virtual size_t dimY( void ) const { return m_N+2; }
        virtual size_t dimZ( void ) const { return m_N+2; }
        /// Provides the "stable" density data (i.e., not currently updated)
        virtual const float* const getDensityData() const { return m_density; }
        virtual const float* const getVorticityMagnitudeData() const { return m_vorticityMagnitude; }
        virtual void getVelocityData( const float*& outVelX, const float*& outVelY, const float*& outVelZ ) const
        {
            outVelX = m_velU; outVelY = m_velV; outVelZ = m_velW;
        }
        virtual void getVorticityData( const float*& outVorticityX, const float*& outVorticityY, const float*& outVorticityZ ) const
        {
            outVorticityX = m_vorticityU; outVorticityY = m_vorticityV; outVorticityZ = m_vorticityW;
        }
        virtual void getVorticityForceData( const float*& outVorticityForceX, const float*& outVorticityForceY, const float*& outVorticityForceZ ) const
        {
            outVorticityForceX = m_vorticityForceU; outVorticityForceY = m_vorticityForceV; outVorticityForceZ = m_vorticityForceW;
        }
        virtual float absorption( void ) const { return m_absorption; }
        //////////////////////////////////////////////////////////////////////////

        void setViscosity( float visc ) { m_visc = visc; }
        void setDiffusion( float diff ) { m_diff = diff; }
        void setVorticity( float vort ) { m_vorticityConfinementFactor = vort; }
        void setAbsorption( float absorption ) { m_absorption = absorption; }
        void setGravityFactor( float gravX, float gravY, float gravZ ) 
        { 
            m_gravityFactor[0] = gravX; 
            m_gravityFactor[1] = gravY; 
            m_gravityFactor[2] = gravZ; 
        }

        /// Used to update and/or clear the source of density
        const float* getDensitySource( void ) { return m_density_source; }
        const float* getTempSource( void ) { return m_temp_source; }
    
        /// Used to update and/or clear the source of velocity
        const float* getVelUSource( void );
        const float* getVelVSource( void );
        const float* getVelWSource( void );

        /// Specify the number of inner-loop PDE solver iterations. 
        /// Roughly linear time taken by entire update() method.
        void setSolverIterations( unsigned int iterations ) { m_solverIterations = iterations; }

        /// Write Python scripts that instance arrays that hold per-update density slices at y=j
        std::ostream& writeYDensitySliceToPythonStream( std::ostream& out, const std::string& varName, unsigned int frame, size_t j ) const;
        /// Write Python scripts that instance arrays that hold per-update velocity field (3d) slices at y=j
        std::ostream& writeYVelocitySliceToPythonStream( std::ostream& out, const std::string& varName, unsigned int frame, size_t j, float dt ) const;

        /// resets the simulation to some arbitrary initial state
        void reset( void );

        void addBottomSource( void );
        void addSourceAtLocation( float x, float y );
    private:
        void reallocate( void );
        void init( void );
        void deleteData( void );
        void zeroData( void );
        void addDensitySources( float dt );
        void addVelocitySources( float dt );
        void addBoom( void );

        /// Compute the curl of a single component, with given (positive) offsets
        /// h should be the size of one voxel
        void curl( float* dest, 
            const float* a, const float* b, 
            int ax, int ay, int az, 
            int bx, int by, int bz,
            float h );

        void computeVorticityConfinment();

        /// Helmholtz projection into a mass-conserving field
        void project( void );

        /// Solve diffusion of field x_prev, writing new values to x
        void diffuse( int boundaryCondition, float* x, float* x_prev, float diff, float dt );

        /// Solve the divergence by Gauss-Seidel relaxation, writing new values to x
        void linearSolve( int boundaryCondition, float* x, float* x_prev, float a, float c );
    
        // Slow version of linearSolve is known correct, but not optimized
        void linearSolveSlow( int boundaryCondition, float* x, float* x_prev, float a, float c );

        // Advect a single dimension held in d[] (with previous timestep d_prev) by the velocity field vx,vy,vz over time dt
        // typically:
        //    vx = m_velU;
        //    vy = m_velV;
        //    vz = m_velW;
        void advect( int boundaryCondition, float* d, float* d_prev, float* vx, float* vy, float* vz, float dt );

        // Make the boundary same as the neighboring in-side cell, 
        // When dim!=0, ensure that dimension has zero total on the boundary wall
        // i.e., make the outer boundary the negative of the in-side cell.
        void enforceBoundary( int dim, float* x );
        // hacked version to remove bounds
        void enforceConcentrationBoundary( int dim, float* x );

        /// Master function that updates the velocity vector (3d) field, 
        /// including advection, diffusion and projection 
        /// onto mass-conserving field.
        void stepVelocity( float dt );

        /// Master function that updates the density scalar field, including
        /// advection and diffusion.
        void stepDensity( float dt );

        /// Returns the index into the parallel arrays (e.g., m_density, m_velU, etc.)
        /// for the given 3d coordinates
        size_t index( size_t i, size_t j, size_t k ) const
        {
            const size_t dim = m_N+2;
            return i + dim*(j + dim*k);
        }

        float& density( size_t i, size_t j, size_t k ) const
        {
            return m_density[ index(i,j,k) ];
        }

        void swapAdvectedFields()
        {
            float* tmp = m_density;
            m_density = m_density_prev;
            m_density_prev = tmp;

            tmp = m_temp;
            m_temp = m_temp_prev;
            m_temp_prev = tmp;
        }

        void swapVelocityField()
        {
            float* tmp = m_velU;
            m_velU = m_velU_prev;
            m_velU_prev = tmp;

            tmp = m_velV;
            m_velV = m_velV_prev;
            m_velV_prev = tmp;

            tmp = m_velW;
            m_velW = m_velW_prev;
            m_velW_prev = tmp;
        }

        size_t m_N; //< N does not include one-cell boundary at each side
        float* m_density; //< (N+2)^3 scalar field holding the density
        
        float* m_density_prev; //< previous time step's density field (scalar)
        // sources to be added with each update, needs to be zeroed if no longer adding
        float* m_density_source;

        float* m_velU;    //< (velU,velV,velW) is a (N+2)^3 vector field holding the velocity
        float* m_velV;
        float* m_velW;

        float* m_temp; ///< scalar field holding the temp at that voxel
        float* m_temp_prev; //< previous time step's temp field (scalar)
        float* m_temp_source;
        
        float* m_vorticityU; //< Curl of the velocity field, used for vorticity confinement
        float* m_vorticityV; //< referred to as omega in Fedkiw,Stam,Jensen
        float* m_vorticityW;

        float* m_vorticityMagnitude; //< Magnitude of the fluids "vorticity" (which is the curl of the velocity)

        float* m_vorticityForceU; //< holds the normalized direction of increased vorticity, N in FSJ temporarily inside computeVorticityConfinment()
        float* m_vorticityForceV; //< and holds the vorticity confinement force after the call to computeVorticityConfinment()
        float* m_vorticityForceW;

        float* m_velU_prev; //< previous time step's velocity field (UVW as a vector)
        float* m_velV_prev;
        float* m_velW_prev;

        float* m_velU_source;
        float* m_velV_source;
        float* m_velW_source;

        unsigned int m_solverIterations; //< number of iterations in inner loop of Gauss-Seidel relaxation
        size_t m_size;  //< total number of elements in main arrays (density and each elem of velocity)
        float m_visc;  //< viscosity, used in velocity diffusion
        float m_diff;  //< material density diffusion
        float* m_div;  //< temporary storage for divergence field (only used in projection)
        float* m_pressure; //< temporary storage for pressure field (only used in projection)
        float m_ambientTemp; //< the temp of the embedding medium that the fluid is in.  (Celsius)
        float m_tempFactor; //< tuning factor that converts a temperature difference to a buoyancy force.
        float m_gravityFactor[3]; //< tuning factor that converts particle density to gravitational force
        float m_vorticityConfinementFactor; //< tuning factor for the amount of vorticity to be restored
        float m_absorption; //< rate at which light is absorbed per unit density
    };
    typedef spark::shared_ptr< Fluid > FluidPtr;
} // end namespace spark
#endif
