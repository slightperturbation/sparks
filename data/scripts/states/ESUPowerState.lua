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

    self.targetWattage = 20
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

function ESUPowerState:activate()
    print( "ESUPowerState:activate" )
    self.startTime = -1

    Sim.activate( self )
end


function ESUPowerState:update( dt )
    -- Convey updates from the UI to the current ESU settings
    ESUModel.theESUModel:updateInput( theESUInput )

    if self.step == "Settings" 
        and ESUModel.theESUModel.mode == self.targetMode 
        and ESUModel.theESUModel.cutWattage == self.targetWattage then
            self.instructionText:setText( string.format(
[[
The ESU is now in CUT mode
and at %d watts power setting.
 
Now use the tool to vaporize
the circular region marked on the tissue.
 
Try increasing and decreasing the
ESU wattage and notice that the
tissue changes are dependent
on the power setting of the ESU.
 
Press "Enter" when finished.
]], self.targetWattage) )
    end

    Sim.update( self, dt )
end

function ESUPowerState:deactivate()
    print( "ESUPowerState:deactivate" )
end

function ESUPowerState:nextState( currTime )
    if input:isKeyDown( KEY_KP_ENTER ) then
        self.theNextState = "Menu"
    end

    if input:isButtonPressed( "stylus", 2 ) then
        self.theNextState = "Menu"
    end
    theNextState = self.theNextState
end

theState = ESUPowerState:new()
theNextState = ""

