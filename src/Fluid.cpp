#include "Fluid.hpp"

#include "Utilities.hpp" //< for getTime() only

#include <limits>
#include <fstream>
#include <algorithm>
#include <cassert>

std::ostream&
spark::Fluid
::writeYDensitySliceToPythonStream(std::ostream& out,
                                   const std::string& varName,
                                   unsigned int frame,
                                   size_t j ) const
{
    out << varName << "[" << frame << "] = array( [";
    for( size_t i=0; i < m_N+2; ++i )
    {
        out << "[";
        for( size_t k=0; k < m_N+2; ++k )
        {
            out << m_density[index(i, j, k)];
            if( k != (m_N+1) ) out << ", ";
        }
        out << "]";
        if( i != (m_N+1) ) out << ", \n"; //new line optional
    }
    out << "], dtype=float32 )\n\n";
    return out;
}

std::ostream&
spark::Fluid
::writeYVelocitySliceToPythonStream(std::ostream& out,
                                    const std::string& varName,
                                    unsigned int frame,
                                    size_t j,
                                    float dt ) const
{
    out << varName << "[" << frame <<"] = array( [";
    for( size_t i=0; i < m_N+2; ++i )
    {
        out << "[";
        for( size_t k=0; k < m_N+2; ++k )
        {
            out << "["
                << dt * m_N * m_velU[index(i, j, k)] << ", "
                << dt * m_N * m_velV[index(i, j, k)] << ", "
                << dt * m_N * m_velW[index(i, j, k)] << "] ";
            if( k != (m_N+1) ) out << ", ";
        }
        out << "]";
        if( i != (m_N+1) ) out << ", \n"; //new line optional
    }
    out << "], dtype=float32 )\n\n";
    return out;
}

spark::Fluid
::Fluid( int size ) 
    :
    m_N( size - 2 ),
    m_density( NULL ), m_density_prev( NULL ), m_density_source( NULL ),
    m_velU( NULL ), m_velV( NULL ), m_velW( NULL ),
    m_temp( NULL ), m_temp_prev( NULL ), m_temp_source( NULL ),
    m_vorticityU( NULL ), m_vorticityV( NULL ), m_vorticityW( NULL ),
    m_vorticityMagnitude( NULL ),
    m_vorticityForceU( NULL ), m_vorticityForceV( NULL ), m_vorticityForceW( NULL ),
    m_velU_prev( NULL ), m_velV_prev( NULL ), m_velW_prev( NULL ),
    m_velU_source( NULL ), m_velV_source( NULL ), m_velW_source( NULL ),
    m_solverIterations( 20 ),
    m_size( size*size*size ),
    m_visc( 0.0000 ),
    m_diff( 0.000 ), // ????
    m_div( NULL ), m_pressure( NULL ),
    m_ambientTemp( 20.0f ),
    m_tempFactor( 1.0f ), 
    m_vorticityConfinementFactor( size * 60.0f ),
    m_absorption( 1.0 )
{
    m_gravityFactor[0] = 0;
    m_gravityFactor[1] = 0;
    m_gravityFactor[2] = 0.3f;
    // enforce a minimum size
    const int minSize = 8;
    if( size < minSize )
    {
        m_N = minSize;
        m_size = (m_N+2)*(m_N+2)*(m_N+2);
    }
    reallocate();
    init();
}

void 
spark::Fluid
::reallocate( void )
{
    deleteData();

    m_density = new float[m_size];
    m_density_prev = new float[m_size];
    m_density_source = new float[m_size];

    m_temp = new float[m_size];
    m_temp_prev = new float[m_size];
    m_temp_source = new float[m_size];

    m_velU = new float[m_size];
    m_velU_prev = new float[m_size];
    m_velU_source = new float[m_size];

    m_velV = new float[m_size];
    m_velV_prev = new float[m_size];
    m_velV_source = new float[m_size];

    m_velW = new float[m_size];
    m_velW_prev = new float[m_size];
    m_velW_source = new float[m_size];

    m_vorticityU = new float[m_size];
    m_vorticityV = new float[m_size];
    m_vorticityW = new float[m_size];

    m_vorticityMagnitude = new float[m_size];

    m_vorticityForceU = new float[m_size];
    m_vorticityForceV = new float[m_size];
    m_vorticityForceW = new float[m_size];

    m_pressure = new float[m_size];
    m_div = new float[m_size];
}

