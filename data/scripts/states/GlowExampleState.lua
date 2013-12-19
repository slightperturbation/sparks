----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Fonts = require "Fonts"
local utilityPasses = require "utilityPasses"
----------------------------------------

local GlowExampleState = {}

function GlowExampleState:new()
    newObj =
    { 
        currTime = 0, 
        startTime = nil,
        theNextState = "" 
    }
    self.__index = self
    return setmetatable(newObj, self)
end

function GlowExampleState:load()
    print( "GlowExampleState:load" )

	local camera = spark:getCamera()
	camera:cameraPos( 0, 0.125, -1 )
	camera:cameraTarget( 0, 0.125, 0 )
	--camera:cameraPos( 0.2, 1.2, -0.9 )
	--camera:cameraTarget( 0.06, 0.1, 0.0 )
	camera:fov( 35 )

    local frameBufferTarget = spark:getFrameBufferRenderTarget()
	spark:createPostProcessingRenderPass( 0.0, 
	      "MSAAFinalRenderPass",
	      "MainRenderTargetTexture", frameBufferTarget, 
	      "texturedOverlayShader" )

	mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
	spark:setMainRenderTarget( mainRenderTarget )
	mainRenderTarget:setClearColor( vec4( 0.0,0.0,0.0, 1.0 ) )

	opaqueRenderPass = spark:createRenderPass( 1.0, "OpaquePass", mainRenderTarget )
	opaqueRenderPass:setDepthWrite( true )
	opaqueRenderPass:setDepthTest( true )
	opaqueRenderPass:disableBlending()

	glowPass = utilityPasses.setupGlowPass( "GlowRenderPass" )


	local boxMat = spark:createMaterial( "constantColorShader" )
	boxMat:setVec4( "u_color", vec4(1.0,0.5,0.01,1) )

	boxA = spark:createCube( vec3(0.0,0,0), 0.25, boxMat, "OpaquePass" )

	boxB = spark:createCube( vec3(-0.25,0,0), 0.25, boxMat, "GlowRenderPass" )

	boxD = spark:createCube( vec3(-0.125,-.25,0), 0.25, boxMat, "OpaquePass" )
	boxC = spark:createCube( vec3(-0.125,-.25,0), 0.25, boxMat, "GlowRenderPass" )



end

function GlowExampleState:activate()
    print( "GlowExampleState:activate" )
    -- acquire resources release on last deactivate()
    self.startTime = -1

    local camera = spark:getCamera()
    camera:cameraPos( 0.0, 0.5, 3 )
    camera:cameraTarget( 0.0, 0.0, 0.0 )
    camera:cameraUp( 0,1,0 )
end

function GlowExampleState:update( dt )
end

function GlowExampleState:deactivate()
    print( "GlowExampleState:deactivate" )
    -- Release resources not needed until next activate()
    self.startTime = nil
end

function GlowExampleState:nextState( currTime )
    if self.startTime == nil then self.startTime = currTime end
    theNextState = self.theNextState
end

theState = GlowExampleState:new()
theNextState = ""
