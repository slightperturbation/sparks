-----------------------------
local ESUModel = require "ESUModel"
----------------------------------------
--[[
	Tissue Simulation methods
	Encapsulates the key parts of the tissue simulation
	Can be used by instances of SimulationStates 

	Note that methods take an "owner" as first argument
	which is expected to be the owning SimulationState
	Objects are then stored in that SimulationState.
	TODO -- this violates encapsulation and these methods
	should be confined to the data in the Sim class, but
	it's a bit easier for playing around with this way, until
	building simulation states is settled.
--]]

local Sim = {}


function abs( a ) 
	if( a < 0 ) then return -a end
	return a
end


--[[
	Creates owner.wattDisplay, owner.modeDisplay and owner.activationTimeDisplay
	as text blocks in the "owner" table.
	Also adds text for units and a background tint.
--]]
function Sim.createHUDElements( owner, esuModel )

	local fontMgr = spark:getFontManager()
	local fontDesc = {}
	fontDesc.name = "Sans"
	fontDesc.fontFilename = "HelveticaNeueLight.ttf"
	fontDesc.size = 48
	local smallFontSize = 16

	fontDesc.material = spark:createMaterial( "TextShader" )
	fontDesc.material:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.material:setVec4( "u_color", vec4( 0.8, 0.8, 0.8, 1 ) )

	fontDesc.rolloverMaterial = spark:createMaterial( "TextShader" )
	fontDesc.rolloverMaterial:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.rolloverMaterial:setVec4( "u_color", vec4( 0.8, 0.8, 0.8, 1 ) )

	fontMgr:addFont( fontDesc.name, fontDesc.size, fontDesc.fontFilename )
	fontMgr:addFont( fontDesc.name, smallFontSize, fontDesc.fontFilename )

	-- Wattage
	owner.wattDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             string.format("%2.0f / %2.0f", esuModel.cutWattage, esuModel.coagWattage) )
	owner.wattDisplay:translate( 0.05, 0.985, 0 )
	wattUnitMsg = spark:createText( fontDesc.name, 
		                            smallFontSize, 
		                            fontDesc.material, 
		                            "HUDPass", 
		                            "Watts" )
	wattUnitMsg:translate( 0.01, 0.98, 0 )
	-- Mode
	owner.modeDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             "[ESUMODE]" )
	owner.modeDisplay:translate( 0.33, 0.985, 0.1 )
	-- Activation Time
	owner.activationTimeDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             string.format("%2.1f", owner.activationTime) )
	owner.activationTimeDisplay:translate( 0.64, 0.985, 0 )
	timeUnitMsg = spark:createText( fontDesc.name, 
		                            smallFontSize, 
		                            fontDesc.material, 
		                            "HUDPass", 
		                            "Seconds" )
	timeUnitMsg:translate( 0.685, 0.98, 0 )

	----------------------------------------------
	-- Height from tissue
	owner.distDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             string.format("%2.1f", owner.tissueDistance) )
	owner.distDisplay:translate( 0.91, 0.985, 0 )
	areaUnitMsg = spark:createText( fontDesc.name, 
		                            smallFontSize, 
		                            fontDesc.material, 
		                            "HUDPass", 
		                            "mm" )
	areaUnitMsg:translate( 0.962, 0.98, 0 )

	-- Contact Area
	-- owner.contactAreaDisplay = spark:createText( fontDesc.name, 
 -- 			                             fontDesc.size, 
	-- 		                             fontDesc.material,
	-- 		                             "HUDPass", 
	-- 		                             string.format("%2.1f", owner.contactArea) )
	-- owner.contactAreaDisplay:translate( 0.91, 0.985, 0 )
	-- areaUnitMsg = spark:createText( fontDesc.name, 
	-- 	                            smallFontSize, 
	-- 	                            fontDesc.material, 
	-- 	                            "HUDPass", 
	-- 	                            "mm^2" )
	-- areaUnitMsg:translate( 0.95, 0.98, 0 )

	-- --Highlight GUI w/ quad
	local bgAccentMat = spark:createMaterial( "constantColorShader" )
	bgAccentMat:setVec4( "u_color", vec4( 0, 0, 0, 0.8 ) )
	local bgQuad = spark:createQuad( vec2(0,0.915), vec2(1.0,1.0),
		bgAccentMat, "HUDUnderPass" )
	bgQuad:translate( 0,0,0 )


end