void
spark::Fluid
::deleteData( void )
{
    delete[] m_density;
    delete[] m_density_prev;
    delete[] m_temp;
    delete[] m_temp_prev;
    delete[] m_temp_source;
    delete[] m_density_source;
    delete[] m_velU;
    delete[] m_velU_prev;
    delete[] m_velU_source;
    delete[] m_velV;
    delete[] m_velV_prev;
    delete[] m_velV_source;
    delete[] m_velW;
    delete[] m_velW_prev;
    delete[] m_velW_source;
    delete[] m_vorticityU;
    delete[] m_vorticityV;
    delete[] m_vorticityW;
    delete[] m_vorticityMagnitude;
    delete[] m_vorticityForceU;
    delete[] m_vorticityForceV;
    delete[] m_vorticityForceW;
    delete[] m_pressure;
    delete[] m_div;
}

spark::Fluid
::~Fluid()
{
    deleteData();
}

void 
spark::Fluid
::init( void )
{
    zeroData();
    swapAdvectedFields();
    swapVelocityField();
    zeroData();
}

void
spark::Fluid
::reset( void )
{
    init();
}

void
spark::Fluid
::setSize( int size )
{
    m_N = size - 2;
    m_size = size*size*size;

    reallocate();
    init();
}

int
spark::Fluid
::getSize( void )
{
    return m_N + 2;
}

void
spark::Fluid
::zeroData( void )
{
    for( size_t i=0; i < m_size; ++i )
    {
        m_density[i] = 0.0f;
        m_temp[i] = m_ambientTemp;
        m_velU[i] = 0.0f;
        m_velV[i] = 0.0f;
        m_velW[i] = 0.0f;
        m_div[i] = 0.0f;
        m_pressure[i] = 0.0f;
        m_density_source[i] = 0.0f;
        m_temp_source[i] = 0.0f;
        m_velU_source[i] = 0.0f;
        m_velV_source[i] = 0.0f;
        m_velW_source[i] = 0.0f;
        m_vorticityU[i] = 0.0f;
        m_vorticityV[i] = 0.0f;
        m_vorticityW[i] = 0.0f;
        m_vorticityForceU[i] = 0.0f;
        m_vorticityForceV[i] = 0.0f;
        m_vorticityForceW[i] = 0.0f;
    }
}

/// Create a 4x4x1 area at the bottom with high temp and source
void
spark::Fluid
::addBottomSource( void )
{
    int cx  = m_N/2;
    int cy  = 0;//m_N/4;
    int cz  = m_N/2;
    float r = (float)m_N/12.0f;

    for( int x=1; x<m_N; ++x )
    {
        for( int y=1; y<m_N; ++y )
        {
            for( int z=1; z<m_N; ++z )
            {
                float d = std::sqrt( 
                    (x-(float)cx)*(x-(float)cx)
                    + (y-(float)cy)*(y-(float)cy)
                    + (z-(float)cz)*(z-(float)cz)
                    );
                if( d <= r )
                {
                    int i = index(x,y,z);
                    m_density[i] = 0.5f;
                    //m_density_source[i] = 10.0;
                    //m_temp[i] = 4.2f * m_ambientTemp;
                    //m_temp_source[i] = 1.8f * m_ambientTemp;
                }
            }
        }
    }
}


