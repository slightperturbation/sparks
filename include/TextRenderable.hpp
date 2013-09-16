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
#include "Updateable.hpp"
#include "TextureManager.hpp"
#include "FileAssetFinder.hpp"
#include "TextureManager.hpp"
#include "FontManager.hpp"

// Freetype-GL
#include "freetype-gl.h"
#include "edtaa3func.h"
#include "font-manager.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
#include "markup.h"

namespace spark
{
    /// Displays the text string (left-aligned) specified by setText()
    /// Example:
    /// FontManager fm( textureManager, "TextureAtlas" );
    /// const std::string fontName = "Sans";
    /// const int fontSize = 72;
    /// fm->addFont( fontName, fontSize, "HelveticaNeue.ttf" );
    /// TextRenderable tr( fm, "Message" );
    /// tr.intialize( fm, fontName, fontSize );
    /// tr.setText( "Hello, World" );
    ///
    class TextRenderable : public Renderable, public Updateable
    {
    public:
        TextRenderable( const RenderableName& name );
        virtual ~TextRenderable();

        /// Sets the font manager to use for resources. Must be called
        /// before other methods.
        /// fontName and fontSize specify the font to use for this TextRenderable.
        /// The given font *must* have been previously added to FontManager
        /// using FontManager::addFont()
        void initialize( FontManagerPtr fm,
                         const std::string& fontName,
                         int fontSize );

        /// Renderable overrides
        virtual void render( const RenderCommand& rc ) const override;
        virtual void update( double dt ) override;
        virtual void attachShaderAttributes( GLuint shaderIndex ) override;
        
        /// Sets the text to display.  Height of the text is set so that
        /// linesPerViewport will fill up the vertical height of the current
        /// viewport.
        void setText( const std::string& msg );
        TextureName getFontTextureName( void ) const;
        glm::vec2 getSizeInPixels( void ) const;

    private:
        void calculateBoundingBox( void );
        /// Replace unsupported characters
        static void filterText( std::string& str );
        FontManagerPtr m_fontManager;
        /// Holds the vertex buffer and font info for rendering
        text_buffer_t m_textBuffer;
        /// Vertex array object for rendering
        GLuint m_vao;
        /// Current text string to be rendered      
        std::string m_text;
        markup_t m_markup;
        bool m_isDirty;
        glm::vec2 m_sizeInPixels;
    };
    typedef spark::shared_ptr< TextRenderable > TextRenderablePtr;

    //////////////////////////////////////////////////////////////////
    // Font Helpers

    /// vertex format used by freetype-gl
    typedef struct {
        float x, y, z;    // position
        float s, t;       // texture
        float r, g, b, a; // color
    } vertex_t;
} // end namespace spark

#endif
