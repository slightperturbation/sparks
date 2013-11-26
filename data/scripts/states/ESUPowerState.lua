----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Sim = require "Sim"
local ESUModel = require "ESUModel"
----------------------------------------

function abs( a ) 
    if( a < 0 ) then return -a end
    return a
end


ESUPowerState = {}

function ESUPowerState:new()
    print( "ESUPowerState:new" )
    newObj = 
    { 
        buttons = {}, 
        stylusOffsetStart = vec3(0,0,0),
        stylusOffsetEnd = vec3(0,0,0),
        angle = 45, 
        hasRunOnce = false, 
        hasVibrated = false,
        startTime = -1, 
        ESUModeLabels = { [ESUINPUT_CUT] = "[Cut]", 
                          [ESUINPUT_COAG] = "[Coag]", 
                          [ESUINPUT_BLEND] = "[Blend]" },
        activationTime = 0,
        currTime = 0, 
        contactArea = 0,  -- area of contact (m^2) between tissue and electrode
        theNextState = ""
    }
    self.__index = self
    return setmetatable(newObj, self)
end


function ESUPowerState:load()
    print( "ESUPowerState:load" )

    local isShadowOn = true
    Render:createDefaultRenderPasses( isShadowOn )
    
    wireRenderPass = spark:createRenderPass( 0.95, "WirePass", mainRenderTarget )
    wireRenderPass:setDepthWrite( true )
    wireRenderPass:setDepthTest( true )
    wireRenderPass:disableBlending()
    wireRenderPass:setWireframe( true )

    -- specifies where the tissue is relative to world coords
    self.worldOffset = vec3( 0, -.1, -.1 ) -- -0.0625 ) -- -0.125 )

    -- Create the tissue
    Sim.createTissue( self, self.worldOffset )
    Sim.createTable( self, self.worldOffset )
    
    -- Add HUD elements to the top of the screen
    Sim.createHUDElements( self, ESUModel.theESUModel )


    -- local showSmoke = true
    -- if( showSmoke ) then
    --  theSmokeVolume:translate( self.worldOffset )
    -- end

    --self.mySpark = spark:createLSpark()

    local scale = 0.0025
    local redMat = spark:createMaterial( "constantColorShader" )
    redMat:setVec4( "u_color", vec4( 1, 0.2, 0.2, 1.0) );
    local greenMat = spark:createMaterial( "constantColorShader" )
    greenMat:setVec4( "u_color", vec4( 0.2, 1, 0.2, 1.0) );

    -- Tmp -- 3D mouse cursor
    useMouseCursorCube = false
    if( useMouseCursorCube ) then
        self.markerBox = spark:createCube( vec3( -scale/1.5, -scale/2.0, -scale/2.0 ), 
            scale, redMat, "OpaquePass" )
        if( isShadowOn ) then
            self.markerBox:setMaterialForPassName( "ShadowPass", shadowMaterial )
        end
        
        self.markerBox2 = spark:createCube( vec3( -scale/2.0, -scale/1.5, -scale/2.0 ), 
            scale, greenMat, "OpaquePass" )
        if( isShadowOn ) then
            self.markerBox2:setMaterialForPassName( "ShadowPass", shadowMaterial )
        end
    end

    -- Here's a nice little marker for the origin
    local bool useZeroMarker = false
    if( useZeroMarker ) then
        local scale = 0.01

        local rMat = spark:createMaterial( "constantColorShader" )
        rMat:setVec4( "u_color", vec4( 1, 0.1, 0.1, 0.5 ) )
        local boxX = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
                                      scale, rMat, "WirePass" )
        boxX:scale( vec3(1, 0.1, 0.1) )

        local gMat = spark:createMaterial( "constantColorShader" )
        gMat:setVec4( "u_color", vec4( 0.1, 1, 0.1, 0.5 ) )
        local boxY = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
                                      scale, gMat, "WirePass" )
        boxY:scale( vec3(0.1, 1, 0.1) )

        local bMat = spark:createMaterial( "constantColorShader" )
        bMat:setVec4( "u_color", vec4( 0.1, 0.1, 1, 0.5 ) )
        local boxZ = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
                                      scale, bMat, "WirePass" )
        boxZ:scale( vec3(0.1, 0.1, 1) )
        --box:translate( self.worldOffset ) 
        if( isShadowOn ) then
            boxX:setMaterialForPassName( "ShadowPass", shadowMaterial )
            boxY:setMaterialForPassName( "ShadowPass", shadowMaterial )
            boxZ:setMaterialForPassName( "ShadowPass", shadowMaterial )
        end
    end



    --Load hook
    local metalMat = spark:createMaterial( "phongShader" )
    metalMat:addTexture( "s_color", "hook_cautery" )
    metalMat:setVec4( "u_light.position_camera", vec4(0,0,0,1) )
    metalMat:setVec4( "u_light.diffuse", vec4(0.3,0.3,0.3,1) )
    metalMat:setVec4( "u_ambientLight", vec4(0.4,0.2,0.2,1) )
    metalMat:setVec4( "u_ka", vec4(0.4,0.4,0.4,1) )
    metalMat:setVec4( "u_kd", vec4(1,1,1,1) )
    metalMat:setVec4( "u_ks", vec4(1,1,1,1) )
    metalMat:setFloat( "u_ns", 100.0 )

    self.instrument = spark:loadMesh( "hook_cautery_new.3DS", metalMat, "OpaquePass" )
    if( isShadowOn ) then
        self.instrument:setMaterialForPassName( "ShadowPass", shadowMaterial )
    end
    --self.instrument = spark:loadMesh( "hook_cautery_new.3DS", cursorMat, "OpaquePass" )

    -- self.sparkMat = spark:createMaterial( "texturedSparkShader" )
    -- self.sparkMat:setVec4( "u_color", vec4( 1, 1, 0, 1) )
    -- --sparkMat:addTexture( "s_color")
    -- self.aSpark = spark:createLSpark( self.worldOffset + vec3(0, 0.01, 0), self.worldOffset + vec3(0,0,0), 1, 1, 3, 0.4, "OpaquePass", self.sparkMat )
    -- -- self.aSpark = spark:createLSpark( self.worldOffset + vec3(0, 0.01, 0), self.worldOffset + vec3(0,0,0), 1, 1, 3, 0.4, "TransparentPass", self.sparkMat )