void
spark::Fluid
::addBoom( void )
{
    const float vu = 100;
    const float vv = 100;
    const float vw = 100;
    const float d = 10.5;

    const size_t cx = m_N/2;
    const size_t cy = m_N/2;
    const size_t cz = m_N/4;

    // Initialize middle
    m_velU[index(cx, cy, cz)] =   vu;
    m_velU[index(cx-1, cy, cz)] = vu;
    m_velU[index(cx+1, cy, cz)] = vu;
    m_velU[index(cx, cy-1, cz)] = vu;
    m_velU[index(cx, cy+1, cz)] = vu;
    m_velU[index(cx, cy, cz-1)] = vu;
    m_velU[index(cx, cy, cz+1)] = vu;

    m_velV[index(cx, cy, cz)] =   vv;
    m_velV[index(cx-1, cy, cz)] = vv;
    m_velV[index(cx+1, cy, cz)] = vv;
    m_velV[index(cx, cy-1, cz)] = vv;
    m_velV[index(cx, cy+1, cz)] = vv;
    m_velV[index(cx, cy, cz-1)] = vv;
    m_velV[index(cx, cy, cz+1)] = vv;

    m_velW[index(cx, cy, cz)] =   vw;
    m_velW[index(cx-1, cy, cz)] = vw;
    m_velW[index(cx+1, cy, cz)] = vw;
    m_velW[index(cx, cy-1, cz)] = vw;
    m_velW[index(cx, cy+1, cz)] = vw;
    m_velW[index(cx, cy, cz-1)] = vw;
    m_velW[index(cx, cy, cz+1)] = vw;

    //// for starters, but a little density at the middle
    m_density[index(m_N/2,     m_N/2,     m_N/2)] = d;
    m_density[index(m_N/2 + 1, m_N/2 + 1, m_N/2)] = d;
    m_density[index(m_N/2 + 1, m_N/2 - 1, m_N/2)] = d;
    m_density[index(m_N/2 - 1, m_N/2 + 1, m_N/2)] = d;
    m_density[index(m_N/2 - 1, m_N/2 - 1, m_N/2)] = d;
    m_density[index(m_N/2, m_N/2, m_N/2+1)] = d;
    m_density[index(m_N/2, m_N/2, m_N/2-1)] = d;

    m_temp[index(m_N/2,     m_N/2,     m_N/2)] = 1.8f * m_ambientTemp;
    m_temp[index(m_N/2 + 1, m_N/2 + 1, m_N/2)] = 1.8f * m_ambientTemp;
    m_temp[index(m_N/2 + 1, m_N/2 - 1, m_N/2)] = 1.8f * m_ambientTemp;
    m_temp[index(m_N/2 - 1, m_N/2 + 1, m_N/2)] = 1.8f * m_ambientTemp;
    m_temp[index(m_N/2 - 1, m_N/2 - 1, m_N/2)] = 1.8f * m_ambientTemp;
    m_temp[index(m_N/2, m_N/2, m_N/2+1)] = 1.8f * m_ambientTemp;
    m_temp[index(m_N/2, m_N/2, m_N/2-1)] = 1.8f * m_ambientTemp;
}

void
spark::Fluid
::saveToFile( const char* filename )
{
    using namespace std;
    ofstream out( filename, std::ios::ate );
    out.precision( numeric_limits< double >::max_digits10 );
    out << m_N << endl;
    assert( m_size == ((m_N+2)*(m_N+2)*(m_N+2)) );
    for( size_t i = 0; i < m_size; ++i )
    {
        out << scientific << m_density[i] << endl;
    }
}

void
spark::Fluid
::loadFromFile( const char* filename )
{
    using namespace std;
    ifstream infile( filename );
    infile >> m_N;
    m_size = (m_N+2)*(m_N+2)*(m_N+2);
    reallocate();
    init();
    for( size_t i = 0; i < m_size; ++i )
    {
        infile >> m_density[i];
        m_density_prev[i] = m_density[i];
    }
    LOG_DEBUG(g_log) << "Finished loading file with N=" << m_N 
                     << " and size=" << m_size << "\n";
}

void
spark::Fluid
::addDensitySources( float dt )
{
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                const size_t ind = index(i,j,k);

                // Explicit sources
                m_density[ind] += dt * m_density_source[ind];
                m_temp[ind]    += dt * m_temp_source[ind];
            }
        }
    }
}

void
spark::Fluid
::addVelocitySources( float dt )
{
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                const size_t ind = index(i,j,k);
                m_velU[ind]    += dt * m_velU_source[ind];
                m_velV[ind]    += dt * m_velV_source[ind];
                m_velW[ind]    += dt * m_velW_source[ind];

                // Buoyancy force
                // assume z = 0,0,1 so only affect w
                m_velW[ind]    += dt * m_tempFactor * ( m_temp[ind] - m_ambientTemp );

                // Gravity force
                // assume z = 0,0,1
                m_velU[ind]    -= dt * m_density[ind] * m_gravityFactor[0];
                m_velV[ind]    -= dt * m_density[ind] * m_gravityFactor[1];
                m_velW[ind]    -= dt * m_density[ind] * m_gravityFactor[2];

                // Vorticity confinement term
                m_velU[ind]    += dt * m_vorticityForceU[ind];
                m_velV[ind]    += dt * m_vorticityForceV[ind];
                m_velW[ind]    += dt * m_vorticityForceW[ind];
            }
        }
    }
}

