
#include "TextRenderable.hpp"


// Freetype-GL
#include "freetype-gl.h"
#include "edtaa3func.h"
#include "font-manager.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
//#include "markup.h"

#include <boost/algorithm/string.hpp>


spark::TextRenderable
::TextRenderable( const RenderableName& name )
: Renderable( name ),
  m_fontTextureName( "_UNASSIGNED_FONT_TEXTURE" ),
  m_vertexBuffer( nullptr ),
  m_fontAtlas( nullptr ),
  m_font( nullptr ),
  m_vao( -1 ),
  m_text( "TextRenderable" ),
  m_linesPerViewport( 24.0 ),
  m_isDirty( true )
{
}

void 
spark::TextRenderable
::initialize( TextureManagerPtr tm, const TextureName& fontTextureName /* FontManagerPtr */ )
{
    if( m_vao == -1 )
    {
        glGenVertexArrays( 1, &m_vao );
    }
    m_fontTextureName = fontTextureName;
    m_textureManager = tm;
    m_finder = m_textureManager->assetFinder();
    // Load font from file
    std::string filename;
    m_finder->findFile( "Vera.ttf", filename );
    if( m_fontAtlas ) 
    {
        texture_atlas_delete( m_fontAtlas );
    }
    m_fontAtlas = texture_atlas_new( 512, 512, 1 );
    // 72pts in a 512x512 texture is a good compromise
    // giant letters look good, tiny fonts look OK
    m_font = texture_font_new( m_fontAtlas, filename.c_str(), 72 );
    { // Create the distance map and copy it to the font atlas
        unsigned char *map = make_distance_map( m_fontAtlas->data,
                                                m_fontAtlas->width,
                                                m_fontAtlas->height);
        memcpy( m_fontAtlas->data, map,
                m_fontAtlas->width*m_fontAtlas->height*sizeof(unsigned char) );
        free(map);
    }
    
    // activate the next texture unit to create a new texture with
    // avoids clobbering existing unit->texture mappings
    GLint textureUnit = m_textureManager->reserveTextureUnit();
    GL_CHECK( glActiveTexture( GL_TEXTURE0 + textureUnit ) );

    // creates texture on current texture unit
    texture_atlas_upload( m_fontAtlas );

    // bind texture with id=atlas->id
    if( m_fontAtlas->id == GL_FALSE )
    {
        std::stringstream msg;
        msg << "Font atlas failed to load using font at filename\""
        << filename << "\".  TextureID for FontAtlas is 0 (GL_FALSE).";
        LOG_ERROR(g_log) << msg.str();
        throw msg.str();
    }
    else
    {
        LOG_DEBUG(g_log) << "FontAtlas TextureId = " << m_fontAtlas->id;
        // Let texture manager know about the font texture
        // this name is used by shaders
        m_textureManager->acquireExternallyAllocatedTexture( m_fontTextureName,
                                                             m_fontAtlas->id,
                                                             GL_TEXTURE_2D,
                                                             textureUnit );
        GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
    }
}

void
spark::TextRenderable
::setText( const std::string& msg, float linesPerViewport )
{
    m_text = msg;
    m_linesPerViewport = linesPerViewport;
    m_isDirty = true;
}

void 
spark::TextRenderable
::render( const RenderCommand& rc ) const
{
    if( m_vertexBuffer )
    {
        glBindVertexArray( m_vao );
        vertex_buffer_render( m_vertexBuffer, GL_TRIANGLES );
        glBindVertexArray( 0 );
    }
}

