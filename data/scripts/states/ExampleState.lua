local ExampleState = {}

function ExampleState:new()
	  newObj = { angle = 45 }
	  self.__index = self
	  return setmetatable(newObj, self)
end

function ExampleState:load()
    self.boxMat = spark:createMaterial( "colorShader" )
    self.boxMat:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )
    self.boxA = spark:createCube( vec3(0,0,0), vec3(0.25,0.25,0.25), self.boxMat, "OpaquePass" )
    self.boxB = spark:createCube( vec3(-0.5,0,0), vec3(0.25,0.25,0.25), self.boxMat, "OpaquePass" )
    self.boxB:rotate( self.angle, vec3(0,1,0) )
end

function ExampleState:activate()
    local camera = spark:getCamera()
    camera:cameraPos( 0.2, 1.2, -0.9 )
    camera:cameraTarget( 0.06, 0.1, 0.0 )
    camera:fov( 48 )
end

function ExampleState:update( dt )
    self.boxB:rotate( 10, vec3(0,1,0) )
end

function ExampleState:fixedUpdate( dt )
end

function ExampleState:deactivate()
end

function ExampleState:nextState( currTime )
  	if currTime > 20 then
      theNextState = "Menu"
    else
      theNextState = "" -- Keep current state
    end
end

theState = ExampleState:new()
theNextState = ""
