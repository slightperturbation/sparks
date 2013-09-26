----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Sim = require "Sim"
----------------------------------------

function abs( a ) 
	if( a < 0 ) then return -a end
	return a
end


SimulationState = {}

function SimulationState:new()
	print( "SimulationState:new" )
	newObj = 
	{ 
		buttons = {}, 
		angle = 45, 
		hasRunOnce = false, 
		hasVibrated = false,
 		startTime = -1, 
		currWattage = 20, 
		currMode = ESUINPUT_COAG,
		ESUModeLabels = { [ESUINPUT_CUT] = "[Cut]", 
		                  [ESUINPUT_COAG] = "[Coag]", 
		                  [ESUINPUT_BLEND] = "[Blend]" },
		activationTime = 0,
		currTime = 0, 
		theNextState = ""
	}
	self.__index = self
	return setmetatable(newObj, self)
end


function SimulationState:load()
	print( "SimulationState:load" )

	local isShadowOn = true
	Render:createDefaultRenderPasses( isShadowOn )


	-- specifies where the tissue is relative to world coords
	self.worldOffset = vec3( 0, -.1, -.1 ) -- -0.0625 ) -- -0.125 )

	-- Create the tissue
	Sim.createTissue( self, self.worldOffset )
	-- Add HUD elements to the top of the screen
	Sim.createHUDElements( self )


	-- local showSmoke = true
	-- if( showSmoke ) then
	-- 	theSmokeVolume:translate( self.worldOffset )
	-- end

	--self.mySpark = spark:createLSpark()

	local scale = 0.02
	local cursorMat = spark:createMaterial( "constantColorShader" )
	cursorMat:setVec4( "u_color", vec4(1,0.2,0.2,1.0) );

	-- Tmp -- 3D mouse cursor
	useMouseCursorCube = false
	if( useMouseCursorCube ) then
		self.markerBox = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
			scale, cursorMat, "OpaquePass" )
		if( isShadowOn ) then
			self.markerBox:setMaterialForPassName( "ShadowPass", shadowMaterial )
		end
		
		local cursorMat2 = spark:createMaterial( "constantColorShader" )
		cursorMat2:setVec4( "u_color", vec4( 0.2,1,0.2,1.0) );
		self.markerBox2 = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
			scale, cursorMat2, "OpaquePass" )
		if( isShadowOn ) then
			self.markerBox2:setMaterialForPassName( "ShadowPass", shadowMaterial )
		end
	end

	-- Here's a nice little marker for the origin
	local bool useZeroMarker = false
	if( useZeroMarker ) then
		local debugMat = spark:createMaterial( "colorShader" )
		debugMat.name = "DebugMaterial"
		debugMat:setVec4( "u_color", vec4( 1, 1, 1, 1 ) )
		local scale = 0.03
		local box = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
			                          scale, debugMat, "OpaquePass" )
		--box:translate( self.worldOffset )	
		if( isShadowOn ) then
			box:setMaterialForPassName( "ShadowPass", shadowMaterial )
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
	-- self.aSpark = spark:createLSpark( self.worldOffset + vec3(0, 0.01, 0), self.worldOffset + vec3(0,0,0), 1, 1, 3, 0.4, "TransparentPass", self.sparkMat )

end

function SimulationState:activate()
	print( "SimulationState:activate" )
	self.startTime = -1

	local camera = spark:getCamera()
	camera:cameraPos( 0.0, 0.345, 0.222 )
	camera:cameraTarget( 0.0, 0.0, 0.0 )
	camera:cameraUp( 0,1,0 )
end


