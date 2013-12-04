----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Sim = require "Sim"
local ESUModel = require "ESUModel"
----------------------------------------

--[[

--]]

CutModeState = {}

function CutModeState:new()
    print( "CutModeState:new" )
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


function CutModeState:load()
    print( "CutModeState:load" )

    local isShadowOn = true

    Render:createDefaultRenderPasses( isShadowOn )

    Sim.load( self )

    ESUModel.theESUModel:createSpark()

    self.targetWattage = 30
    self.targetMode = ESUINPUT_CUT
    Sim.createInstructionText( self, string.format( 
[[
Change the MODE setting on the ESU 
touchscreen to "CUT" mode (yellow),
 
And set the power to %d watts.
 
(Waiting...)
]], self.targetWattage ) )
    self.step = "Settings"

end

function CutModeState:activate()
    print( "CutModeState:activate" )
    self.startTime = -1

    Sim.activate( self )
end


function CutModeState:update( dt )
    -- Convey updates from the UI to the current ESU settings
    ESUModel.theESUModel:updateInput( theESUInput )

    if self.step == "Settings" 
        and ESUModel.theESUModel.mode == self.targetMode 
        and ESUModel.theESUModel.cutWattage == self.targetWattage then
            self.instructionText:setText( 
[[
The ESU is now in CUT mode.
 
Now use the tool to vaporize
the circular region marked on the tissue.
 
Try increasing and decreasing the
ESU wattage and notice the difference
in height from the tissue that the 
electricity arcs.
 
Try switching to COAG mode to
see the difference in arcing
distance vs CUT mode.
 
Press "Enter" when finished.
]])
    end

    Sim.update( self, dt )
end

function CutModeState:deactivate()
    print( "CutModeState:deactivate" )
end

function CutModeState:nextState( currTime )
    if input:isKeyDown( KEY_KP_ENTER ) then
        theNextState = "Menu"
    end

    if input:isButtonPressed( "stylus", 2 ) then
        theNextState = "Menu"
    end
end

theState = CutModeState:new()
theNextState = ""

