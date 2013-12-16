----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Sim = require "Sim"
local ESUModel = require "ESUModel"
----------------------------------------

--[[

--]]

ContactAreaState = {}

function ContactAreaState:new()
    print( "ContactAreaState:new" )
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


function ContactAreaState:load()
    print( "ContactAreaState:load" )

    local isShadowOn = true

    Render:createDefaultRenderPasses( isShadowOn )

    Sim.load( self )

    ESUModel.theESUModel:createSpark()

    Sim.createInstructionText( self, string.format( 
[[
TODO: contact area lesson text...
]] ) )
    self.step = "Settings"

end

function ContactAreaState:activate()
    print( "ContactAreaState:activate" )
    self.startTime = -1

    Sim.activate( self )
end


function ContactAreaState:update( dt )

    -- Convey updates from the UI to the current ESU settings
    ESUModel.theESUModel:updateInput( theESUInput )

    Sim.update( self, dt )

    local shouldPrint = false
    local moveRate = 0.1
    if input:isKeyDown( KEY_UP ) then
        self.sphere:translate( 0, moveRate, 0 )
        shouldPrint = true
    end
    if input:isKeyDown( KEY_DOWN ) then
        self.sphere:translate( 0, -moveRate, 0 )
        shouldPrint = true
    end
    if input:isKeyDown( KEY_RIGHT ) then
        self.sphere:translate( moveRate, 0, 0 )
        shouldPrint = true
    end
    if input:isKeyDown( KEY_LEFT ) then
        self.sphere:translate( -moveRate, 0, 0 )
        shouldPrint = true
    end

    if shouldPrint then
        local p = self.sphere:getTranslation()
        print( string.format( "debug tool pos: %f, %f, %f", 
                              p.x, p.y, p.z ) )
    end
end

function ContactAreaState:deactivate()
    print( "ContactAreaState:deactivate" )
end

function ContactAreaState:nextState( currTime )
    if input:isKeyDown( KEY_KP_ENTER ) then
        self.theNextState = "Menu"
    end

    theNextState = self.theNextState
end

theState = ContactAreaState:new()
theNextState = ""

