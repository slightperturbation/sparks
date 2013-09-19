
#include "IlluminationModel.hpp"

#include "Projection.hpp"
#include "Material.hpp"
#include "Renderable.hpp"

spark::AmbientLight
::AmbientLight( const glm::vec4& color )
: m_color( color )
{ }

spark::AmbientLight
::~AmbientLight()
{ }

std::string
spark::AmbientLight
::name( void ) const
{
    std::stringstream ss;
    ss << "Ambient[color=" << m_color << "]";
    return ss.str();
}

void
spark::AmbientLight
::setShaderUniforms( MaterialPtr material, ConstRenderablePtr renderable ) const
{
    material->setShaderUniform( "u_lightAmbientColor", m_color );
}

std::string
spark::ShadowLight
::name( void ) const
{
    std::stringstream ss;
    ss << "u_shadowLight[" << m_index << "]";
    return ss.str();
}

void
spark::ShadowLight
::setShaderUniforms( MaterialPtr material, ConstRenderablePtr renderable ) const
{
    const glm::mat4 model = renderable->getTransform();
    const glm::mat4 projViewModel =   m_projection->projectionMatrix() 
                                    * m_projection->viewMatrix()
                                    * model;
   
    material->setShaderUniform( name() + ".projViewModelMat", projViewModel );
    material->setShaderUniform( name() + ".color", m_color );
    //    material->setShaderUniform( nam() + ".shadowMap", tm->get... );
}

spark::ShadowLight
::ShadowLight( unsigned int index,
                    const glm::vec4& color,
                    ProjectionPtr projection )
: m_index( index ),
  m_color( color ),
  m_projection( projection )
{ }

spark::ShadowLight
::~ShadowLight()
{ }

void
spark::IlluminationModel
::addAmbientLight( glm::vec4 color )
{
    LightPtr l( new AmbientLight( color ) );
    m_lights.push_back( l );
}

void
spark::IlluminationModel
::addShadowLight( glm::vec4 color, ProjectionPtr projection )
{
    LightPtr l( new ShadowLight( m_lights.size(), color, projection ) );
    m_lights.push_back( l );
}

std::ostream& spark::operator<<( std::ostream& out,
                          const spark::IlluminationModel& ill )
{
    out << "IlliminationModel{";
    int counter = 0;
    for( auto i = ill.m_lights.begin(); i != ill.m_lights.end(); ++i )
    {
        out << "(Light " << ++counter << "=" << (*i)->name() << ")";
    }
    out << "}";
    return out;
}

void
spark::IlluminationModel
::setShaderUniforms( MaterialPtr material, ConstRenderablePtr renderable ) const
{
    for( auto i = m_lights.begin(); i != m_lights.end(); ++i )
    {
        (*i)->setShaderUniforms( material, renderable );
    }
}