void
spark::Fluid
::project( void )
{
    // compute divergence of velocity field
    float* div = m_div;//m_velV_prev;  
    float* p   = m_pressure; //m_velU_prev;  
    float* vx  = m_velU;
    float* vy  = m_velV;
    float* vz  = m_velW;
    float  a   = -0.5f/m_N;
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                div[index(i, j, k)] = 
                    a * (  vx[index(i+1, j,   k  )] - vx[index(i-1,j,  k  )]
                         + vy[index(i,   j+1, k  )] - vy[index(i,  j-1,k  )]
                         + vz[index(i,   j,   k+1)] - vz[index(i,  j,  k-1)] );
                p[index(i,j,k)] = 0;
            }
        }
    }

    enforceConcentrationBoundary( 0, div );
    enforceConcentrationBoundary( 0, p );

    // Gauss-Seidel solver of the pressure field
    linearSolve( 0, p, div, 1, 6 );

    // subtract the gradient of pressure field from velocity
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                vx[index(i,j,k)] -= 0.5f*m_N*( p[index(i+1, j,   k  )] - p[index(i-1, j,   k  )] );
                vy[index(i,j,k)] -= 0.5f*m_N*( p[index(i,   j+1, k  )] - p[index(i,   j-1, k  )] );
                vz[index(i,j,k)] -= 0.5f*m_N*( p[index(i,   j,   k+1)] - p[index(i,   j,   k-1)] );
            }
        }
    }
    enforceConcentrationBoundary( 1, vx );
    enforceConcentrationBoundary( 2, vy );
    enforceConcentrationBoundary( 3, vz );
}

void 
spark::Fluid
::diffuse( int boundaryCondition, 
           float* x, float* x_prev, 
           float diff, float dt )
{
    float a = dt * diff * m_N * m_N * m_N;
    linearSolve( boundaryCondition, x, x_prev, a, 1+6*a );
}

void
spark::Fluid
::linearSolveSlow( int boundaryCondition, 
                   float* x, float* x_prev, 
                   float a, float c )
{
    const float invC = 1.0 / c;
    //LOG_DEBUG(g_log) << "PRE linear Solve a=" << a << ", invc=" << invC << ",  middle voxel = " << x[index(m_N/2, m_N/2, m_N/2)] << " prev=" << x_prev[index(m_N/2, m_N/2, m_N/2)] << "\n";
    for ( size_t iter = 0 ; iter < m_solverIterations; ++iter ) 
    {
        for( size_t k = 1; k <= m_N; k++ )
        {
            for( size_t j = 1; j <= m_N; j++ )
            {
                for( size_t i = 1; i <= m_N; i++ )
                {
                    x[index(i, j, k)] = invC * ( x_prev[index(i, j, k)] 
                    + a*( x[index(i-1, j,   k  )] 
                        + x[index(i+1, j,   k  )] 
                        + x[index(i,   j-1, k  )]
                        + x[index(i,   j+1, k  )]
                        + x[index(i,   j,   k-1)] 
                        + x[index(i,   j,   k+1)] 
                      )
                    );
                }
            }
        }
        // Each iteration, enforce boundary
        enforceConcentrationBoundary( boundaryCondition, x );
    }
    //LOG_DEBUG(g_log) << "POST linear Solve a=" << a << ", invc=" << invC << ",  middle voxel = " << x[index(m_N/2, m_N/2, m_N/2)] << " prev=" << x_prev[index(m_N/2, m_N/2, m_N/2)] << "\n";
}

void 
spark::Fluid
::linearSolve( int boundaryCondition, 
               float* x, float* x_prev, 
               float a, float c )
{
    const int dim = m_N+2;
    const int dim2 = dim*dim;
    const float invC = 1.0 / c;
    size_t ind;
    for ( size_t iter = 0 ; iter < m_solverIterations; ++iter ) 
    {
        for( size_t k = 1; k <= m_N; k++ )
        {
            for( size_t j = 1; j <= m_N; j++ )
            {
                for( size_t i = 1; i <= m_N; i++ )
                {
                    ind = index(i, j, k);
                    x[ind] = invC * ( x_prev[ind] 
                    + a*( x[ind-1]  //index(i-1, j, k) 
                    + x[ind+1] // index(i+1, j,   k  )] 
                    + x[ind-dim]//index(i,   j-1, k  )]
                    + x[ind+dim]//index(i,   j+1, k  )]
                    + x[ind-dim2]// index(i,   j,   k-1)] 
                    + x[ind+dim2]//index(i,   j,   k+1)] 
                    )
                        );
                }
            }
        }
        // Each iteration, enforce boundary
        enforceConcentrationBoundary( boundaryCondition, x );
    }
}

void 
spark::Fluid
::curl( float* dest, 
        const float* a, const float* b, 
        int ax, int ay, int az, 
        int bx, int by, int bz,
        float h )
{
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                // TODO -- reduce # of calls to index
                dest[index(i, j, k)] = h * ( a[index(i+ax,j+ay,k+az)] - b[index(i+bx,j+by,k+bz)] );
            }
        }
    }
}

