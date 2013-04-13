//
//  Material.hpp
//  sparks
//
//  Created by Brian Allen on 4/9/13.
//
//

#ifndef sparks_Material_hpp
#define sparks_Material_hpp

class TextureUnit;

class Material
{
public:
    
    void use( void )
    {
        m_shader->use();
        
        // bind textures
    }
    
private:
    std::vector< TextureUnit > m_textureUnits;
    ShaderPtr m_shader;
    bool m_receivesShadows;
    bool m_castsShadows;
    bool m_isTranslucent;
};

class TextureUnit
{
public:
    // texture
    // minfilter, magfilter, mipmap, wrapping
};

#endif
