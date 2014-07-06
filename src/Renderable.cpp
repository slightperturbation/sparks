#include "Renderable.hpp"

#include "Material.hpp"

#include <glm/gtc/matrix_transform.hpp>

spark::Renderable
::Renderable( const RenderableName& name )
    : m_name( name ), m_requiresExplicitMaterial( false )
{ }

spark::Renderable
::~Renderable()
{
    LOG_DEBUG(g_log) << "Dtor - Renderable \"" << name() << "\".";
}

spark::RenderableName 
spark::Renderable
::name( void ) const 
{ 
    return m_name; 
}

void
spark::Renderable
::name( const RenderableName& aName ) 
{ 
    m_name = aName; 
}

bool
spark::Renderable
::requiresExplicitMaterial( void ) const
{ 
    return m_requiresExplicitMaterial; 
}

void 
spark::Renderable
::setRequireExplicitMaterial( bool explicitMat )
{ 
    m_requiresExplicitMaterial = explicitMat;
}

const glm::mat4& 
spark::Renderable
::getTransform( void ) const 
{ 
    return m_objectTransform; 
}

void 
spark::Renderable
::setTransform( const glm::mat4& mat ) 
{
    m_objectTransform = mat; 
}

void 
spark::Renderable
::transform( const glm::mat4& mat )
{
    m_objectTransform = m_objectTransform * mat; 
}

void 
spark::Renderable
::scale( const glm::vec3& scaleFactor )
{
    m_objectTransform = glm::scale( m_objectTransform, scaleFactor );
}
void 
spark::Renderable
::scale( float scaleFactor )
{
    // TODO$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//    m_objectTransform = glm::scale( m_objectTransform,
//        scaleFactor,
//        scaleFactor,
//        scaleFactor );
}
void
spark::Renderable
::translate( const glm::vec3& x )
{
    m_objectTransform = glm::translate( m_objectTransform, x );
}

void 
spark::Renderable
::translate( float x, float y, float z )
{
    m_objectTransform = glm::translate( m_objectTransform,
        glm::vec3(x,y,z) );
}

glm::vec3
spark::Renderable
::getTranslation( void )
{
    return glm::vec3( m_objectTransform[3][0], 
                      m_objectTransform[3][1],
                      m_objectTransform[3][2] ); 
}

void 
spark::Renderable
::rotate( float angleInDegrees, const glm::vec3& axis )
{
    m_objectTransform = glm::rotate( m_objectTransform,
        angleInDegrees,
        axis );
}

void 
spark::Renderable
::alignZAxisWithVector( const glm::vec3& dir )
{
    glm::mat4 invModel = glm::inverse( m_objectTransform );
    glm::vec4 dirN4 = glm::normalize( invModel * glm::vec4(dir, 0) );
    glm::vec3 dirN( dirN4.x, dirN4.y, dirN4.z );
    float angle = std::acos( glm::dot( dirN, glm::vec3(0,0,1) ) );
    const float epsilon = 1e-20f;
    if( std::fabs( angle ) > epsilon )
    {
        glm::vec3 axis = glm::cross( dirN, glm::vec3(0,0,1 ) ) ;
        axis = glm::normalize( axis );
        this->rotate( glm::degrees(angle), axis );
    }
}

spark::ConstMaterialPtr 
spark::Renderable
::getMaterialForPassName( const RenderPassName& renderPassName ) const
{ 
    auto itr = m_materials.find( renderPassName );
    if( itr != m_materials.end() )
    {
        LOG_TRACE(g_log) << "Using material \""
            << (*itr).second->name()
            << "\" for pass \""
            << renderPassName
            << "\" for renderable \""
            << m_name << "\".";
        return (*itr).second;
    }
    else
    {
        return ConstMaterialPtr();
    }
}

spark::MaterialPtr
spark::Renderable
::getMaterialForPassName( const RenderPassName& renderPassName )
{
    auto itr = m_materials.find( renderPassName );
    if( itr != m_materials.end() )
    {
        if( g_log->isTrace() )
        {
            LOG_TRACE(g_log) << "Using material \""
                             << (*itr).second->name()
                             << "\" for pass \""
                             << renderPassName
                             << "\" for renderable \""
                             << m_name << "\".";
        }
        return (*itr).second;
    }
    else
    {
        return MaterialPtr();
    }
}

void
spark::Renderable
::setMaterialForPassName( const spark::RenderPassName& renderPassName, 
                          spark::MaterialPtr material )
{
    if( g_log->isTrace() )
    {
        LOG_TRACE(g_log) << "Assigning material \"" << material->name() 
                         << "\" to renderable \"" << name() << "\" for pass \"" 
                         << renderPassName << "\".";
    }
    m_materials[ renderPassName ] = material;
    GLuint shaderId = material->getGLShaderIndex();
    attachShaderAttributes( shaderId );
}

std::ostream& spark::operator<<( std::ostream& out, 
                                 ConstRenderablePtr renderable )
{
    out << renderable->name();
    if( renderable->m_requiresExplicitMaterial )
    {
        out << "(E)";
    }
    out << "{";
    for( auto i = renderable->m_materials.begin();
         i != renderable->m_materials.end();
         ++i )
    {
        out << i->first << "->" << i->second->name() << ", ";
    }
    out << "}";
    return out;
}

std::ostream& spark::operator<<( std::ostream& out, 
                                 RenderablePtr renderable )
{
    ConstRenderablePtr cr = renderable;
    out << cr;
    return out;
}