void 
spark::Fluid
::computeVorticityConfinment( void )
{
    if( m_vorticityConfinementFactor == 0.0 )
    {
        return;
    }
    float h = 2.0/(m_N+2); // width of two voxels
    // Compute curl of the velocity = known as the vorticity of the fluid (\omega)
    curl( m_vorticityU, m_velW, m_velV, 0,1,0, 0,0,1, h );
    curl( m_vorticityV, m_velU, m_velW, 0,0,1, 1,0,0, h );
    curl( m_vorticityW, m_velV, m_velU, 1,0,0, 0,1,0, h );

    // \omega = \nabla \cross u
    // \eta = \nabla | \omega |  -- gradient of the scalar field that is the magnitude of the vorticity
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                // Compute eta = | \omega |
                const int ind = index(i,j,k);
                m_vorticityMagnitude[ind] 
                = std::sqrt( m_vorticityU[ind]*m_vorticityU[ind] 
                           + m_vorticityV[ind]*m_vorticityV[ind]
                           + m_vorticityW[ind]*m_vorticityW[ind] );
            }
        }
    }
    // compute the gradient of the magnitude of the vorticity  
    // NOTE, m_vorticityForceUVW is used as a temporary var holding the gradient of the vorticity
    // \eta = \nabla | \omega |
    const int dx = index(1,0,0) - index(0,0,0);
    const int dy = index(0,1,0) - index(0,0,0);
    const int dz = index(0,0,1) - index(0,0,0);
    int ind = 0;
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                ind = index(i,j,k);
                //if( index(i+1,j,k) != (ind + dx) ) throw "ERROR";
                //if( index(i,j+1,k) != (ind + dy) ) throw "ERROR";
                //if( index(i,j,k+1) != (ind + dz) ) throw "ERROR";
                //if( index(i-1,j,k) != (ind - dx) ) throw "ERROR";
                //if( index(i,j-1,k) != (ind - dy) ) throw "ERROR";
                //if( index(i,j,k-1) != (ind - dz) ) throw "ERROR";

                m_vorticityForceU[ind] = h * ( m_vorticityMagnitude[ind+dx] - m_vorticityMagnitude[ind-dx] );
                m_vorticityForceV[ind] = h * ( m_vorticityMagnitude[ind+dy] - m_vorticityMagnitude[ind-dy] );
                m_vorticityForceW[ind] = h * ( m_vorticityMagnitude[ind+dz] - m_vorticityMagnitude[ind-dz] );
            }
        }
    }
    // Normalize the vorticity gradient  N = \frac{\eta}{|\eta|}
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                const int ind = index(i,j,k);
                float len = std::sqrt( m_vorticityForceU[ind]*m_vorticityForceU[ind]
                                     + m_vorticityForceV[ind]*m_vorticityForceV[ind]
                                     + m_vorticityForceW[ind]*m_vorticityForceW[ind] );
                float invlen = 0;
                const float epsilon = 1e-10;
                if( len > epsilon )
                {
                    invlen = 1.0f / len;
                }
                m_vorticityForceU[ind] *= invlen;
                m_vorticityForceV[ind] *= invlen;
                m_vorticityForceW[ind] *= invlen;
            }
        }
    }
    // now m_vorticityForceUVW holds the gradient of the normalized velocity curl, AKA N in FSJ

    // compute the final force as the scaled cross product of N x w
    // or m_vorticityForce x m_velCurl, since vorticityForce temporarily holds N (normalized)
    float f = m_vorticityConfinementFactor * (1.0/(m_N+2));
    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                const int ind = index(i,j,k);
                float nU = m_vorticityForceU[ind];
                float nV = m_vorticityForceV[ind];
                float nW = m_vorticityForceW[ind];
                m_vorticityForceU[ind] = f * ( nV * m_vorticityW[ind] - nW * m_vorticityV[ind] );
                m_vorticityForceV[ind] = f * ( nW * m_vorticityU[ind] - nU * m_vorticityW[ind] );
                m_vorticityForceW[ind] = f * ( nU * m_vorticityV[ind] - nV * m_vorticityU[ind] );
            }
        }
    }
}