void 
spark::TextRenderable
::update( float dt )
{
    if( !m_isDirty )
    {
        return;
    }
    if( m_vertexBuffer )
    {
        vertex_buffer_delete( m_vertexBuffer );
    }
    // create the texture and vertex buffer
    m_vertexBuffer = vertex_buffer_new( "v_position:3f,v_texCoord:2f,v_color:4f" );

    vec2 pen = {{0,0}};
    vec4 black = {{1,1,1,1}};
    
    // Need to select the correct texture unit for add_text
    m_textureManager->activateTextureUnitForHandle( m_fontTextureName );

    std::vector< std::string > lines;
    boost::split( lines, m_text, boost::is_any_of("\n") );
    vec4 bbox = {{0,0,0,0}};
    for( auto lineIter = lines.begin(); lineIter != lines.end(); ++lineIter )
    {
        std::wstring wline( lineIter->begin(), lineIter->end() );
        add_text( m_vertexBuffer, m_font, wline.c_str(), &black, &pen, bbox );
        
        // drop pen to next line
        pen.y -= m_font->height;
        pen.x  = 0;
    }
//    
//    //wchar_t *text = L"The quick brown fox jumps over the lazy dog. THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG-!@#$%^&*()_+{}[];:'.,?/\|012345678905";
//    wchar_t *text = L"A";
//    add_text( m_vertexBuffer, m_font, text, &black, &pen, bbox );
//
//    pen.y -= m_font->height;
//    pen.x  = 0;
//    add_text( m_vertexBuffer, m_font, L"B", &black, &pen, bbox );

    // normalize size of vertex buffer to [0,1]
    {
        vector_t * vertices = m_vertexBuffer->vertices;

        float maxDim = std::max<float>( bbox.width, bbox.height );
        // convert pts to fraction of framebuffer
        float scale = 1.0f / (m_linesPerViewport*53.0f);  
        for( size_t i = 0; i < vector_size(vertices); ++i )
        {
            vertex_t * vertex = (vertex_t *) vector_get(vertices,i);
            vertex->x -= (int)bbox.x;
            vertex->x *= scale;
            vertex->y -= (int)bbox.y;
            vertex->y *= scale;
        }
    }    
    m_isDirty = false;
}

void 
spark::TextRenderable
::attachShaderAttributes( GLuint shaderIndex )
{
    // Nothing.  This is handled by the freetype-gl vertextbuffer
}


///////////////////////////////////////////////////////////////////////////
// From FreeType-GL demo-distance-field-2.c
///////////////////////////////////////////////////////////////////////////


// --------------------------------------------------------------- add_text ---
void
spark
::add_text( vertex_buffer_t * buffer, texture_font_t * font,
            const wchar_t * text, vec4 * color, vec2 * pen, vec4& bbox )
{
    size_t i;
    float r = color->red, g = color->green, b = color->blue, a = color->alpha;
    for( i=0; i<wcslen(text); ++i )
    {
        texture_glyph_t *glyph = texture_font_get_glyph( font, text[i] );
        if( glyph != NULL )
        {
            int kerning = 0;
            if( i > 0)
            {
                kerning = texture_glyph_get_kerning( glyph, text[i-1] );
            }
            pen->x += kerning;
            float x0  = (int)( pen->x + glyph->offset_x );
            float y0  = (int)( pen->y + glyph->offset_y );
            float x1  = (int)( x0 + glyph->width );
            float y1  = (int)( y0 - glyph->height );
            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;
            GLuint indices[6] = {0,1,2, 0,2,3};
            vertex_t vertices[4] = { { x0,y0,0,  s0,t0,  r,g,b,a },
            { x0,y1,0,  s0,t1,  r,g,b,a },
            { x1,y1,0,  s1,t1,  r,g,b,a },
            { x1,y0,0,  s1,t0,  r,g,b,a } };
            vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );
            pen->x += glyph->advance_x;

            if  (x0 < bbox.x)                bbox.x = x0;
            if  (y1 < bbox.y)                bbox.y = y1;
            if ((x1 - bbox.x) > bbox.width)  bbox.width  = x1-bbox.x;
            if ((y0 - bbox.y) > bbox.height) bbox.height = y0-bbox.y;
        }
    }
}


// ------------------------------------------------------ make_distance_map ---
unsigned char *
spark
::make_distance_map( unsigned char *img,
                     unsigned int width, unsigned int height )
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
