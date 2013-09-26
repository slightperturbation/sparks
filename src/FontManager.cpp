#include "Spark.hpp"
#include "FontManager.hpp"

spark::FontManager
::FontManager( TextureManagerPtr tm,
               const TextureName& textureName )
: m_textureManager( tm ),
  m_textureName( textureName ),
  m_isDirty( false )
{
    m_finder = tm->assetFinder();
    // font_manager_new just allocates heap memory, not gfx memory
    m_fontManager = font_manager_new( 1024, 1024, 1 );
    if( m_fontManager->cache )
    {
        free( m_fontManager->cache );
    }
    const wchar_t *cache = L" !\"#$%&'()*+,-./0123456789:;<=>?"
    L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    L"`abcdefghijklmnopqrstuvwxyz{|}~";
    m_fontManager->cache = wcsdup( cache );
    
    // activate the next texture unit to create a new texture with
    // avoids clobbering existing unit->texture mappings
    GLint textureUnit = m_textureManager->reserveTextureUnit();
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + textureUnit ) );
    // create the opengl texture
    texture_atlas_upload( m_fontManager->atlas );
    if( m_fontManager->atlas->id == GL_FALSE )
    {
        std::stringstream msg;
        msg << "Font atlas failed to load.  TextureID for FontAtlas is 0 (GL_FALSE).";
        LOG_ERROR(g_log) << msg.str();
        throw msg.str();
    }
    else
    {
        LOG_DEBUG(g_log) << "FontAtlas TextureId = " 
                         << m_fontManager->atlas->id;
        // Let texture manager know about the font texture
        // this name is used by shaders
        m_textureManager->acquireExternallyAllocatedTexture( m_textureName,
                                                             m_fontManager->atlas->id,
                                                             GL_TEXTURE_2D,
                                                             textureUnit );
    }
}

spark::FontManager
::~FontManager()
{
    font_manager_delete( m_fontManager );
}

void
spark::FontManager
::generateAtlas( void )
{
    // Must set texture unit here because
    // font_manager_get_from_filename calls:
    // ->texture_font_load_glyphs
    // ->texture_atlas_upload
    // ->glBindTexture( GL_TEXTURE_2D, atlas->id );
    m_textureManager->activateTextureUnitForHandle( m_textureName );
    LOG_INFO(g_log) << "Generating Texture Atlas for Fonts";
    for( auto fontDesc = m_fontLoadQueue.begin();
        fontDesc != m_fontLoadQueue.end();
        fontDesc++ )
    {
        texture_font_t* font;
        LOG_INFO(g_log) << "\tLoading font from file \""
        << fontDesc->filename << "\" size=" << fontDesc->size ;
        font = font_manager_get_from_filename( m_fontManager,
                                              fontDesc->filename.c_str(),
                                              fontDesc->size );
        if( !font )
        {
            LOG_ERROR(g_log) << "Failed to load font from \""
            << fontDesc->filename << "\".";
            continue;
        }
        else
        {
            auto key = make_pair( fontDesc->name, fontDesc->size );
            m_fonts[ key ] = font;
        }
    }
    { // Create the distance map and copy it to the font atlas
        texture_atlas_t* atlas = m_fontManager->atlas;
        unsigned char *map = make_distance_map( atlas->data,
                                                atlas->width,
                                                atlas->height);
        memcpy( atlas->data, map,
               atlas->width * atlas->height * sizeof(unsigned char) );
        free(map);
    }
    texture_atlas_upload( m_fontManager->atlas );
    GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
    m_isDirty = false;
}

void
spark::FontManager
::addFont( const FontName& fontName,
           int fontSize,
           const std::string& fontFilename )
{
    for( auto iter = m_fontLoadQueue.begin();
        iter != m_fontLoadQueue.end();
        ++iter )
    {
        if( (iter->name == fontName) && (iter->size == fontSize) )
        {
            // already queued for loading
            return;
        }
    }
    FontDesc desc;
    if( m_finder->findFile( fontFilename, desc.filename ) )
    {
        desc.name = fontName;
        desc.size = fontSize;
        m_fontLoadQueue.push_back( desc );
        m_isDirty = true;
    }
    else
    {
        LOG_ERROR(g_log) << "Failed to find font with filename \""
        << desc.filename << "\".";
    }
}

void
spark::FontManager
::clear( void )
{
    m_fontLoadQueue.clear();
    m_fonts.clear();
}

texture_font_t*
spark::FontManager
::getFont( std::string fontName, int fontSize )
{
    if( m_isDirty )
    {
        generateAtlas();
    }
    auto key = make_pair( fontName, fontSize );
    auto iter = m_fonts.find( key );
    if( iter != m_fonts.end() )
    {
        return iter->second;
    }
    else
    {
        // Can't add without a filename
        return nullptr;
    }
}

const spark::TextureName&
spark::FontManager
::getFontAtlasTextureName( void ) const
{
    return m_textureName;
}


// ------------------------------------------------------ make_distance_map ---
unsigned char *
spark
::make_distance_map( unsigned char *img,
                     unsigned int width,
                     unsigned int height )
{
    short * xdist = (short *)  malloc( width * height * sizeof(short) );
    short * ydist = (short *)  malloc( width * height * sizeof(short) );
    double * gx   = (double *) calloc( width * height, sizeof(double) );
    double * gy      = (double *) calloc( width * height, sizeof(double) );
    double * data    = (double *) calloc( width * height, sizeof(double) );
    double * outside = (double *) calloc( width * height, sizeof(double) );
    double * inside  = (double *) calloc( width * height, sizeof(double) );
    int i;

    // Convert img into double (data)
    double img_min = 255, img_max = -255;
    for( i=0; i<width*height; ++i)
    {
        double v = img[i];
        data[i] = v;
        if (v > img_max) img_max = v;
        if (v < img_min) img_min = v;
    }
    // Rescale image levels between 0 and 1
    for( i=0; i<width*height; ++i)
    {
        data[i] = (img[i]-img_min)/img_max;
    }

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient( data, height, width, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, outside);
    for( i=0; i<width*height; ++i)
        if( outside[i] < 0 )
            outside[i] = 0.0;

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset(gx, 0, sizeof(double)*width*height );
    memset(gy, 0, sizeof(double)*width*height );
    for( i=0; i<width*height; ++i)
        data[i] = 1 - data[i];
    computegradient( data, height, width, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, inside);
    for( i=0; i<width*height; ++i)
        if( inside[i] < 0 )
            inside[i] = 0.0;

    // distmap = outside - inside; % Bipolar distance field
    unsigned char *out = (unsigned char *) malloc( width * height * sizeof(unsigned char) );
    for( i=0; i<width*height; ++i)
    {
        outside[i] -= inside[i];
        outside[i] = 128+outside[i]*16;
        if( outside[i] < 0 ) outside[i] = 0;
        if( outside[i] > 255 ) outside[i] = 255;
        out[i] = 255 - (unsigned char) outside[i];
        //out[i] = (unsigned char) outside[i];
    }

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( data );
    free( outside );
    free( inside );
    return out;
}