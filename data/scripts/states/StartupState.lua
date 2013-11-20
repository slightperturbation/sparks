--StartupState.lua

local StartupState = {}

function StartupState:new()
	  newObj = {  }
	  self.__index = self
	  return setmetatable(newObj, self)
end

function StartupState:load()
	require "loadShaders"
	require "loadTextures"
end

function StartupState:activate()
end

function StartupState:update( dt )
end

function StartupState:deactivate()
end

function StartupState:nextState( currTime )
	-- theNextState = "Loading"

	--theNextState = "Example"
	--theNextState = "TextureTest"
	--theNextState = "ESUPower"
	theNextState = "Menu"
	--theNextState = "Simulation"
	--theNextState = "Calibration"
end

theState = StartupState:new()
theNextState = ""
