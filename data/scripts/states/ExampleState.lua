----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
----------------------------------------

local ExampleState = {}

function ExampleState:new()
	  newObj = { angle = 45, startTime = nil }
	  self.__index = self
	  return setmetatable(newObj, self)
end

function ExampleState:load()
    print( "ExampleState:load" )
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
                                             0.25, --scale 
                                             3, --recursiveDepth
                                             0.5, --forkProb
                                             "TransparentPass",
                                             self.sparkMat )
    self.sparkActivationTime = 0
    self.sparkPeriod = 0.75
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

    self.sparkActivationTime = self.sparkActivationTime + dt
    if self.sparkActivationTime > self.sparkPeriod then
        self.sparkActivationTime = 0
        self.mySpark:reseat( vec3(1.0,0,0), vec3(-1.0, 0, 0),
                             1.0, --intensity
                             0.25, --scale 
                             3, --recursiveDepth
                             0.5 --forkProb
                             )
    end
    self.mySpark:update( dt )

end

function ExampleState:deactivate()
    print( "ExampleState:deactivate" )
    -- Release resources not needed until next activate()
    self.startTime = nil
end

function ExampleState:nextState( currTime )
    if self.startTime == nil then self.startTime = currTime end
  	-- if (currTime - self.startTime) > 5 then
   --    theNextState = "Menu"
   --  else
   --    theNextState = "" -- Keep current state
   --  end
end

theState = ExampleState:new()
theNextState = ""
