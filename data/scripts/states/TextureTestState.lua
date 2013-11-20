TextureTestState.lua

----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
----------------------------------------

local TextureTestState = {}

function TextureTestState:new()
	  newObj = { angle = 45, startTime = nil }
	  self.__index = self
	  return setmetatable(newObj, self)
end

function TextureTestState:load()
    self.boxMat = spark:createMaterial( "colorShader" )
    self.boxMat:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )
    self.boxA = spark:createCube( vec3(0,0,0), 0.25, self.boxMat, "OpaquePass" )
    self.boxB = spark:createCube( vec3(-0.5,0,0), 0.25, self.boxMat, "OpaquePass" )
    self.boxB:rotate( self.angle, vec3(0,1,0) )
end

function TextureTestState:activate()
    -- acquire resources release on last deactivate()
    local camera = spark:getCamera()
    camera:cameraPos( 0.2, 1.2, -0.9 )
    camera:cameraTarget( 0.06, 0.1, 0.0 )
    camera:fov( 48 )
end

function TextureTestState:update( dt )
    self.boxB:rotate( 10, vec3(0,1,0) )
end

function TextureTestState:deactivate()
    -- Release resources not needed until next activate()
    self.startTime = nil
end

function TextureTestState:nextState( currTime )
    if self.startTime == nil then self.startTime = currTime end
  	if (currTime - self.startTime) > 5 then
      theNextState = "Menu"
    else
      theNextState = "" -- Keep current state
    end
end

theState = TextureTestState:new()
theNextState = ""
