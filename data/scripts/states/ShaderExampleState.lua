----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Fonts = require "Fonts"
----------------------------------------

local ShaderExampleState = {}

function ShaderExampleState:new()
    newObj =
    { 
        currTime = 0, 
        startTime = nil 
    }
    self.__index = self
    return setmetatable(newObj, self)
end

function ShaderExampleState:load()
    print( "ShaderExampleState:load" )
    local isShadowOn = true
    Render:createDefaultRenderPasses( isShadowOn )

    -- Add a message to the screen
    self.textMaterial = spark:createMaterial( "TextShader" )
    self.textMaterial:addTexture( "s_color", spark:getFontManager():getFontAtlasTextureName() )
    self.textMaterial:setVec4( "u_color", vec4(1,0.5,0.5,1.0) )

    self.msg = spark:createText( Fonts.defaultFontName, 
                                 Fonts.defaultFontTextSize, 
                                 self.textMaterial,
                                 "HUDPass", 
                                 "" )
    self.msg:translate( 0.05, 0.9, 0 )


    ------------------------------
    self.testMaterial = spark:createMaterial( "phongShader" )
    self.testMaterial:addTexture( "s_color", "tissueDiffuse" )
    self.testMaterial:addTexture( "s_normal", "tissueNormal" )

    self.testMaterial:setVec4( "u_light.position_camera", vec4(shadowSource.x, shadowSource.y, shadowSource.z ,1) ) -- generally, should match shadow camera
    self.testMaterial:setVec4( "u_light.diffuse", vec4(0.7,0.7,0.7,1) )
    self.testMaterial:setVec4( "u_ambientLight", vec4(0.2,0.2,0.2,1) )
    self.testMaterial:setVec4( "u_ks", vec4(1,1,1,1) )
    self.testMaterial:setFloat( "u_ns", 15.0 )
    self.testMaterial:setBool( "u_textureSwapUV", false )
    self.testMaterial:setVec2( "u_textureRepeat", vec2(1,1) )
    self.testMaterial:setVec4( "u_color", vec4( 1, 0.5, 0.5, 0.5) );
    
    -- This material can receive shadows
    self.testMaterial:addTexture( "s_shadowMap", "light0_shadowMap" )

    self.sphere = spark:loadMesh( "sphere.obj", self.testMaterial, "OpaquePass" )
    self.sphere:setMaterialForPassName( "ShadowPass", shadowMaterial ) -- casts shadow
    self.sphere:translate( 1.5, 0, 0 )
    self.sphere:scale( 2 )

    self.box = spark:createCube( vec3(-1.5, 0, -0.5), 1, self.testMaterial, "OpaquePass" )
    self.box:setMaterialForPassName( "ShadowPass", shadowMaterial ) -- casts shadow

    self.ground = spark:createCube( vec3(-5, -11, -5), 10, self.testMaterial, "OpaquePass" )
    self.ground:setMaterialForPassName( "ShadowPass", shadowMaterial ) -- casts shadow


    local tissueScale = 0.25
    local worldOffset = vec3( 0, -.1, -.1 )
    self.tissue = spark:createPlane( worldOffset + vec3( 0, 0.5*tissueScale, -0.5*tissueScale ), 
                                      vec2(tissueScale, tissueScale), 
                                      ivec2( 512, 512 ), -- faster
                                      --ivec2( 1024, 1024 ), -- nicer
                                      self.testMaterial, 
                                      "OpaquePass" )
    self.tissue:setMaterialForPassName( "ShadowPass", shadowMaterial ) -- casts shadow
    
    self.table = spark:createCube( worldOffset + vec3(-0.5, -0.025, -0.5), 1, 
        self.testMaterial, "OpaquePass" )
    self.table:rotate( 90, vec3(1,0,0) )


    -- Create a Picture-in-Picture 
    if false then
        local pipCamera = spark:createPerspectiveProjection( vec3( -0.025, 5.75, -5.05 ), -- camera
                                                       vec3( 0, 0, 0 ),        -- target
                                                       vec3( 0, 1, 0 ),        -- up
                                                       50.0, -- FOV
                                                       1,  -- near plane
                                                       50 )   -- far plane
        local pipTarget = spark:createTextureRenderTarget( "pipTexture" )
        -- Note we're creating a second pass with the name OpaquePass that renders
        -- after the standard OpaquePass (1.0) and before the HUDPass (0.1)
        local pipPass = spark:createRenderPassWithProjection( 0.9, "OpaquePass", pipCamera, pipTarget )
        -- A standard shader for direct painting
        self.pipMaterial = spark:createMaterial( "texturedOverlayShader" )
        self.pipMaterial:addTexture( "s_color", "pipTexture" )
        -- The quad to show the PIP using the new material
        local pipSize = 0.3
        self.pipQuad = spark:createQuad( vec2( 0.5, 0.5 ), 
                                         vec2( pipSize*(9/16), pipSize ), -- 9/16 is aspect ratio of display 
                                         self.pipMaterial, "HUDPass" )
    end

    -- self.pencil = spark:loadMesh( "pen.obj", self.testMaterial, "OpaquePass" )
    -- self.sphere:setMaterialForPassName( "ShadowPass", shadowMaterial ) -- casts shadow
end

function ShaderExampleState:activate()
    print( "ShaderExampleState:activate" )
    -- acquire resources release on last deactivate()
    self.startTime = -1

    local camera = spark:getCamera()
    camera:cameraPos( 0.0, 0.5, 3 )
    camera:cameraTarget( 0.0, 0.0, 0.0 )
    camera:cameraUp( 0,1,0 )
end

function ShaderExampleState:update( dt )
    self.currTime = self.currTime + dt
    self.msg:setText( string.format( "Time: %f", self.currTime ) )

    self.sphere:rotate( dt*45.0, vec3(0,1,0) )
    self.box:rotate( dt*30.0, vec3(1,0,0) )
end

function ShaderExampleState:deactivate()
    print( "ShaderExampleState:deactivate" )
    -- Release resources not needed until next activate()
    self.startTime = nil
end

function ShaderExampleState:nextState( currTime )
    if self.startTime == nil then self.startTime = currTime end
    theNextState = "" -- Keep current state
end

theState = ShaderExampleState:new()
theNextState = ""