function SimulationState:update( dt )
	-- self.aSpark = spark:createLSpark( self.worldOffset + vec3(0, 0.01, 0), self.worldOffset + vec3(0,0,0), 1, 1, 3, 0.4, "TransparentPass", self.sparkMat )

	-- Report current ESU settings
	self.currWattage = esuInput:wattage()
	self.currMode = esuInput:mode()

	self.wattDisplay:setText( string.format("%2.0f", self.currWattage) )
	self.modeDisplay:setText( self.ESUModeLabels[ self.currMode ] ) 

	-- Get control inputs
	local stylusPos = vec3()
	local stylusMat = mat4()

	if( isWindows() ) then
		stylusPos = input:getPosition( "stylus" )
		stylusMat = input:getTransform( "stylus" )
	else
		-- Debugging (and on non-zspace machines, use mouse)
		stylusPos = input:getPosition( "mouse" ) -- TODO -- Transform to coords mimicing stylus?
		stylusMat = mat4() --input:getTransform( "mouse" )
	end

	local screenSpaceOffset = vec3( 0, 0.25, 0 )
	-- green block on final pos & orient
	if useMouseCursorCube then 
		self.markerBox2:setTransform( mat4() )
		self.markerBox2:translate( screenSpaceOffset )
		self.markerBox2:applyTransform( stylusMat )
		-- red block on base position
		self.markerBox:setTransform( mat4() )
		self.markerBox:translate( stylusPos )
	end
	
	local floorHeight = self.worldOffset.y - screenSpaceOffset.y
	local passDepth = 0.0010
	local useOnlyPosition = false -- for debugging
	local limitDepth = true

	local isBelowSurface = stylusPos.y < (floorHeight - passDepth)
	local isNearHolster = stylusPos.x > 0.275
	-- Vibrate when below surface (?)
	if( isBelowSurface and not isNearHolster and not self.hasVibrated ) then
		input:vibrateForSeconds( "stylus", .15 )
		self.hasVibrated = true
	else
		input:stopVibration( "stylus" )
	end
	-- Reset the vibration flag once above surface
	if( not isBelowSurface ) then
		self.hasVibrated = false
	end

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


	local touchThreshold = 0.0050  -- visual tweak
	local sparkThreshold = 0.01  -- depends on the mode (voltage, freq) and electrode
	if( self.currMode == ESUINPUT_CUT ) then
		sparkThreshold = 0.0065
	end
	if( self.currMode == ESUINPUT_BLEND ) then
		sparkThreshold = 0.0075
	end

	local isActivated = false
	if( isWindows() ) then
		if( input:isButtonPressed( "stylus", 0 ) ) then
			isActivated = true
		end
	else
		if( input:isButtonPressed( "mouse", 0 ) ) then
			-- hack to allow testing with mouse
			stylusPos.y = floorHeight
			isActivated = true
		end
	end

	if( isActivated ) then
		local toolTipPos = stylusPos.y
		print( "Activation at " .. self.currWattage .. " watts " .. self.ESUModeLabels[self.currMode] .. " at dist " .. abs( toolTipPos - floorHeight ) )
		
		local distFromTissue = toolTipPos - floorHeight
		
		--if abs( distFromTissue ) < touchThreshold then
			-- Contact heating
			-- stylusPos is directly from zSpace, need to transform it to the "world"
			
			--theTissueSim:accumulateHeat( 2*(stylusPos.x - self.worldOffset.x), 2*(stylusPos.z - self.worldOffset.z), self.currWattage ) 
		
		--else
		if abs(distFromTissue) < sparkThreshold then
			-- Non-Contact heating
			-- create visual spark from worldCoord  
			local spreadAngle = 90.0 * math.pi / 180.0 -- 30 degrees
			local spreadSlope = math.tan( spreadAngle * 0.5 )
			-- multi-sample the spark hits
			sampleCount = 8
			for count = 0, sampleCount do
				local xpos = 2*(stylusPos.x - self.worldOffset.x) + distFromTissue * math.random() * spreadSlope
				local ypos = 2*(stylusPos.z - self.worldOffset.z) + distFromTissue * math.random() * spreadSlope
				theTissueSim:accumulateHeat( xpos, ypos, self.currWattage/sampleCount ) 
			end
		end

		self.activationTime = self.activationTime + dt
		txt = string.format("%2.1f", self.activationTime)
		self.activationTimeDisplay:setText( txt )


		--TODO play sound

	end
end

function SimulationState:deactivate()
	print( "SimulationState:deactivate" )

	-- terminate vibration incase it's still on
	input:stopVibration( "stylus" )
end

function SimulationState:nextState( currTime )
	theNextState = self.theNextState
	-- For now, theNextState global is used to pass
	-- the next desired state back to the app
	-- TODO should be changed to use the return value
	-- print( "SimulationState:nextState( " .. currTime .. " )")
	--self.activationText:translate( 0.01, 0.65, 0 )
	-- if (currTime - self.startTime) > 10 then 
	-- 	theNextState = "Loading" 
	-- 	print( "Changing state to menu!" )
	-- else
	-- 	theNextState = ""
	-- end
	if input:isButtonPressed( "stylus", 2 ) then
		theNextState = "Menu"
	end
end

theState = SimulationState:new()
theNextState = ""

