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
                                 "Hello, World!" )
    self.msg:translate( 0.05, 0.9, 0 )


    ------------------------------
    self.testMaterial = spark:createMaterial( "phongShader" )
    self.testMaterial:addTexture( "s_color", "hook_cautery" )
    self.testMaterial:setVec4( "u_light.position_camera", vec4(0,1,1,1) )
    self.testMaterial:setVec4( "u_light.diffuse", vec4(0.4,0.4,0.4,1) )
    self.testMaterial:setVec4( "u_ambientLight", vec4(0.0,0.0,0.0,1) )
    self.testMaterial:setVec4( "u_ks", vec4(1,1,1,1) )
    self.testMaterial:setFloat( "u_ns", 100.0 )
    self.testMaterial:setBool( "u_textureSwapUV", false )
    self.testMaterial:setVec2( "u_textureRepeat", vec2(1,1) )
    self.testMaterial:setVec4( "u_color", vec4( 1, 0.5, 0.5, 0.5) );
    
    self.testMaterial:addTexture( "s_shadowMap", "light0_shadowMap" )

    self.sphere = spark:loadMesh( "sphere.obj", self.testMaterial, "OpaquePass" )
    self.sphere:translate( 1.5, 0, 0 )
    self.sphere:scale( 2 )
    self.sphere:setMaterialForPassName( "ShadowPass", shadowMaterial )

    self.box = spark:createCube( vec3(-1.5, 0, 0), 1, self.testMaterial, "OpaquePass" )
    self.box:setMaterialForPassName( "ShadowPass", shadowMaterial )

end

function ShaderExampleState:activate()
    print( "ShaderExampleState:activate" )
    -- acquire resources release on last deactivate()
    self.startTime = -1

    local camera = spark:getCamera()
    camera:cameraPos( 0.0, 0.345, 3.222 )
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
