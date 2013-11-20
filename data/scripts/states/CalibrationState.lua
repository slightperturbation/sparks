----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render" -- provides globals for shadows
----------------------------------------

CalibrationState = {}

function CalibrationState:new()
    print("CalibrationState:new")
    newObj = { startTime = nil }
	self.__index = self
	return setmetatable(newObj, self)
end

function CalibrationState:load()
    print("CalibrationState:load")
    local isShadowOn = false
    Render:createDefaultRenderPasses( isShadowOn )

    self.boxMatRed = spark:createMaterial( "colorShader" )
    self.boxMatRed:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )
    self.boxPos = spark:createCube( vec3(0,0,0), 0.25, self.boxMatRed, "OpaquePass" )
    if( isShadowOn ) then
        self.boxPos:setMaterialForPassName( "ShadowPass", shadowMaterial )
    end
    
    self.boxMatBlue = spark:createMaterial( "colorShader" )
    self.boxMatBlue:setVec4( "u_color", vec4(0.3,0.3,1.0,1.0) )
    self.boxTracked = spark:createCube( vec3(0,0,0), 0.25, self.boxMatBlue, "OpaquePass" )
    if( isShadowOn ) then
        self.boxTracked:setMaterialForPassName( "ShadowPass", shadowMaterial )
    end

        -- Here's a nice little marker for the origin
    local bool useZeroMarker = true
    if( useZeroMarker ) then
        local scale = 0.01

        local rMat = spark:createMaterial( "constantColorShader" )
        rMat:setVec4( "u_color", vec4( 1, 0.1, 0.1, 0.5 ) )
        local boxX = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
                                      scale, rMat, "OpaquePass" )
        boxX:scale( vec3(1, 0.1, 0.1) )

        local gMat = spark:createMaterial( "constantColorShader" )
        gMat:setVec4( "u_color", vec4( 0.1, 1, 0.1, 0.5 ) )
        local boxY = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
                                      scale, gMat, "OpaquePass" )
        boxY:scale( vec3(0.1, 1, 0.1) )

        local bMat = spark:createMaterial( "constantColorShader" )
        bMat:setVec4( "u_color", vec4( 0.1, 0.1, 1, 0.5 ) )
        local boxZ = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
                                      scale, bMat, "OpaquePass" )
        boxZ:scale( vec3(0.1, 0.1, 1) )
        --box:translate( self.worldOffset ) 
        if( isShadowOn ) then
            boxX:setMaterialForPassName( "ShadowPass", shadowMaterial )
            boxY:setMaterialForPassName( "ShadowPass", shadowMaterial )
            boxZ:setMaterialForPassName( "ShadowPass", shadowMaterial )
        end
    end
end

function CalibrationState:activate()
    -- acquire resources release on last deactivate()
    local camera = spark:getCamera()
    camera:cameraPos( 0.2, 1.2, -0.9 )
    camera:cameraTarget( 0.06, 0.1, 0.0 )
    camera:fov( 48 )
end

function CalibrationState:update( dt )
    local trackingDeviceName = "trakStar"

    local trackingPos = input:getPosition( trackingDeviceName )
    local trackingMat = input:getTransform( trackingDeviceName )

    self.boxPos:setTransform( mat4() ) -- reset transform
    self.boxPos:translate( trackingPos )

    self.boxTracked:setTransform( mat4() )
    self.boxTracked:applyTransform( trackingMat )
end

function CalibrationState:deactivate()
    -- Release resources not needed until next activate()
    self.startTime = nil
end

function CalibrationState:nextState( currTime )
    theNextState = ""
end

theState = CalibrationState:new()
theNextState = ""
