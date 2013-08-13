#ifndef SPARKS_FONTMANAGER_HPP
#define SPARKS_FONTMANAGER_HPP

#include "TextureManager.hpp"

// Freetype-GL
#include "freetype-gl.h"
#include "edtaa3func.h"
#include "font-manager.h"
#include "vertex-buffer.h"
#include "text-buffer.h"

namespace spark
{
    // Forward Decl for friendship
    class TextRenderable;

    /// Returns an allocated bipolar distance field (using malloc)
    unsigned char* make_distance_map( unsigned char *img,
        unsigned int width,
        unsigned int height );

    typedef std::string FontName;

    /// Uses a single texture atlas to store all fonts and sizes.
    /// See class TextRenderable for example usage.
    class FontManager
    {
        struct FontDesc
        {
            std::string filename;
            std::string name;
            int size;
        };
    public:
        /// Creates an atlas for managing fonts with the given texture name.
        FontManager( TextureManagerPtr tm, const TextureName& atlasTextureName );

        /// Build the atlas and upload to the video card
        /// Will be automatically called if getFont() is called after addFont()
        /// if not manually called.
        /// Note that this may take some time to process the distance_map.
        void generateAtlas( void );

        /// Load the font at fontFilename with fontSize size into the atlas.
        /// distance map should only be computed once per font.
        /// need to queue all the "active" fonts
        /// and run make_distance_map once
        void addFont( const FontName& fontName, int fontSize,
                      const std::string& fontFilename );
        
        /// Remove all current fonts
        void clear( void );
        
        /// Returns the font with exactly the given name and size.
        /// If no such font has been loaded, a nullptr is returned.
        texture_font_t* getFont( std::string fontName, int fontSize );

        /// Returns the texture name specified in the ctor.
        /// Use this texture name to get the atlas texture from the TextureManager.
        const TextureName& getFontAtlasTextureName( void ) const;
        
        /// Provide TextRenderable direct access to the held font_manager_t
        friend class TextRenderable;
    private:
        /// Queue of fonts that should be loaded on the next generateAtlas call.
        std::vector< FontDesc > m_fontLoadQueue;

        font_manager_t* m_fontManager;
        TextureManagerPtr m_textureManager;
        FileAssetFinderPtr m_finder;
        TextureName m_textureName;
        std::map< std::pair<std::string,int>, texture_font_t* > m_fonts;
        bool m_isDirty;
    };
    typedef spark::shared_ptr< FontManager > FontManagerPtr;
}


#endif
