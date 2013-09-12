

local Button = require "button"
-----------------------------

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
		startTime = -1, 
		currWattage = 200, 
		activationTime = 0,
		currTime = 0 
	}
	self.__index = self
	return setmetatable(newObj, self)
end

function incrWattage( simState )
	print( "Change wattage up" )
	if( simState.currWattage < 65 ) then
		simState.currWattage = simState.currWattage + 10
		print( "New Wattage: "..simState.currWattage )
		simState.buttons["wattage"].text:setText( string.format("%2.0f", simState.currWattage) )
	end
end
function decrWattage( simState )
	print( "Change wattage up" )
	if( simState.currWattage > 15 ) then
		simState.currWattage = simState.currWattage - 10
		print( "New Wattage: "..simState.currWattage )
		simState.buttons["wattage"].text:setText( string.format("%2.0f", simState.currWattage) )
	end
end

function SimulationState:load()
	print( "SimulationState:load" )

	spark:createPostProcessingRenderPass( 0.0, 
	      "MSAAFinalRenderPass",
	      "MainRenderTargetTexture", spark:getFrameBufferRenderTarget(), 
	      "texturedOverlayShader" )
 
	mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
	spark:setMainRenderTarget( mainRenderTarget )
	mainRenderTarget:setClearColor( vec4( 0.1, 0.05, 0.05, 1.0 ) )

	opaqueRenderPass = spark:createRenderPass( 1.0, "OpaquePass", mainRenderTarget )
	opaqueRenderPass:setDepthWrite( true )
	opaqueRenderPass:setDepthTest( true )
	opaqueRenderPass:disableBlending()

	transRenderPass = spark:createRenderPass( 0.9, "TransparentPass", mainRenderTarget )
	transRenderPass:setDepthWrite( false )
	transRenderPass:setDepthTest( true )
	transRenderPass:useInterpolatedBlending()

	HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", mainRenderTarget )
	HUDRenderPass:setDepthTest( false )
	HUDRenderPass:setDepthWrite( false )
	HUDRenderPass:useInterpolatedBlending()
	
	-- specifies where the tissue is relative to world coords
	self.worldOffset = vec3( 0, -.1, -0.125 )
	local showTable = true
	if( showTable ) then
		self.clothMat = spark:createMaterial( "phongShader" )
		self.clothMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
		self.clothMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
		self.clothMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
		self.clothMat:setVec4( "u_ka", vec4(1,1,1,1) )
		self.clothMat:setVec4( "u_kd", vec4(1,1,1,1) )
		self.clothMat:setVec4( "u_ks", vec4(1,1,1,1) )
		self.clothMat:setFloat( "u_ns", 100.0 )
		self.clothMat:setFloat( "u_activationTime", 0.0 )
		self.clothMat:addTexture( "s_color", "cloth" );
		self.clothMat:setVec2( "u_textureRepeat", vec2(4,4) )
		local table = spark:createCube(self.worldOffset + vec3(-0.5, -0.025, -0.5), 1, self.clothMat, "OpaquePass" )
		table:rotate( 90, vec3(1,0,0) )
	end

	local showTissue = true
	if showTissue  then
		self.tissueMat = spark:createMaterial( "tissueShader" ) --"tissueShader_procedural"  )
		self.tissueMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
		self.tissueMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
		self.tissueMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
		self.tissueMat:setVec4( "u_ka", vec4(1,1,1,1) )
		self.tissueMat:setVec4( "u_kd", vec4(1,1,1,1) )
		self.tissueMat:setVec4( "u_ks", vec4(1,1,1,1) )
		self.tissueMat:setFloat( "u_ns", 100.0 )
		self.tissueMat:setFloat( "u_activationTime", 0.0 )

		self.tissueMat:addTexture( "s_color", "tissueDiffuse" );
		self.tissueMat:addTexture( "s_bump", "tissueBump" );
		self.tissueMat:addTexture( "s_normal", "tissueNormal" );
		self.tissueMat:addTexture( "s_ambient", "tissueAmbient" );
		self.tissueMat:addTexture( "s_temperature", theTissueSim:getTempMapTextureName() )
		self.tissueMat:addTexture( "s_condition", theTissueSim:getConditionMapTextureName() )

		self.tissueMat_debug = spark:createMaterial( "tissueShader_debug" )
		self.tissueMat_debug:addTexture( "s_temperature", theTissueSim:getTempMapTextureName() )
		self.tissueMat_debug:addTexture( "s_condition", theTissueSim:getConditionMapTextureName() )

		-- Global theTissueSim is the tissue simulation, declared in C++
		self.tissueMat:addTexture( "s_temperature", theTissueSim:getTempMapTextureName() )
		self.tissueMat:addTexture( "s_condition", theTissueSim:getConditionMapTextureName() )
		self.tissue = spark:createCube( self.worldOffset + vec3(-0.25, 0, -0.25), 0.5, self.tissueMat, "OpaquePass" )
		self.tissue:rotate( 90, vec3(1,0,0) )


		-- show the target ring
		self.tissueMat:setVec2( "u_targetCircleCenter", vec2( 0.7, 0.6 ) )
		self.tissueMat:setFloat( "u_targetCircleOuterRadius", 0.025 )
		self.tissueMat:setFloat( "u_targetCircleInnerRadius", 0.022 )
	end
	-- local showSmoke = true
	-- if( showSmoke ) then
	-- 	theSmokeVolume:translate( self.worldOffset )
	-- end


	local scale = 0.02
	local cursorMat = spark:createMaterial( "constantColorShader" )
	cursorMat:setVec4( "u_color", vec4(1,0.2,0.2,1.0) );

	-- Tmp -- 3D mouse cursor
	self.markerBox = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
		scale, cursorMat, "OpaquePass" )
	
	local cursorMat2 = spark:createMaterial( "constantColorShader" )
	cursorMat2:setVec4( "u_color", vec4( 0.2,1,0.2,1.0) );
	self.markerBox2 = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
		scale, cursorMat2, "OpaquePass" )


	-- Here's a nice little marker for the origin
	local bool useZeroMarker = false
	if( useZeroMarker ) then
		local debugMat = spark:createMaterial( "colorShader" )
		debugMat.name = "DebugMaterial"
		debugMat:setVec4( "u_color", vec4( 1, 1, 1, 1 ) )
		local scale = 0.01
		local box = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), scale, debugMat, "OpaquePass" )
		--box:translate( self.worldOffset )	
	end

	local mainFontSize = 36
	local smallFontSize = 12
	local fontMgr = spark:getFontManager()
	local fontFilename = "Vera.ttf"--"HelveticaNeueLight.ttf" );
	fontMgr:addFont( "Sans", mainFontSize, fontFilename )
	fontMgr:addFont( "Sans", smallFontSize, fontFilename )

	local fontDesc = {}
	fontDesc.name = "Sans"
	fontDesc.size = mainFontSize

	fontDesc.material = spark:createMaterial( "TextShader" )
	fontDesc.material:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.material:setVec4( "u_color", vec4( 0.7, 0.7, 0.7, 1 ) )

	fontDesc.rolloverMaterial = spark:createMaterial( "TextShader" )
	fontDesc.rolloverMaterial:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.rolloverMaterial:setVec4( "u_color", vec4( 1.0, 0.7, 0.7, 1 ) )

	fontMgr:addFont( fontDesc.name, fontDesc.size, fontFilename );


	self.buttons["wattage"] = Button:new( 0.05, 0.95, 
		string.format("%2.0f", self.currWattage), fontDesc )
	self.buttons["wattage"].onClick = incrWattage
	self.buttons["wattage"].onClick2 = decrWattage

	wattUnitMsg = spark:createText( fontDesc.name, 
		                            12, 
		                            fontDesc.material, 
		                            "HUDPass", "Watts" )
	wattUnitMsg:translate( 0.0875, 0.945, 0 )

	self.buttons["waveText"] = Button:new( 0.25, 0.95, "[Cut]", fontDesc )
	self.buttons["activation"] = Button:new( 0.5, 0.95, 
		string.format("%2.1f", self.activationTime), fontDesc )
	timeUnitMsg = spark:createText( fontDesc.name, 
		                            12, 
		                            fontDesc.material, 
		                            "HUDPass", "Seconds" )
	timeUnitMsg:translate( 0.54, 0.945, 0 )


	self.buttons["task"] = Button:new( 0.9, 0.95, "Dessicate", fontDesc )


	-- --Highlight GUI w/ quad
	local bgAccentMat = spark:createMaterial( "constantColorShader" )
	bgAccentMat:setVec4( "u_color", vec4(0,0,0,0.15) )
	local bgQuad = spark:createQuad( vec2(0,0.9), vec2(1.0,1.0),
		bgAccentMat, "HUDPass" )
	--bgQuad:translate( 0,0,-1 )
	----

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

	if( input:isButtonPressed("mouse", 0) ) then
		self.tissue:setMaterialForPassName( "OpaquePass", self.tissueMat_debug )
		print( "USING DEBUG TEXTURE" )
		for name,button in pairs(self.buttons) do
			if( button:isOver( input:getPosition("mouse") ) ) then
				button.onClick(self)
			end
		end
	elseif( input:isButtonPressed("mouse", 1) ) then
		self.tissue:setMaterialForPassName( "OpaquePass", self.tissueMat )
		print( "USING NORMAL TEXTURE" )
		for name,button in pairs(self.buttons) do
			if( button:isOver( input:getPosition("mouse") ) ) then
				button.onClick2(self)
			end
		end
	else
		for name,button in pairs(self.buttons) do
			if( button:isOver( input:getPosition("mouse") ) ) then
				--print( "Mouse over: "..name )
				button:onMouseOver()
			else
				--print( "Mouse out : "..name )
				button:onMouseOut()
			end
		end
	end

	-- local mousePos = input:getPosition( "mouse" ) 
	-- --local xform = input:getTransform( "mouse" )
	-- --self.markerBox:setTransform( xform )
	-- self.markerBox:setTransform( mat4() )
	-- self.markerBox:translate( 0.0002 * (mousePos.x - 1920.0/2.0), -- hard coded half-width
	-- 	                           -0.02,
	-- 	                           0.0002 * (mousePos.y - 1054/2.0) ) -- hard coded half-height
	-- self.markerBox:scale( 0.0025 )


	-- Debugging (and on non-zspace machines, use mouse)
	local stylusPos = input:getPosition( "stylus" )
	local stylusMat = input:getTransform( "stylus" )
	--local stylusPos = input:getPosition( "mouse" )

	local screenSpaceOffset = vec3( 0, 0.25, 0 )
	-- green block on final pos & orient
	self.markerBox2:setTransform( mat4() )
	self.markerBox2:translate( screenSpaceOffset )
	self.markerBox2:applyTransform( stylusMat )

	-- red block on base position
	self.markerBox:setTransform( mat4() )
	self.markerBox:translate( stylusPos )
	
	local floorHeight = self.worldOffset.y - screenSpaceOffset.y
	local passDepth = 0.0010
	local useOnlyPosition = false -- for debugging
	local limitDepth = true
	local isBelowSurface = stylusPos.y < (floorHeight - passDepth)
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


	local touchThreshold = 0.005
	local sparkThreshold = 0.005
	if( input:isButtonPressed( "stylus", 0 ) ) then
		local toolTipPos = stylusPos.y
		print( "Activation at " .. self.currWattage .. " watts at dist " .. abs( toolTipPos - floorHeight ) )
		
		if( abs( toolTipPos - floorHeight ) < touchThreshold ) then
			theTissueSim:accumulateHeat( stylusPos.x - self.worldOffset.x, stylusPos.z - self.worldOffset.z, self.currWattage ) 
		end

		self.activationTime = self.activationTime + dt
		txt = string.format("%2.1f", self.activationTime)
		self.buttons["activation"].text:setText( txt )


		--TODO play sound

	end
end

function SimulationState:fixedUpdate( dt )
	--print( "SimulationState:fixedUpdate" )	
end

function SimulationState:deactivate()
	print( "SimulationState:deactivate" )
end

function SimulationState:nextState( currTime )
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
end

theState = SimulationState:new()
theNextState = ""