/// Compute the scalar field d by back-projecting d_prev by velocity field vx,vy,vz
void
spark::Fluid
::advect( int boundaryCondition, 
          float* d, float* d_prev, 
          float* vx, float* vy, float* vz, 
          float dt )
{
    float dt0 = dt * m_N;
    float x, y, z;  // back-traced position
    int i0, i1, j0, j1, k0, k1;  // index of grid points around the back-traced position xyz
    float s0, s1, t0, t1, u0, u1; // distance between grid points and xyz

    for( size_t k = 1; k <= m_N; k++ )
    {
        for( size_t j = 1; j <= m_N; j++ )
        {
            for( size_t i = 1; i <= m_N; i++ )
            {
                // back-project the velocities to find x,y,z that landed here at i,j,k
                x = i - dt0 * vx[index(i,j,k)]; 
                y = j - dt0 * vy[index(i,j,k)];
                z = k - dt0 * vz[index(i,j,k)]; 

                // bound x,y,z to the grid  (so pulling from the outside of the grid gives 
                // you the clamped edge)
                x = std::max( 0.5f, x ); x = std::min( ((float)m_N)+0.5f, x );
                y = std::max( 0.5f, y ); y = std::min( ((float)m_N)+0.5f, y );
                z = std::max( 0.5f, z ); z = std::min( ((float)m_N)+0.5f, z );
                // (xyz) lies inside a cell, get the coordinates around it.
                // note -- we'll average x from coordinates
                // i0,j0,k0, i1,j0,k0, i1,j1,k0, i0,j1,k0
                // i0,j0,k1, i1,j0,k1, i1,j1,k1, i0,j1,k1
                // note that the truncated xyz coord becomes i0,j0,z0
                i0 = (int)x; i1 = i0+1;
                j0 = (int)y; j1 = j0+1;
                k0 = (int)z; k1 = k0+1;

                // the distance of the xyz point from the ijk vertex
                s1 = x - i0;  s0 = 1.0f - s1;
                t1 = y - j0;  t0 = 1.0f - t1;
                u1 = z - k0;  u0 = 1.0f - u1;

                // reverse projection -- add up all the densities that would have 
                // gotten to i,j,k
                // scaled by the normalized distance from the xyz back-traced position
                d[index(i,j,k)] 
                =  s0*(
                      t0*(u0*d_prev[index(i0,j0,k0)] + u1*d_prev[index(i0,j0,k1)])
                    +(t1*(u0*d_prev[index(i0,j1,k0)] + u1*d_prev[index(i0,j1,k1)]))
                  )
                 + s1*(
                      t0*(u0*d_prev[index(i1,j0,k0)] + u1*d_prev[index(i1,j0,k1)])
                    +(t1*(u0*d_prev[index(i1,j1,k0)] + u1*d_prev[index(i1,j1,k1)]))
                  );
            }
        }
    }
    enforceConcentrationBoundary( boundaryCondition, d );
}

