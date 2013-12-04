CoagModeState.lua
----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Sim = require "Sim"
local ESUModel = require "ESUModel"
----------------------------------------

--[[

--]]

CoagModeState = {}

function CoagModeState:new()
    print( "CoagModeState:new" )
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


function CoagModeState:load()
    print( "CoagModeState:load" )

    local isShadowOn = true

    Render:createDefaultRenderPasses( isShadowOn )

    Sim.load( self )

    ESUModel.theESUModel:createSpark()

    Sim.createInstructionText( owner, 
[[Set the ESU to CUT mode
]])

end

function CoagModeState:activate()
    print( "CoagModeState:activate" )
    self.startTime = -1

    Sim.activate( self )
end


function CoagModeState:update( dt )
    -- Convey updates from the UI to the current ESU settings
    ESUModel.theESUModel:updateInput( theESUInput )

    Sim.update( self, dt )
end

function CoagModeState:deactivate()
    print( "CoagModeState:deactivate" )
end

function CoagModeState:nextState( currTime )
    theNextState = self.theNextState
    -- For now, theNextState global is used to pass
    -- the next desired state back to the app
    -- TODO should be changed to use the return value
    -- print( "CoagModeState:nextState( " .. currTime .. " )")
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

theState = CoagModeState:new()
theNextState = ""

