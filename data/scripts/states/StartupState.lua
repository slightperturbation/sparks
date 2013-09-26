--StartupState.lua

local StartupState = {}

function StartupState:new()
	  newObj = {  }
	  self.__index = self
	  return setmetatable(newObj, self)
end

function StartupState:load()
end

function StartupState:activate()
end

function StartupState:update( dt )
end

function StartupState:deactivate()
end

function StartupState:nextState( currTime )
	--theNextState = "Menu"
	theNextState = "Menu"
end

theState = StartupState:new()
theNextState = ""
