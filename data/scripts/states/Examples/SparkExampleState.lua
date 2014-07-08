----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
----------------------------------------

local SparkExampleState = {}

function SparkExampleState:new()
    newObj =
    { 
        angle = 45, 
        startTime = nil,
        theNextState = ""
    }
    self.__index = self
    return setmetatable(newObj, self)
end

function SparkExampleState:load()
    print( "SparkExampleState:load" )
    local isShadowOn = false
    Render:createDefaultRenderPasses( isShadowOn )

    self.boxMat = spark:createMaterial( "colorShader" )
    self.boxMat:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )

    self.boxA = spark:createCube( vec3(-1.0, 0, 0), 0.05, self.boxMat, "OpaquePass" )
    self.boxB = spark:createCube( vec3( 1.0, 0, 0), 0.05, self.boxMat, "OpaquePass" )


    self.sparkMat = spark:createMaterial( "texturedSparkShader" )
    self.sparkIntensity = 1.0
    self.sparkMat:setVec4( "u_color", vec4(1.0, 1.0, 0.5, self.sparkIntensity) )
    self.mySpark = spark:createLSpark( vec3(1.0,0,0), vec3(-1.0, 0, 0),
                                             1.0, --intensity
                                             0.01, --scale 
                                             1, --recursiveDepth
                                             0.0, --forkProb
                                             "TransparentPass",
                                             self.sparkMat )
    self.sparkActivationTime = 0
    self.sparkPeriod = 0.75
end

function SparkExampleState:activate()
    print( "SparkExampleState:activate" )
    -- acquire resources release on last deactivate()
    self.startTime = -1
    self.theNextState = ""

    local camera = spark:getCamera()
    camera:cameraPos( 0.0, 0.345, 3.222 )
    camera:cameraTarget( 0.0, 0.0, 0.0 )
    camera:cameraUp( 0,1,0 )
end

function SparkExampleState:update( dt )

    self.sparkActivationTime = self.sparkActivationTime + dt
    if self.sparkActivationTime > self.sparkPeriod then
        self.sparkActivationTime = 0
        self.mySpark:reseat( vec3(1.0,0,0), vec3(-1.0, 0, 0),
                             1.0, --intensity
                             0.1, --scale 
                             5, --recursiveDepth
                             0.5 --forkProb
                             )
    end
    self.mySpark:update( dt )

end

function SparkExampleState:deactivate()
    print( "SparkExampleState:deactivate" )
    -- Release resources not needed until next activate()
    self.startTime = nil
end

function SparkExampleState:nextState( currTime )
    if self.startTime == nil then self.startTime = currTime end
    theNextState = self.theNextState
end

theState = SparkExampleState:new()
theNextState = ""