end

function ESUPowerState:activate()
    print( "ESUPowerState:activate" )
    self.startTime = -1

    local camera = spark:getCamera()
    camera:cameraPos( 0.0, 0.345, 0.222 )
    camera:cameraTarget( 0.0, 0.0, 0.0 )
    camera:cameraUp( 0,1,0 )
end


function ESUPowerState:update( dt )
    -- Convey updates from the UI to the current ESU settings
    ESUModel.theESUModel:updateInput( theESUInput )
    ESUModel.theESUModel:update( dt )
    if ESUModel.theESUModel.mode == ESUINPUT_BLEND then
        self.wattDisplay:setText( string.format( "%2.0f / %2.0f", 
                                                 ESUModel.theESUModel.cutWattage, 
                                                 ESUModel.theESUModel.coagWattage) )
    end
    if ESUModel.theESUModel.mode == ESUINPUT_COAG then
        self.wattDisplay:setText( string.format( "%2.0f", 
                                                 ESUModel.theESUModel.coagWattage) )
    end
    if ESUModel.theESUModel.mode == ESUINPUT_CUT then
        self.wattDisplay:setText( string.format( "%2.0f", 
                                                 ESUModel.theESUModel.cutWattage) )
    end
    self.modeDisplay:setText( self.ESUModeLabels[ ESUModel.theESUModel.mode ] ) 


    -- Get control inputs
    if( isWindows() ) then
        inputDeviceName = "stylus"
        --inputDeviceName = "trakStar"
    else
        inputDeviceName = "mouse"
    end

    local stylusPos = input:getPosition( inputDeviceName )
    local stylusMat = input:getTransform( inputDeviceName )

    -- if( inputDeviceName == "trakStar" ) then

    -- end 

    -- if( input:isKeyDown( string.byte('C') ) ) then
    --  print( "stylusOffset.x = " .. stylusPos.x )
    --  print( "stylusOffset.y = " .. stylusPos.y )
    --  print( "stylusOffset.z = " .. stylusPos.z )
    --  self.stylusOffsetStart = stylusPos
    --  self.stylusOffsetEnd = stylusPos
    -- end
    -- if( input:isKeyDown( string.byte('D') ) ) then
    --  self.stylusOffsetEnd = stylusPos
    -- end
    -- if( input:isKeyDown( string.byte('G') ) ) then
    --  self.stylusOffsetStart = vec3(0,0,0)
    --  self.stylusOffsetEnd = vec3(0,0,0)
    -- end

    -- stylusPos = stylusPos + (self.stylusOffsetStart - self.stylusOffsetEnd)

    -- if( isWindows() ) then
    --  stylusPos = input:getPosition( "stylus" )
    --  stylusMat = input:getTransform( "stylus" )
    -- else
    --  -- Debugging (and on non-zspace machines, use mouse)
    --  stylusPos = input:getPosition( "mouse" ) -- TODO -- Transform to coords mimicing stylus?
    --  stylusMat = mat4() --input:getTransform( "mouse" )
    -- end

    local screenSpaceOffset = vec3( 0, 0.25, 0 )
    -- green block on final pos & orient
    if useMouseCursorCube then 
        -- green as the full, unmodified transform
        self.markerBox2:setTransform( stylusMat )
        --self.markerBox2:applyTransform( stylusMat )
        -- red block on base position
        self.markerBox:setTransform( mat4() )
        self.markerBox:translate( stylusPos )
    end
    local floorHeight = self.worldOffset.y - screenSpaceOffset.y
    if inputDeviceName == "trakStar" then
        -- HACK for ascension trakStar
        floorHeight = -0.1 -- self.worldOffset.y - screenSpaceOffset.y
    end
    local passDepth = 0.0010
    local useOnlyPosition = false -- for debugging only
    local limitDepth = true

    local isBelowSurface = stylusPos.y < (floorHeight - passDepth)
    local isNearHolster = stylusPos.x > 0.275
    -- Vibrate when below surface (?)
    if( isBelowSurface and not isNearHolster and not self.hasVibrated ) then
        input:vibrateForSeconds( inputDeviceName, .15 )
        self.hasVibrated = true
    else
        input:stopVibration( inputDeviceName )
    end
    -- Reset the vibration flag once above surface
    if( not isBelowSurface ) then
        self.hasVibrated = false
    end


    ------------------------------------------------
    -- zSpace Tracker
    if( inputDeviceName == "stylus" ) then
        if( useOnlyPosition ) then
            self.instrument:setTransform( mat4() )
            self.instrument:translate( stylusPos )
            self.instrument:translate( 0,.3,0 )
            self.instrument:rotate( 120,  vec3(0,0,1) )
            self.instrument:rotate( 30,  vec3(0,1,0) )
            self.instrument:scale( 0.002 )
        else
            self.instrument:setTransform( mat4() )
            self.instrument:translate( screenSpaceOffset )
            if( mat4_at(stylusMat, 3, 1 ) < (floorHeight - passDepth) ) then
                mat4_set(stylusMat, 3,1, floorHeight - passDepth )
                stylusPos = vec3( stylusPos.x, floorHeight - passDepth, stylusPos.z )
            end 
            self.instrument:applyTransform( stylusMat )
            self.instrument:rotate( -90,  vec3(0,1,0) )
            self.instrument:scale( 0.002 )
        end
    end

    ------------------------------------------------
    -- TrakStar
    if( inputDeviceName == "trakStar" ) then
        if( useOnlyPosition ) then
            self.instrument:setTransform( mat4() )
            self.instrument:translate( stylusPos )
            self.instrument:translate( 0,.3,0 )
            self.instrument:rotate( 120,  vec3(0,0,1) )
            self.instrument:rotate( 30,  vec3(0,1,0) )
            self.instrument:scale( 0.002 )
        else
            self.instrument:setTransform( mat4() )
            if( mat4_at(stylusMat, 3, 1 ) < (floorHeight - passDepth) ) then
                -- force both the position and the transform to honor
                -- the floor height
                mat4_set(stylusMat, 3,1, floorHeight - passDepth )
                stylusPos = vec3( stylusPos.x, floorHeight - passDepth, stylusPos.z )
            end 
            --print( "FloorHeight:\t" .. floorHeight )
            self.instrument:applyTransform( stylusMat )
            self.instrument:rotate( 90,  vec3(0,1,0) )
            self.instrument:scale( 0.002 )


            -- self.instrument:setTransform( mat4() )
            -- --self.instrument:translate( screenSpaceOffset )
            -- -- if( mat4_at(stylusMat, 3, 1 ) < (floorHeight - passDepth) ) then
            -- --   mat4_set(stylusMat, 3,1, floorHeight - passDepth )
            -- --   stylusPos = vec3( stylusPos.x, floorHeight - passDepth, stylusPos.z )
            -- -- end 
            -- self.instrument:applyTransform( stylusMat )
            -- --self.instrument:rotate( -90,  vec3(0,1,0) )
            -- --self.instrument:scale( 0.002 )
        end
    end 

    local isActivated = false
    if( input:isButtonPressed( "mouse", 0 ) ) then
        isActivated = true
    end
    if( input:isButtonPressed( "mouse", 1 ) ) then
        isActivated = true
    end
    if( input:isButtonPressed( "stylus", 0 ) ) then
        isActivated = true
    end

    if( isActivated ) then
        local toolTipPos = stylusPos.y
        --print( "Activation:\t" .. stylusPos.x .. ",\t\t" .. stylusPos.y .. ",\t\t" .. stylusPos.z .. ",\t\tmat.y=" .. mat4_at(stylusMat, 3, 1 )  )
        -- print( "Activation at " .. ESUModel.theESUModel.cutWattage .. " / " .. ESUModel.theESUModel.coagWattage 
        --  .. " watts " .. self.ESUModeLabels[ESUModel.theESUModel.mode] 
        --  .. " at dist " .. abs( toolTipPos - floorHeight ) )
        
        local distFromTissue = toolTipPos - floorHeight -- floorHeight already has worldOffset baked in
        local xpos = 2*(stylusPos.x - self.worldOffset.x) -- tissue has been moved by worldOffset
        local ypos = 2*(stylusPos.z - self.worldOffset.z) 

        -- radius of contact is determined by the penetration depth
        local radiusOfSparkEffect = 0.002
        local effectiveRadiusOfElectrode = 0.004
        local radiusOfContact = math.max( 
            math.min( effectiveRadiusOfElectrode, -distFromTissue ), 
            radiusOfSparkEffect ) 

        radiusOfContact = 0.002 -- debug override!
        ESUModel.theESUModel:activate( theTissueSim, 
            xpos, ypos,                        -- location of activation
            math.max( 0.0, distFromTissue),    -- positive distance from tissue
            radiusOfContact, dt )

        -- Update the total time reported
        self.activationTime = self.activationTime + dt
        txt = string.format("%2.1f", self.activationTime)
        self.activationTimeDisplay:setText( txt )

        --TODO play sound
        -- if ESUModel.theESUModel.mode == spark.ESUINPUT_CUT then

        -- end
        -- if ESUModel.theESUModel.mode == spark.ESUINPUT_COAG then

        -- end
        -- if ESUModel.theESUModel.mode == spark.ESUINPUT_BLEND then

        -- end

    end

    -- Debugging tool
    if( input:isKeyDown( string.byte('X') ) ) then
        local xpos = 0 + math.random() * 0.2 - 0.1
        local ypos = 0 + math.random() * 0.2 - 0.1
        local distFromTissue = 0
        local radiusOfContact = 0.002
        ESUModel.theESUModel:activate( theTissueSim, xpos, ypos, distFromTissue, radiusOfContact, dt )
    end

end

function ESUPowerState:deactivate()
    print( "ESUPowerState:deactivate" )

    -- terminate vibration incase it's still on
    if( isWindows() ) then
        input:stopVibration( "stylus" )
    end
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
    if( isWindows() ) then
        if input:isButtonPressed( "stylus", 2 ) then
            theNextState = "Menu"
        end
    end
end

theState = ESUPowerState:new()
theNextState = ""

