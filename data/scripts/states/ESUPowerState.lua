----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Sim = require "Sim"
local ESUModel = require "ESUModel"
----------------------------------------

--[[

--]]

ESUPowerState = {}

function ESUPowerState:new()
    print( "ESUPowerState:new" )
    newObj = 
    { 
        buttons = {}, 
        hasVibrated = false,
        startTime = -1, 
        activationTime = 0,
        currTime = 0, 
        contactArea = 0,  -- area of contact (m^2) between tissue and electrode
        tissueDistance = 0,  -- distance to tissue in mm
        theNextState = "",

    }
    self.__index = self
    return setmetatable(newObj, self)
end


function ESUPowerState:load()
    print( "ESUPowerState:load" )

    local isShadowOn = true

    Render:createDefaultRenderPasses( isShadowOn )

    Sim.load( self )

    ESUModel.theESUModel:createSpark()

    Sim.createInstructionText( owner, 
[[Set power to 60 Watts
]])

end

function ESUPowerState:activate()
    print( "ESUPowerState:activate" )
    self.startTime = -1

    Sim.activate( self )
end


function ESUPowerState:update( dt )
    -- Convey updates from the UI to the current ESU settings
    ESUModel.theESUModel:updateInput( theESUInput )

    Sim.update( self, dt )
end

function ESUPowerState:deactivate()
    print( "ESUPowerState:deactivate" )
end

function ESUPowerState:nextState( currTime )
    theNextState = self.theNextState
    -- For now, theNextState global is used to pass
    -- the next desired state back to the app
    -- TODO should be changed to use the return value
    -- print( "ESUPowerState:nextState( " .. currTime .. " )")
    --self.activationText:translate( 0.01, 0.65, 0 )
    -- if (currTime - self.startTime) > 10 then 
    --  theNextState = "Loading" 
    --  print( "Changing state to menu!" )
    -- else
    --  theNextState = ""
    -- end
    if input:isButtonPressed( "stylus", 2 ) then
        theNextState = "Menu"
    end
end

theState = ESUPowerState:new()
theNextState = ""

