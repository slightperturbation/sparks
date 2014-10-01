--StartupState.lua

local StartupState = {}

function StartupState:new()
	  newObj = { 
	  	theNextState = ""
	  }
	  self.__index = self
	  return setmetatable(newObj, self)
end

function StartupState:load()
	require "loadShaders"
	require "loadTextures"
	self.Fonts = require "Fonts"
	self.Fonts:init()
end

function StartupState:activate()
	print( "StartupState:activate()" )
	self.theNextState = ""
end

function StartupState:update( dt )
end

function StartupState:deactivate()
	print( "StartupState:deactivate()" )
end

function StartupState:nextState( currTime )
	--self.theNextState = "Loading"

	--self.theNextState = "ContactArea"
	
	--self.theNextState = "Example"
	--self.theNextState = "ShaderExample"
	--self.theNextState = "GlowExample"
	--self.theNextState = "ShadowTest"
	--self.theNextState = "SparkExample"

	--self.theNextState = "Simulation"

	self.theNextState = "ShadowExample"

	--self.theNextState = "Menu"
	--self.theNextState = "CoagMode"
	--self.theNextState = "Simulation"
	--self.theNextState = "Example"
	--self.theNextState = "TextureTest"
	--self.theNextState = "ESUPower"
	--self.theNextState = "Calibration"

	theNextState = self.theNextState
end

theState = StartupState:new()
theNextState = ""
