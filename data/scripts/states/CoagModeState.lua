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

    self.targetWattage = 30
    Sim.createInstructionText( self, string.format( 
[[
Change the MODE setting on the ESU 
touchscreen to "COAG" mode (blue),
 
And set the power to %d watts.
 
(Waiting...)
]], self.targetWattage ) )
    self.step = "Settings"

    -- tissueMat:setVec2( "u_targetCircleCenter", vec2( 0.4, 0.4 ) )
    -- tissueMat:setFloat( "u_targetCircleOuterRadius", 0.025 )
    -- tissueMat:setFloat( "u_targetCircleInnerRadius", 0.024 )

end

function CoagModeState:activate()
    print( "CoagModeState:activate" )
    self.startTime = -1

    Sim.activate( self )
end


function CoagModeState:update( dt )
    -- Convey updates from the UI to the current ESU settings
    ESUModel.theESUModel:updateInput( theESUInput )

    if self.step == "Settings" 
        and ESUModel.theESUModel.mode == ESUINPUT_COAG 
        and ESUModel.theESUModel.coagWattage == self.targetWattage then
            self.instructionText:setText( 
[[
The ESU is now in COAG mode.
 
Now use the tool to dessicate
the circular region marked on the tissue.
 
Try increasing and decreasing the
ESU wattage and notice the difference
in height from the tissue that the 
electricity arcs.
 
Try switching to CUT mode to
see the difference in arcing
distance vs COAG mode.
 
Press "Enter" when finished.
]])
    end

    Sim.update( self, dt )
end

function CoagModeState:deactivate()
    print( "CoagModeState:deactivate" )
end

function CoagModeState:nextState( currTime )
    if input:isKeyDown( KEY_KP_ENTER ) then
        theNextState = "Menu"
    end

    if input:isButtonPressed( "stylus", 2 ) then
        theNextState = "Menu"
    end
end

theState = CoagModeState:new()
theNextState = ""