function Sim.createInstructionText( owner, instructions )
	local fontMgr = spark:getFontManager()
	local fontDesc = {}
	fontDesc.name = "Sans"
	fontDesc.fontFilename = "HelveticaNeueLight.ttf"
	fontDesc.size = 32
	fontDesc.material = spark:createMaterial( "TextShader" )
	fontDesc.material:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.material:setVec4( "u_color", vec4( 1, 1, 0.8, 1 ) )

	fontMgr:addFont( fontDesc.name, fontDesc.size, fontDesc.fontFilename )
	owner.instructionText = spark:createText( fontDesc.name, 
									  fontDesc.size, 
									  fontDesc.material,
									  "HUDPass", 
									  instructions )
	owner.instructionText:translate( 0.025, 0.785, 0 )

	-- --Highlight GUI w/ quad
	local bgAccentMat = spark:createMaterial( "constantColorShader" )
	bgAccentMat:setVec4( "u_color", vec4( 0, 0, 0, 0.2 ) )
	local bgQuad = spark:createQuad( 
		vec2( 0.015, 0.4 ), -- position of lower-left corner, 0,0 is bottom-left of screen
		vec2( 0.25,0.5 ),  -- size
		bgAccentMat, "HUDUnderPass" )

end


--[[
	Handle events on State load
--]]
function Sim.load( owner )
	-- specifies where the tissue is relative to world coords
	owner.worldOffset = vec3( 0, -.1, -.1 ) -- -0.0625 ) -- -0.125 )

	-- Create the tissue
	Sim.createTissue( owner, owner.worldOffset )
	owner.table = Sim.createTable( owner, owner.worldOffset )

	-- Add HUD elements to the top of the screen
	Sim.createHUDElements( owner, ESUModel.theESUModel )

	Sim.createInstruments( owner )

	-- set the default instrument
	owner.instrument = owner.hookMesh
end

--[[
	Handle events on State activation
--]]
function Sim.activate( owner )

	-- Initialize the position of the camera based on input device
	local camera = spark:getCamera()
	local inputDeviceName = input:getDefaultDeviceName()
	-- Setup the camera based on the input device

	-- Default view
	camera:cameraPos( 0.0, 0.345, 0.222 )
	--camera:cameraPos( 0.0, 0.125, 0.18 ) -- close-up, good for screen shots
	camera:cameraTarget( 0, -0.02, 0 )
	camera:cameraUp( 0,1,0 )

	if inputDeviceName == "trakStar" then
		camera:cameraPos( 0.0, 0.1, -0.025 )
		camera:cameraTarget( 0.0, -0.1, -0.1 )
		camera:cameraUp( 0,1,0 )
	end
end

--[[
	Handle events on State deactivation
--]]
function Sim.deactivate( owner )
	-- terminate vibration, just in case it's still on
	local inputDeviceName = input:getDefaultDeviceName()
	if( inputDeviceName == "stylus" ) then
		input:stopVibration( inputDeviceName )
	end
end

