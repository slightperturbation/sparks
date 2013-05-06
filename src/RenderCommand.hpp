//
//  RenderCommand.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_RenderCommand_hpp
#define sparks_RenderCommand_hpp

#include "SoftTestDeclarations.hpp"

//#include <vector>
#include <functional>
//#include <queue>

///////
#include "Material.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <sstream>

class Light
{
public:
    virtual ~Light() {}
    virtual std::string name( void ) const = 0;
    virtual void setShaderUniforms( MaterialPtr material ) = 0;
};
typedef std::shared_ptr< Light > LightPtr;
typedef std::shared_ptr< const Light > ConstLightPtr;

class AmbientLight : public Light
{
    virtual std::string name( void ) const 
    {
        std::stringstream ss;
        ss << "Ambient[color=" << m_color << "]";
        return ss.str();
    }
    virtual void setShaderUniforms( MaterialPtr material )
    {
        material->setShaderUniform( "u_lightAmbientColor", m_color );
    }
private:
    glm::vec4 m_color;
};

class PointLight : public Light
{
    virtual std::string name( void ) const 
    {
        std::stringstream ss;
        ss << "u_pointLight[" << m_index << ", pos=" << m_pos << "]";
        return ss.str();
    }
    virtual void setShaderUniforms( MaterialPtr material )
    {
        material->setShaderUniform( name() + ".pos", m_pos );
        material->setShaderUniform( name() + ".color", m_color );
    }
private:
    std::string uniformName( void ) const 
    {
        std::stringstream ss;
        ss << "u_pointLight[" << m_index << "]";
        return ss.str();
    }
    unsigned int m_index;
    glm::vec3 m_pos;
    glm::vec4 m_color;
};

class Illumination
{
public:
    virtual std::string name( void ) const 
    { 
        std::stringstream out;
        out << "{";
        int counter = 0;
        for( auto i = m_lights.begin(); i != m_lights.end(); ++i )
        {
            out << "(Light " << ++counter << "=" << (*i)->name() << ")";
        }
        out << "}";
        return out.str();
    }
    void setShaderUniforms( MaterialPtr material )
    {
        for( auto i = m_lights.begin(); i != m_lights.end(); ++i )
        {
            (*i)->setShaderUniforms( material );
        }
    }
private:
    std::vector< LightPtr > m_lights;
};
typedef std::shared_ptr< Illumination > IlluminationPtr;
typedef std::shared_ptr< const Illumination > ConstIlluminationPtr;
///////////

class RenderCommand
{
public:
    /// Apply this render command.  Only apply changes that differ
    /// from the precedingCommand.
    void operator() ( const RenderCommand& precedingCommand );

    ConstRenderPassPtr m_pass;
    ConstPerspectivePtr m_perspective;
    ConstRenderablePtr m_renderable;
    ConstMaterialPtr m_material;
    ConstIlluminationPtr m_illumination;
    friend std::ostream& operator<<( std::ostream& out, const RenderCommand& rc );
};

struct RenderCommandCompare 
    : public std::binary_function< const RenderCommand&, 
                                   const RenderCommand&, bool >
{
    bool operator()( const RenderCommand& a, const RenderCommand& b ) const;
};


#endif
