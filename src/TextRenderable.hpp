//
//  TextRenderable.hpp
//  sparkGui
//
//  Created by Brian Allen on 7/16/13.
//
//

#ifndef spark_TextRenderable_hpp
#define spark_TextRenderable_hpp

#include "Renderable.hpp"
#include "TextureManager.hpp"
#include "FileAssetFinder.hpp"

// Freetype-GL
#include "freetype-gl.h"
#include "edtaa3func.h"
#include "font-manager.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
//#include "markup.h"

namespace spark
{
    class TextRenderable : public Renderable
    {
    public:
        TextRenderable( const RenderableName& name );
        virtual ~TextRenderable() {}

        void initialize( TextureManagerPtr tm, 
                         const TextureName& fontTextureName
                         /* FontManagerPtr */ );
        /// Renderable
        virtual void render( const RenderCommand& rc ) const override;
        virtual void update( float dt ) override;
        virtual void attachShaderAttributes( GLuint shaderIndex ) override;

        void setText( const std::string& msg, float linesPerViewport = 10.0f );
    private:
        TextureName m_fontTextureName;
        TextureManagerPtr m_textureManager;
        FileAssetFinderPtr m_finder;
        vertex_buffer_t* m_vertexBuffer;
        texture_atlas_t* m_fontAtlas;
        texture_font_t*  m_font;
        GLuint m_vao;
        std::string m_text;
        float m_linesPerViewport;
        bool m_isDirty;
    };
    typedef spark::shared_ptr< TextRenderable > TextRenderablePtr;


    //////////////////////////////////////////////////////////////////////
    // Font Helpers
    void add_text( vertex_buffer_t * buffer, 
                   texture_font_t * font,
                   const wchar_t * text,
                   vec4 * color, 
                   vec2 * pen,
                   vec4& bbox );
    unsigned char* make_distance_map( unsigned char *img,
                                      unsigned int width, 
                                      unsigned int height );
    
    // ------------------------------------------------------- typedef & struct ---
    typedef struct {
        float x, y, z;    // position
        float s, t;       // texture
        float r, g, b, a; // color
    } vertex_t;
    

} // end namespace spark

#endif