--[[
	Handle events on State update
--]]
function Sim.update( owner, dt )

	owner.wattDisplay:setText( string.format( "%2.0f / %2.0f", 
		                                     ESUModel.theESUModel.cutWattage, 
		                                     ESUModel.theESUModel.coagWattage) )
	owner.modeDisplay:setText( ESUModel.theESUModel.ESUModeLabels[ ESUModel.theESUModel.mode ] ) 


	-- Get control inputs
	inputDeviceName = input:getDefaultDeviceName()
	if inputDeviceName == "NO_DEFAULT_DEVICE_SPECIFIED" then
		-- ERROR
		print( "Error: "..inputDeviceName )
	end

	local stylusPos = input:getDefaultDevicePosition()
	local stylusMat = input:getDefaultDeviceTransform()

	-- TODO -- screenSpaceOffset is a bit of a hack for getting the zspace world to 
	--  where we want it.  Is overriden when using TrakStar
	--  Should remove this and use only world coords
	local screenSpaceOffset = vec3( 0, 0.25, 0 )

	-- green cube on final pos & orient
	if useMouseCursorCube then 
		-- green as the full, unmodified transform
		owner.markerBox2:setTransform( stylusMat )
		--owner.markerBox2:applyTransform( stylusMat )
		-- red block on base position
		owner.markerBox:setTransform( mat4() )
		owner.markerBox:translate( stylusPos )
	end

	local tissueHeight = owner.worldOffset.y - screenSpaceOffset.y
	if inputDeviceName == "trakStar" then
		-- HACK for ascension trakStar
		tissueHeight = -0.1 -- owner.worldOffset.y - screenSpaceOffset.y = -0.35 != -0.1
	end
	local passDepth = 0.0010
	local useOnlyPosition = false -- for debugging only
	local limitDepth = true

	local isBelowSurface = stylusPos.y < (tissueHeight - passDepth)
	local isNearHolster = stylusPos.x > 0.275
	-- Vibrate when below surface
	if( isBelowSurface and not isNearHolster and not owner.hasVibrated ) then
		input:vibrateForSeconds( inputDeviceName, .15 )
		owner.hasVibrated = true
	else
		input:stopVibration( inputDeviceName )
	end
	-- Reset the vibration flag once above surface
	if( not isBelowSurface ) then
		owner.hasVibrated = false
	end


	------------------------------------------------
	if( useOnlyPosition ) then
		-- Only useful for debugging 
		owner.instrument:setTransform( mat4() )
		owner.instrument:translate( stylusPos )
		owner.instrument:translate( 0,.3,0 )
		owner.instrument:rotate( 120,  vec3(0,0,1) )
		owner.instrument:rotate( 30,  vec3(0,1,0) )
		owner.instrument:scale( 0.002 )
	else
		owner.instrument:setTransform( mat4() )
		owner.instrument:translate( screenSpaceOffset )
		if( mat4_at(stylusMat, 3, 1 ) < (tissueHeight - passDepth) ) then
			mat4_set(stylusMat, 3,1, tissueHeight - passDepth )
			stylusPos = vec3( stylusPos.x, tissueHeight - passDepth, stylusPos.z )
		end 
		owner.instrument:applyTransform( stylusMat )
	end

	-- What is happening here?  x - z axis confusion between the input spaces?
	-- This block should be removed
	if not useOnlyPosition then
		-- zSpace Tracker
		if( inputDeviceName == "stylus" ) then
			owner.instrument:rotate( -90,  vec3(0,1,0) )
		end
		-- TrakStar
		if( inputDeviceName == "trakStar" ) then
				owner.instrument:rotate( 90,  vec3(0,1,0) )
		end
		owner.instrument:scale( 0.002 )
	end 

	local worldStylusPos = vec3( stylusPos.x, stylusPos.y, stylusPos.z )
	worldStylusPos.y = worldStylusPos.y + screenSpaceOffset.y

	--print( "STYLUS POS: " .. worldStylusPos.x .. ", " .. worldStylusPos.y .. ", ".. worldStylusPos.z )
	
	local tissueContactPos = vec3( stylusPos.x, stylusPos.y, stylusPos.z )
	tissueContactPos.y = owner.worldOffset.y -- 0.025
	
	-- Map from world coordinates to Tissue UV coordinates (xpos,ypos)
	local toolTipPos = stylusPos.y
	local distFromTissue = toolTipPos - tissueHeight -- tissueHeight already has worldOffset baked in

	owner.distDisplay:setText( string.format( "%3.0f", math.max(distFromTissue, 0) * 1000.0 ) ) -- convert to mm

	--------------------------------------------------------
	-- Activate ESU

	local isActivated = input:isDefaultDeviceButtonPressed( 0 ) or input:isDefaultDeviceButtonPressed( 1 ) 
	-- if( input:isButtonPressed( "mouse", 0 ) ) then
	-- 	isActivated = true
	-- end
	-- if( input:isButtonPressed( "mouse", 1 ) ) then
	-- 	isActivated = true
	-- end

	-- When using the zSpace, query the buttons on the 
	-- stylus handle as well
	-- if inputDeviceName == "stylus" then
	-- 	if( input:isButtonPressed( "stylus", 0 ) ) then
	-- 		isActivated = true
	-- 	end
	-- 	if( input:isButtonPressed( "stylus", 1 ) ) then
	-- 		isActivated = true
	-- 	end
	-- end

	if( isActivated ) then
		--print( "Activation:\t" .. stylusPos.x .. ",\t\t" .. stylusPos.y .. ",\t\t" .. stylusPos.z .. ",\t\tmat.y=" .. mat4_at(stylusMat, 3, 1 )  )
		-- print( "Activation at " .. ESUModel.theESUModel.cutWattage .. " / " .. ESUModel.theESUModel.coagWattage 
		-- 	.. " watts " .. ESUModel.ESUModeLabels[ESUModel.theESUModel.mode] 
		-- 	.. " at dist " .. abs( toolTipPos - tissueHeight ) )
		local xpos = 2*(stylusPos.x - owner.worldOffset.x) -- tissue has been moved by worldOffset
		local ypos = 2*(stylusPos.z - owner.worldOffset.z) 

		-- radius of contact is determined by the penetration depth
		local radiusOfSparkEffect = 0.002
		local effectiveRadiusOfElectrode = 0.004
		local radiusOfContact = math.max( 
			math.min( effectiveRadiusOfElectrode, -distFromTissue ), 
			radiusOfSparkEffect ) 

		radiusOfContact = 0.002 -- debug override!
		ESUModel.theESUModel:activate( theTissueSim, 
			xpos, ypos,                        -- location of activation
			worldStylusPos,                         -- vec3 tool-tip position
			tissueContactPos,                  -- vec3 positon on tissue nearest to tool
			math.max( 0.0, distFromTissue),    -- positive distance from tissue
			radiusOfContact, dt )

		-- Update the total time reported
		owner.activationTime = owner.activationTime + dt
		txt = string.format("%2.1f", owner.activationTime)
		owner.activationTimeDisplay:setText( txt )

		--TODO play sound
		-- if ESUModel.theESUModel.mode == spark.ESUINPUT_CUT then

		-- end
		-- if ESUModel.theESUModel.mode == spark.ESUINPUT_COAG then

		-- end
		-- if ESUModel.theESUModel.mode == spark.ESUINPUT_BLEND then

		-- end

	end

	-- Debugging -- Cause randomly located activation with current settings
	if( input:isKeyDown( string.byte('X') ) ) then
		local touchThreshold = 0.0004 -- meters
		local sparkThreshold = 0.001 -- meters
		local xpos = 0 + math.random() * 0.2 - 0.1
		local ypos = 0 + math.random() * 0.2 - 0.1
		local distFromTissue = 0.0005
		local radiusOfContact = 0.002
		-- local tissuePos = vec3( xpos/2.0 + owner.worldOffset.x, 0.1 + owner.worldOffset.y, ypos/2.0 + owner.worldOffset.z )
		-- local tipPos = tissuePos + vec3( 0, 0.15 + owner.worldOffset.y, 0 )
		local tissuePos = vec3( xpos/2.0 + owner.worldOffset.x, owner.worldOffset.y, ypos/2.0 + owner.worldOffset.z )
		local tipPos = tissuePos + vec3( 0, 0.025, 0 )

		print( "Spark tip: " .. tipPos.x .. ", " .. tipPos.y .. ", " .. tipPos.z )
		ESUModel.theESUModel:activate( theTissueSim, xpos, ypos, 
			tipPos,                         -- vec3 tool-tip position
			tissuePos,                  -- vec3 positon on tissue nearest to tool
			distFromTissue, radiusOfContact, dt )
	end

 	ESUModel.theESUModel:update( dt )

