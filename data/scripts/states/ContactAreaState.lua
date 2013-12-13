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


    -- Create an orthogonal view depth map around the tool-tip to 
    -- provide the total 
    if true then
            -- shadowSource = vec3( 0.001, 1, 0 )
            -- --shadowSource = vec3( -2, 2, 1 )
            -- -- Either ortho (=directional light) or perspective (=local/spot light) can be used
            -- shadowCamera = spark:createOrthogonalProjection( -0.3, 0.3,  -- left, right
            --                                                  -0.3, 0.3,  -- bottom, top
            --                                                  -.25, 2,      -- near, far
            --                                                  shadowSource -- from direction 
            --                                                  )
        
        local toolTipPos = vec3( 0, 0, 1 - self.worldOffset.z )
        --local toolTipPos = vec3( self.stylusPos.x, self.stylusPos.z, 1 - self.worldOffset.z )
        local contactAreaRegionSize = vec3( 0.005, 0.005, 0.005 ) -- one cm cube around tooltip
        local contactAreaRegionDirection = vec3( 0.001, 1, 0 ) -- looking up
        self.contactAreaCamera = spark:createOrthogonalProjection( toolTipPos.x - contactAreaRegionSize.x, -- left
                                                                    toolTipPos.x + contactAreaRegionSize.x, -- right
                                                                    toolTipPos.y - contactAreaRegionSize.y, -- bottom
                                                                    toolTipPos.y + contactAreaRegionSize.y, -- top
                                                                    toolTipPos.z + contactAreaRegionSize.z, -- near
                                                                    toolTipPos.z - contactAreaRegionSize.z, -- far
                                                                    contactAreaRegionDirection )

        local contactAreaDepthTarget = spark:createDepthMapRenderTarget( "contactAreaDepthMap", 64, 64 )
        local contactAreaPass = spark:createRenderPassWithProjection( 10.0, "ContactAreaPass", self.contactAreaCamera, contactAreaDepthTarget )

        -- create a material for contact Area rendering
        self.contactAreaMaterial = spark:createMaterial( "contactAreaShader" )

        -- Add the tools to the depth map render
        self.hookMesh:setMaterialForPassName( "ContactAreaPass", self.contactAreaMaterial ) -- self.contactAreaMaterial writes the depth

        -- Dummy sphere tool for testing
        self.testMaterial = spark:createMaterial( "colorShader" )
        self.testMaterial:setVec4( "u_color", vec4(1,0.5,0.5,1.0) )
        self.sphere = spark:loadUpdateableMesh( "sphere.obj", self.testMaterial, "OpaquePass" )
        self.sphere:setMaterialForPassName( "ContactAreaPass", self.contactAreaMaterial ) -- casts shadow
        self.sphere:translate( 0, 0, 0 )
        self.sphere:scale( 0.01 )

        -- Debugging-- render as a texture
        -- A standard shader for direct painting
        self.depthMapRenderMaterial = spark:createMaterial( "texturedOverlayShader" )
        self.depthMapRenderMaterial:addTexture( "s_color", "contactAreaDepthMap" )
        -- The quad to show the PIP using the new material
        local pipSize = 0.3
        self.pipQuad = spark:createQuad( vec2( 0.05, 0.05 ), 
                                         vec2( pipSize*(9/16), pipSize ), -- 9/16 is aspect ratio of display 
                                         self.depthMapRenderMaterial, "HUDPass" )
    end

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

    if input:isKeyDown( KEY_UP ) then
        self.sphere:translate( 0, 0.05, 0 )
    end
    if input:isKeyDown( KEY_DOWN ) then
        self.sphere:translate( 0, -0.05, 0 )
    end
    if input:isKeyDown( KEY_RIGHT ) then
        self.sphere:translate( 0.05, 0, 0 )
    end
    if input:isKeyDown( KEY_LEFT ) then
        self.sphere:translate( -0.05, 0, 0 )
    end

    --local toolTipPos = vec3( 0, 0, 1 - self.worldOffset.z )
    --print( "x="..self.stylusPos.x..", z="..self.stylusPos.z )
    local toolTipPos = vec3( -self.stylusPos.z, -self.stylusPos.x, 1 - self.worldOffset.z )
    local contactAreaRegionSize = vec3( 0.02, 0.02, 0.1 ) -- one cm cube around tooltip
    local contactAreaRegionDirection = vec3( 0.001, 1, 0 ) -- looking up
    spark:updateOrthogonalProjection( self.contactAreaCamera,
                                      toolTipPos.x - contactAreaRegionSize.x, -- left
                                      toolTipPos.x + contactAreaRegionSize.x, -- right
                                      toolTipPos.y - contactAreaRegionSize.y, -- bottom
                                      toolTipPos.y + contactAreaRegionSize.y, -- top
                                      toolTipPos.z + contactAreaRegionSize.z, -- near
                                      toolTipPos.z - contactAreaRegionSize.z, -- far
                                      contactAreaRegionDirection )
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

