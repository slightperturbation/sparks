
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

spark::TextRenderable
::~TextRenderable()
{
    vertex_buffer_delete( m_textBuffer.buffer );
    if( m_markup.family )
    {
        free( m_markup.family );
    }
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
    
    // TODO!  Inverse scale by screen resolution!
    //scale( glm::vec3( 1.0f/800.0f, 1.0f/600.0f, 1.0f ) );

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

spark::TextureName
spark::TextRenderable
::getFontTextureName( void ) const
{
    return ( m_fontManager ) ?
    m_fontManager->getFontAtlasTextureName()
    : TextureName("INVALID_FONT_ATLAS_TEXTURE_NAME_IN_TEXT_RENDERABLE");
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
::update( double dt )
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
    filterText( m_text );
    
    text_buffer_clear( &m_textBuffer );
    std::wstring wtext( m_text.begin(), m_text.end() );
    text_buffer_add_text( &m_textBuffer, &pen, &m_markup,
        const_cast<wchar_t*>(wtext.c_str()),
        wtext.size() );
    
    calculateBoundingBox();
    m_isDirty = false;
}

/// returns four-vector holding: ( minx, miny, maxx, maxy )
glm::vec2
spark::TextRenderable
::getSizeInPixels( void ) const
{
    return m_sizeInPixels;
}

void
spark::TextRenderable
::calculateBoundingBox( void )
{
    float maxFloat = 1e10;
    // bounding box is based on the vertices times the renderable xform
    vector_t * vertices = m_textBuffer.buffer->vertices;
    float minx = maxFloat;
    float miny = maxFloat;
    float maxx = -maxFloat;
    float maxy = -maxFloat;
    
    for( size_t i = 0; i < vector_size(vertices); ++i )
    {
        vertex_t * vertex = (vertex_t *) vector_get(vertices,i);
        
        //std::cerr << "VERTEX : " << vertex->x << "\t\t" << vertex->y << "\n";
        glm::vec4 p( vertex->x, vertex->y, 0, 0 );
        minx = std::min( minx, p.x );
        miny = std::min( miny, p.y );
        maxx = std::max( maxx, p.x );
        maxy = std::max( maxy, p.y );
    }
    m_sizeInPixels.x = maxx - minx;
    m_sizeInPixels.y = maxy - miny;
}

void 
spark::TextRenderable
::attachShaderAttributes( GLuint shaderIndex )
{
    // Nothing.  This is handled by the freetype-gl vertextbuffer
}

