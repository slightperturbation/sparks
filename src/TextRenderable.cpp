
#include "TextRenderable.hpp"

// Freetype-GL
#include "freetype-gl.h"
#include "edtaa3func.h"
#include "font-manager.h"
#include "vertex-buffer.h"
#include "text-buffer.h"

#include <boost/algorithm/string.hpp>

spark::TextRenderable
::TextRenderable( const RenderableName& name )
: Renderable( name ),
  m_vao( -1 ),
  m_text( "Lorem ipsum dolor sit amet, consectetur adipisicing elit,\nsed do eiusmod tempor incididunt ut labore\net dolore magna aliqua." ),
  m_isDirty( true )
{
}

void
spark::TextRenderable
::initialize( FontManagerPtr fontManager,
              const std::string& fontName,
              int fontSize )
{
    if( m_vao == -1 )
    {
        glGenVertexArrays( 1, &m_vao );
    }
    m_fontManager = fontManager;
    
    if( !m_fontManager->m_fontManager )
    {
        LOG_ERROR(g_log) << "TextRenderable[\"" << name() 
            << "\"] failed to initialize due to null fontManager.";
        throw "TextRenderable failed to init due to null fontManager";
    }
    // Init text-buffer
    // can't use text_buffer_new because we want to have our own font_manager
    m_textBuffer.manager = m_fontManager->m_fontManager;
    m_textBuffer.line_start = 0;
    m_textBuffer.line_ascender = 0;
    m_textBuffer.base_color.r = 0.0;
    m_textBuffer.base_color.g = 0.0;
    m_textBuffer.base_color.b = 0.0;
    m_textBuffer.base_color.a = 1.0;
    m_textBuffer.line_descender = 0;
    m_textBuffer.buffer = vertex_buffer_new( "v_position:3f,v_texCoord:2f,v_color:4f,ashift:1f,agamma:1f ");

    // Default markup
    vec4 black  = {{0.0, 0.0, 0.0, 1.0}};
    vec4 white  = {{1.0, 1.0, 1.0, 1.0}};
    vec4 none   = {{1.0, 1.0, 1.0, 0.0}};
    vec4 color = white;
    m_markup.family  = strdup( fontName.c_str() );
    m_markup.size    = fontSize;
    m_markup.bold    = 0;
    m_markup.italic  = 0;
    m_markup.rise    = 0.0;
    m_markup.spacing = 0;
    m_markup.gamma   = 0;
    m_markup.foreground_color    = color;
    m_markup.background_color    = none;
    m_markup.underline           = 0;
    m_markup.underline_color     = color;
    m_markup.overline            = 0;
    m_markup.overline_color      = color;
    m_markup.strikethrough       = 0;
    m_markup.strikethrough_color = color;
    m_markup.font = m_fontManager->getFont( fontName, fontSize );
    m_markup.font->kerning = 1;
    m_markup.font->hinting = 1;
    //m_markup.font->filtering = 1; // Purpose?
    
    if( !m_markup.font )
    {
        LOG_ERROR(g_log) << "markup.font was null. aborting TextRenderable::update()";
        throw "markup.font was null. aborting TextRenderable::update()";
    }
}

void
spark::TextRenderable
::setText( const std::string& msg )
{
    m_text = msg;
    m_isDirty = true;
}

void 
spark::TextRenderable
::render( const RenderCommand& rc ) const
{
    if( m_textBuffer.buffer )
    {
        glBindVertexArray( m_vao );
        vertex_buffer_render( m_textBuffer.buffer, GL_TRIANGLES );
        glBindVertexArray( 0 );
    }
}

void
spark::TextRenderable
::filterText( std::string& str )
{
    std::string out;
    // Tabs to four spaces
    boost::replace_all( str, "\t", "    " );
}

void 
spark::TextRenderable
::update( float dt )
{
    if( !m_isDirty )
    {
        return;
    }
    if( !m_markup.font )
    {
        LOG_ERROR(g_log) << "markup.font was null. aborting TextRenderable::update()";
        throw "markup.font was null. aborting TextRenderable::update()";
    }
    vec2 pen = {{0,0}};
    vec4 bbox = {{0,0,0,0}};
    filterText( m_text );
    
    text_buffer_clear( &m_textBuffer );
    std::wstring wtext( m_text.begin(), m_text.end() );
    text_buffer_add_text( &m_textBuffer, &pen, &m_markup,
        const_cast<wchar_t*>(wtext.c_str()),
        wtext.size() );
    // Deprecated method for writing multiple lines, text_buffer replaces this:
    //vec4 black = {{1,1,1,1}};
    //std::vector< std::string > lines;
    //boost::split( lines, m_text, boost::is_any_of("\n") );
    //for( auto lineIter = lines.begin(); lineIter != lines.end(); ++lineIter )
    //{
    //    std::wstring wline( lineIter->begin(), lineIter->end() );
    //    
    //    add_text( m_textBuffer.buffer, m_markup.font, wline.c_str(), &black, &pen, bbox );
    //    //text_buffer_printf( &m_textBuffer, &pen, &m_markup, wline.c_str(), NULL );
    //    // drop pen to next line
    //    pen.y -= m_markup.font->height;
    //    pen.x  = 0;
    //}

    // Scale the text buffer's vertex
    if( true )
    {
        vector_t * vertices = m_textBuffer.buffer->vertices;
        // convert pts to fraction of frame buffer
        float scaleW = 1.0f / m_fontManager->m_fontManager->atlas->width;
        float scaleH = 1.0f / m_fontManager->m_fontManager->atlas->height;
        for( size_t i = 0; i < vector_size(vertices); ++i )
        {
            vertex_t * vertex = (vertex_t *) vector_get(vertices,i);
            vertex->x -= (int)bbox.x;
            vertex->x *= scaleW;
            vertex->y -= (int)bbox.y;
            vertex->y *= scaleH;
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

