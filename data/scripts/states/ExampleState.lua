----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Fonts = require "Fonts"
----------------------------------------

local ExampleState = {}

function ExampleState:new()
    newObj =
    { 
        currTime = 0, 
        startTime = nil 
    }
    self.__index = self
    return setmetatable(newObj, self)
end

function ExampleState:load()
    print( "ExampleState:load" )
    local isShadowOn = false
    Render:createDefaultRenderPasses( isShadowOn )

    self.boxMat = spark:createMaterial( "colorShader" )
    self.boxMat:setVec4( "u_color", vec4(1,0.5,0.5,1.0) )

    self.boxA = spark:createCube( vec3(-1.0, 0, 0), 0.25, self.boxMat, "OpaquePass" )
    self.boxB = spark:createCube( vec3( 1.0, 0, 0), 0.25, self.boxMat, "OpaquePass" )

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
end

function ExampleState:activate()
    print( "ExampleState:activate" )
    -- acquire resources release on last deactivate()
    self.startTime = -1

    local camera = spark:getCamera()
    camera:cameraPos( 0.0, 0.345, 3.222 )
    camera:cameraTarget( 0.0, 0.0, 0.0 )
    camera:cameraUp( 0,1,0 )
end

function ExampleState:update( dt )
    self.currTime = self.currTime + dt
    self.msg:setText( string.format( "Time: %f", self.currTime ) )

    self.boxA:rotate( dt*45.0, vec3(0,1,0) )
    self.boxB:rotate( dt*30.0, vec3(1,0,0) )
end

function ExampleState:deactivate()
    print( "ExampleState:deactivate" )
    -- Release resources not needed until next activate()
    self.startTime = nil
end

function ExampleState:nextState( currTime )
    if self.startTime == nil then self.startTime = currTime end
    theNextState = "" -- Keep current state
end

theState = ExampleState:new()
theNextState = ""
