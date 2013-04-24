#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "SoftTestDeclarations.hpp"
#include "Material.hpp"

#include <glm/glm.hpp>

/// Interface for objects to be rendered.
/// Exposes methods for changing shader and material properties.
/// Note that renderable has a collection of materials that can be 
/// applied as appropriate during different render passes.
class Renderable
{
public:
    Renderable( const RenderableName& name )
    : m_name( name )
    { }
    virtual ~Renderable() { }

    virtual RenderableName name( void ) const { return m_name; }
    virtual void name( const RenderableName& aName ) { m_name = aName; }

    /// Responsible for emitting GL primitives
    virtual void render( void ) const = 0;
    
    const glm::mat4& getTransform( void ) const { return m_objectTransform; }
    void setTransform( const glm::mat4& mat ) { m_objectTransform = mat; }

    ConstMaterialPtr getMaterialForPassName( const RenderPassName& renderPassName ) const
    { 
        auto itr = m_materials.find( renderPassName );
        if( itr != m_materials.end() )
        {
            return (*itr).second;
        }
        else
        {
            LOG_ERROR(g_log) << "Failed to find material for renderable for pass \""
            << renderPassName << "\" for renderable \"" << m_name << "\".";
            assert( false );
            return ConstMaterialPtr( NULL );
        }
    }
    void setMaterialForPassName( const RenderPassName& renderPassName, 
                             MaterialPtr material )
    {
        m_materials[ renderPassName ] = material;
    }

protected:
    RenderableName m_name;
    std::map< const RenderPassName, MaterialPtr > m_materials;
    glm::mat4 m_objectTransform;
};

/// Abstract class for objects that will be updated periodically.
class Updatable
{
public:
    virtual ~Updatable() {}

    /// Update this object by dtSeconds.
    /// dtSeconds is the wall-clock time difference between updates. 
    /// Subclasses are free to ignore it and use a fixed time-step.
    virtual void update( float dtSeconds ) = 0;
};

#endif