end

--[[
	Create the background/context objects
--]]
function Sim.createTable( owner, worldOffset )
	owner.clothMat = spark:createMaterial( "phongShader" )
	owner.clothMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
	owner.clothMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
	owner.clothMat:setVec4( "u_ambientLight", vec4(0.1,0.1,0.1,1) )
	owner.clothMat:setVec4( "u_ka", vec4(1,1,1,1) )
	owner.clothMat:setVec4( "u_kd", vec4(1,1,1,1) )
	owner.clothMat:setVec4( "u_ks", vec4(1,1,1,1) )
	owner.clothMat:setFloat( "u_ns", 100.0 )
	owner.clothMat:setFloat( "u_activationTime", 0.0 )
	owner.clothMat:addTexture( "s_color", "bgCloth" )
	owner.clothMat:setVec2( "u_textureRepeat", vec2(4,4) )
	local table = spark:createCube( worldOffset + vec3(-0.5, -0.025, -0.5), 1, owner.clothMat, "OpaquePass" )
	table:rotate( 90, vec3(1,0,0) )

	if( isShadowOn ) then
		owner.table:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end

	return table
end

--[[
	Create the instruments (e.g., lap hook)
--]]
function Sim.createInstruments( owner )
	--Load hook
	local hookMat = spark:createMaterial( "phongShader" )
	hookMat:addTexture( "s_color", "hook_cautery" )
	hookMat:setVec4( "u_light.position_camera", vec4(0,1,1,1) )
	hookMat:setVec4( "u_light.diffuse", vec4(0.4,0.4,0.4,1) )
	hookMat:setVec4( "u_ambientLight", vec4(0.0,0.0,0.0,1) )
	hookMat:setVec4( "u_ks", vec4(1,1,1,1) )
	hookMat:setFloat( "u_ns", 100.0 )
	hookMat:setBool( "u_textureSwapUV", true )
	hookMat:setVec2( "u_textureRepeat", vec2(0.5,1) )


	owner.hookMesh = spark:loadMesh( "hook_cautery_new.3DS", hookMat, "OpaquePass" )
	if( isShadowOn ) then
		owner.hookMesh:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end
	--owner.hookMesh = spark:loadMesh( "hook_cautery_new.3DS", cursorMat, "OpaquePass" )