void
spark::Fluid
::enforceBoundary( int dim, float* x )
{
    const float N = m_N;

    // 4 walls
    for( size_t a = 1; a <= N; a++ )
    {
        for( size_t b = 1; b <= N; b++ )
        {
            x[index(0,  a,b)] = dim==1 ? -x[index(1,a,b)] : x[index(1,a,b)];
            x[index(N+1,a,b)] = dim==1 ? -x[index(N,a,b)] : x[index(N,a,b)];

            x[index(a,0,  b)] = dim==2 ? -x[index(a,1,b)] : x[index(a,1,b)];
            x[index(a,N+1,b)] = dim==2 ? -x[index(a,N,b)] : x[index(a,N,b)];

            x[index(a,b,  0)] = dim==3 ? -x[index(a,b,1)] : x[index(a,b,1)];
            x[index(a,b,N+1)] = dim==3 ? -x[index(a,b,N)] : x[index(a,b,N)];
        }
    }

    // 12 edges
    for( size_t a = 1; a <= N; ++a )
    {
        // x varies
        x[index(a,0,0)] = 0.5*( x[index(a,1,0)] + x[index(a,0,1)] );
        x[index(a,N+1,0)] = 0.5*( x[index(a,N,0)] + x[index(a,N+1,1)] );
        x[index(a,0,N+1)] = 0.5*( x[index(a,1,N+1)] + x[index(a,0,N)] );
        x[index(a,N+1,N+1)] = 0.5*( x[index(a,N,N+1)] + x[index(a,N+1,N)] );

        // y varies
        x[index(0,a,0)] = 0.5*( x[index(1,a,0)] + x[index(0,a,1)] );
        x[index(N+1,a,0)] = 0.5*( x[index(N,a,0)] + x[index(N+1,a,1)] );
        x[index(0,a,N+1)] = 0.5*( x[index(1,a,N+1)] + x[index(0,a,N)] );
        x[index(N+1,a,N+1)] = 0.5*( x[index(N,a,N+1)] + x[index(N+1,a,N)] );

        // z varies
        x[index(0,0,a)] = 0.5*( x[index(1,0,a)] + x[index(0,1,a)] );
        x[index(N+1,0,a)] = 0.5*( x[index(N,0,a)] + x[index(N+1,1,a)] );
        x[index(0,N+1,a)] = 0.5*( x[index(1,N+1,a)] + x[index(0,N,a)] );
        x[index(N+1,N+1,a)] = 0.5*( x[index(N,N+1,a)] + x[index(N+1,N,a)] );
    }

    // 8 corners
    const float third = 1.0f/3.0f;
    x[index(0,  0,  0  )] = third*( x[index(1,  0,  0)] + x[index(0,  1,  0)] + x[index(0,  0,  1)] );

    x[index(N+1,0,  0  )] = third*( x[index(N,  0,  0)] + x[index(N+1,1,  0)] + x[index(N+1,0,  1)] );
    x[index(0,  N+1,0  )] = third*( x[index(1,N+1,  0)] + x[index(0  ,N,  0)] + x[index(0,  N+1,1)] );
    x[index(0,  0,  N+1)] = third*( x[index(1,  0,N+1)] + x[index(0  ,1,N+1)] + x[index(0,  0,  N)] );

    x[index(0  ,N+1,N+1)] = third*( x[index(1,N+1,N+1)] + x[index(0,  N,N+1)] + x[index(0,  N+1,N)] );
    x[index(N+1,0,  N+1)] = third*( x[index(N,0,  N+1)] + x[index(N+1,0,N+1)] + x[index(N+1,0,  N)] );
    x[index(N+1,N+1,0  )] = third*( x[index(N,N+1,  0)] + x[index(N+1,N,  0)] + x[index(N+1,N+1,1)] );

    x[index(N+1,N+1,N+1)] = third*( x[index(N,N+1,N+1)] + x[index(N+1,N,N+1)] + x[index(N+1,N+1,N)] );
}

void
spark::Fluid
::enforceConcentrationBoundary( int dim, float* x )
{
    const float N = m_N;
    
    // 4 walls
    for( size_t a = 1; a <= N; a++ )
    {
        for( size_t b = 1; b <= N; b++ )
        {
            x[index(0,  a,b)] = 0;
            x[index(N+1,a,b)] = 0;
            
            x[index(a,0,  b)] = dim==2 ? -x[index(a,1,b)] : x[index(a,1,b)];
            x[index(a,N+1,b)] = 0;
            
            x[index(a,b,  0)] = 0;
            x[index(a,b,N+1)] = 0;
        }
    }

    // 12 edges
    for( size_t a = 1; a <= N; ++a )
    {
        // x varies
        x[index(a,0,0)] = 0;
        x[index(a,N+1,0)] = 0;
        x[index(a,0,N+1)] = 0;
        x[index(a,N+1,N+1)] = 0;
        
        // y varies
        x[index(0,a,0)] = 0;
        x[index(N+1,a,0)] = 0;
        x[index(0,a,N+1)] = 0;
        x[index(N+1,a,N+1)] = 0;
        
        // z varies
        x[index(0,0,a)] = 0;
        x[index(N+1,0,a)] = 0;
        x[index(0,N+1,a)] = 0;
        x[index(N+1,N+1,a)] = 0;
    }
    
    // 8 corners
    const float third = 1.0f/3.0f;
    x[index(0,  0,  0  )] = third*( x[index(1,  0,  0)] + x[index(0,  1,  0)] + x[index(0,  0,  1)] );
    
    x[index(N+1,0,  0  )] = third*( x[index(N,  0,  0)] + x[index(N+1,1,  0)] + x[index(N+1,0,  1)] );
    x[index(0,  N+1,0  )] = third*( x[index(1,N+1,  0)] + x[index(0  ,N,  0)] + x[index(0,  N+1,1)] );
    x[index(0,  0,  N+1)] = third*( x[index(1,  0,N+1)] + x[index(0  ,1,N+1)] + x[index(0,  0,  N)] );
    
    x[index(0  ,N+1,N+1)] = third*( x[index(1,N+1,N+1)] + x[index(0,  N,N+1)] + x[index(0,  N+1,N)] );
    x[index(N+1,0,  N+1)] = third*( x[index(N,0,  N+1)] + x[index(N+1,0,N+1)] + x[index(N+1,0,  N)] );
    x[index(N+1,N+1,0  )] = third*( x[index(N,N+1,  0)] + x[index(N+1,N,  0)] + x[index(N+1,N+1,1)] );
    
    x[index(N+1,N+1,N+1)] = third*( x[index(N,N+1,N+1)] + x[index(N+1,N,N+1)] + x[index(N+1,N+1,N)] );
}


