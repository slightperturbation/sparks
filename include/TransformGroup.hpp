//
//  CompositeRenderable.hpp
//  sparkGui
//
//  Created by Brian Allen on 8/14/13.
//
//

#ifndef sparkGui_TransformGroup_hpp
#define sparkGui_TransformGroup_hpp

#include "Spark.hpp"
#include "Renderable.hpp"

namespace spark
{
    /// Holds multiple Renderables, but does *not* act as a traditional scene graph
    /// TransformGroup can hold Renderables and provide a convient way to
    /// transform (translate, rotate, etc.) multiple renderables as a group.
    /// Of couse, constituent renderables must be added to scenes individually
    class TransformGroup
    {
    public:
        TransformGroup( void ) {}
        
        void add( RenderablePtr r ) { m_children.push_back( r ); }
        void clear( void ) { m_children.clear(); }
        bool empty( void ) const { return m_children.empty(); }
        
        void setTransform( const glm::mat4& mat )
        {
            for( auto child = m_children.begin();
                 child != m_children.end();
                 child++ )
            {
                child->setTransform( mat );
            }
        }
        void transform( const glm::mat4& mat )
        {
            for( auto child = m_children.begin();
                child != m_children.end();
                child++ )
            {
                child->transform( mat );
            }
        }
        void scale( const glm::vec3& scaleFactor )
        {
            for( auto child = m_children.begin();
                child != m_children.end();
                child++ )
            {
                child->scale( scaleFactor );
            }
        }
        void scale( float scaleFactor )
        {
            for( auto child = m_children.begin();
                child != m_children.end();
                child++ )
            {
                child->scale( scaleFactor );
            }
        }
        void translate( const glm::vec3& x )
        {
            for( auto child = m_children.begin();
                child != m_children.end();
                child++ )
            {
                child->translate( x );
            }
        }
        void translate( float x, float y, float z )
        {
            for( auto child = m_children.begin();
                child != m_children.end();
                child++ )
            {
                child->translate( x, y, z );
            }
        }
        void rotate( float angleInDegrees, const glm::vec3& axis )
        {
            for( auto child = m_children.begin();
                child != m_children.end();
                child++ )
            {
                child->rotate( angleInDegrees, axis );
            }
        }
    private:
        std::vector< RenderablePtr > m_children;
    };
}

#endif