end


--[[
	Create the tissue model to be simulated
--]]
function Sim.createTissue( owner, worldOffset )
	useHeightMap = true
	if( useHeightMap ) then
		--owner.tissueMat = spark:createMaterial( "tissueShader_debug" )
		--owner.tissueMat = spark:createMaterial( "tissueShader_heightMap" )
		owner.tissueMat = spark:createMaterial( "tissueShader_fast" )
	else
		--tissueShader
		owner.tissueMat = spark:createMaterial( "tissueShader_procedural" )
		owner.tissueMat:setVec2( "u_textureRepeat", vec2(3.3,3.3)) 
	end
	owner.tissueMat:addTexture( "s_shadowMap", "light0_shadowMap" )

	owner.tissueMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
	owner.tissueMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
	owner.tissueMat:setVec4( "u_ambientLight", vec4(0.1,0.1,0.1,1) )
	owner.tissueMat:setVec4( "u_ka", vec4(.1,.1,.1,1) )
	owner.tissueMat:setVec4( "u_kd", vec4(.7,.7,.7,1) )
	owner.tissueMat:setVec4( "u_ks", vec4(1,1,1,1) )
	owner.tissueMat:setFloat( "u_ns", 100.0 )
	owner.tissueMat:setFloat( "u_activationTime", 0.0 )

	owner.tissueMat:addTexture( "s_color", "tissueDiffuse" );
	owner.tissueMat:addTexture( "s_bump", "tissueBump" );
	--owner.tissueMat:addTexture( "s_normal", "tissueNormal" );
	--owner.tissueMat:addTexture( "s_ambient", "tissueAmbient" );
	owner.tissueMat:addTexture( "s_charNormal", "tissueCharNormal" );

	-- Global theTissueSim is the tissue simulation, declared in C++
	owner.tissueMat:addTexture( "s_depthMap", theTissueSim:getVaporizationDepthMapTextureName() )
	--owner.tissueMat:addTexture( "s_temperature", theTissueSim:getTempMapTextureName() )
	owner.tissueMat:addTexture( "s_condition", theTissueSim:getConditionMapTextureName() )

	owner.tissueMat_debug = spark:createMaterial( "tissueShader_debug" )
	--owner.tissueMat_debug:addTexture( "s_temperature", theTissueSim:getTempMapTextureName() )
	owner.tissueMat_debug:addTexture( "s_condition", theTissueSim:getConditionMapTextureName() )
	owner.tissueMat_debug:addTexture( "s_depthMap", theTissueSim:getVaporizationDepthMapTextureName() )
	owner.tissueMat_debug:setVec3( "u_offset", vec3(0, 0.1, 0) )

	local tissueScale = 0.25
	owner.tissue = spark:createPlane( worldOffset + vec3( 0, 0.5*tissueScale, -0.5*tissueScale ), 
		                              vec2(tissueScale, tissueScale), 
		                              ivec2( 512, 512 ), -- faster
		                              --ivec2( 1024, 1024 ), -- nicer
	                                  owner.tissueMat, 
	                                  "OpaquePass" )
	
	-- Wireframe for debugging
	--owner.tissue:setMaterialForPassName( "WirePass", owner.tissueMat_debug )


	if( isShadowOn ) then
		owner.tissue:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end

	-- show the target ring
	owner.tissueMat:setVec2( "u_targetCircleCenter", vec2( 0.4, 0.4 ) )
	owner.tissueMat:setFloat( "u_targetCircleOuterRadius", 0.025 )
	owner.tissueMat:setFloat( "u_targetCircleInnerRadius", 0.024 )
end


return Sim