void 
spark::Fluid
::stepVelocity( float dt )
{
    computeVorticityConfinment();
    // update vel{UVW} with sources, including external and internal forces
    addVelocitySources(dt);
    
    swapVelocityField();
    // now the "current" velocity is in _prev, compute the 
    // new vel from diffusion and put it in velU,velV,velW
    diffuse( 1, m_velU, m_velU_prev, m_visc, dt );
    diffuse( 2, m_velV, m_velV_prev, m_visc, dt );
    diffuse( 3, m_velW, m_velW_prev, m_visc, dt );
    
    // project updates the m_vel field to enforce divergence free property on m_velUVW
    project();
    
    swapVelocityField(); 
    // now the "current" velocity is in _prev, compute the 
    // new vel and put it in velU,velV,velW
    advect( 1, m_velU, m_velU_prev,   m_velU_prev, m_velV_prev, m_velW_prev, dt );
    advect( 2, m_velV, m_velV_prev,   m_velU_prev, m_velV_prev, m_velW_prev, dt );
    advect( 3, m_velW, m_velW_prev,   m_velU_prev, m_velV_prev, m_velW_prev, dt );
    // m_vel holds the new velocity
    
    // project updates the m_vel field to enforce divergence free property on m_velUVW
    project();
}

void
spark::Fluid
::stepDensity( float dt )
{
    addDensitySources(dt);
    
    swapAdvectedFields(); // now m_density is ready for writing; density data in m_density_prev
    diffuse( 0, m_density, m_density_prev, m_diff, dt );
    // diffused density is now in m_density
    
    swapAdvectedFields(); // now m_density is ready for writing
    advect( 0, m_density, m_density_prev, m_velU, m_velV, m_velW, dt );
    // now m_density has the advected density
}

void 
spark::Fluid
::fixedUpdate( float dt )
{
    stepVelocity( dt * 0.05 );
    stepDensity( dt * 0.05 );
}

void
spark::Fluid
::addSourceAtLocation( float x, float y )
{
    const float lenOfCell = 1.0f / (m_N+2);
    int rx = (int)( (x/lenOfCell) + ( (float)(m_N+2)/2.0f - 0.5f ) );
    int ry = (int)( (y/lenOfCell) + ( (float)(m_N+2)/2.0f - 0.5f ) );
    
    size_t i = index( rx, ry, m_N );
    m_density[ i ] = std::min( m_density[i] + 0.15, 0.4 );

    //m_density_prev[ i ] = 0.4;
    //m_temp[ i ] = 150.0;
    //m_temp_prev[ i ] = m_ambientTemp + 50.0;

    //m_density[ index( rx, ry, 2) ] = 1;
    //m_density_prev[ index( rx, ry, 2 ) ] = 1;
    //m_density[ index( rx, ry, 3) ] = 1;
    //m_density_prev[ index( rx, ry, 3 ) ] = 1;


    //m_density[ index( 1, 1, 1) ] = 1;
    //m_density_prev[ index( 1, 1, 1 ) ] = 1;

    //m_density[ index( 1, m_N, 1) ] = 1;
    //m_density_prev[ index( 1, m_N, 1 ) ] = 1;

    //m_density[ index( m_N, 1, 1) ] = 1;
    //m_density_prev[ index( m_N, 1, 1 ) ] = 1;

    //m_density[ index(x+1,y+1,1) ] = 0.5f;
    //m_density[ index(x+1,y-1,1) ] = 0.5f;
    //m_density[ index(x-1,y+1,1) ] = 0.5f;
    //m_density[ index(x-1,y-1,1) ] = 0.5f;

    //m_density[ index(x,y,2) ] = 0.5f;
    //m_density[ index(x+1,y+1,2) ] = 0.5f;
    //m_density[ index(x+1,y-1,2) ] = 0.5f;
    //m_density[ index(x-1,y+1,2) ] = 0.5f;
    //m_density[ index(x-1,y-1,2) ] = 0.5f;

    //m_density_source[ index(x,y,1) ] = 0.5f;
    //m_temp[ index(x,y,1) ] = m_ambientTemp + 50.0f;
}