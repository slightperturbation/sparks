//
//  PointSparkRenderable.cpp
//  sparks
//
//  Created by Brian Allen on 3/19/13.
//
//

#include "PointSparkRenderable.hpp"
#include <Eigen/OpenGLSupport>

PointSparkRenderable
::PointSparkRenderable( SparkPtr spark,
                        TextureManagerPtr tm,
                        ShaderManagerPtr sm )
: Renderable( "PointSparkRenderable" ),
  m_spark( spark )
{
    // Build materials for needed passes
    ShaderName colorShaderName = m_name + "_ColorShader";
    sm->loadShaderFromFiles( colorShaderName, 
//        "PointSparkVertex.glsl",
//        "PointSparkFragment.glsl" );
"colorVertexShader.glsl",
"colorFragmentShader.glsl" );

    ShaderPtr colorShader( new Shader( colorShaderName, sm ) );
    MaterialPtr sparkColorMaterial( new Material( colorShader ) );
    this->setMaterialForPassName( g_colorRenderPassName, sparkColorMaterial );
}

void
PointSparkRenderable
::render( void ) const
{
    const PointCharges& aggregate = m_spark->aggregate();
    glBegin( GL_POINTS );
    for( size_t i=0; i<aggregate.size(); ++i )
    {
        glColor3f( 1,1,1 );
        glPointSize( 3.0 );
        glVertex( aggregate[i].pos );
    }
    
    const PointCharges& candidate = m_spark->candidate();
    for( size_t i=0; i<candidate.size(); ++i )
    {
        glColor3f( 1,0.1,0.1 );
        float field = candidate[i].phi.norm();
        LOG_DEBUG(g_log) << "CAN[" << i << "] = " << field << "\n";
        glPointSize( field );
        glVertex( candidate[i].pos );
    }
    glEnd();
}

void
PointSparkRenderable
::update( float dt )
{
    
}
