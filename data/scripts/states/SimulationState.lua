

SimulationState = {}

function SimulationState:new()
	print( "SimulationState:new" )
	newObj = { angle = 45, hasRunOnce = false, startTime = -1, startActivation = 4, stopActivation = 5.22, currTime = 0 }
	self.__index = self
	return setmetatable(newObj, self)
end

function SimulationState:load()
	print( "SimulationState:load" )

	spark:createPostProcessingRenderPass( 0.0, 
	      "MSAAFinalRenderPass",
	      "MainRenderTargetTexture", spark:getFrameBufferRenderTarget(), 
	      "texturedOverlayShader" )
 
	mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
	spark:setMainRenderTarget( mainRenderTarget )
	mainRenderTarget:setClearColor( vec4( 0,1,0,0.5 ) )

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


	-- Make skin box
	self.tissueMat = spark:createMaterial( "tissueShader" )
	self.tissueMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
	self.tissueMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
	self.tissueMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
	self.tissueMat:setVec4( "u_ka", vec4(1,1,1,1) )
	self.tissueMat:setVec4( "u_kd", vec4(1,1,1,1) )
	self.tissueMat:setVec4( "u_ks", vec4(1,1,1,1) )
	self.tissueMat:setFloat( "u_ns", 100.0 )
	self.tissueMat:setFloat( "u_activationTime", 0.0 )
	self.tissueMat:addTexture( "s_color", "skinColor" )
	--local skin = spark:createCube( vec3(-2.5, .25, -2.5), vec3(5, 0.5, 5), self.tissueMat, "OpaquePass" )
	--skin:rotate( 90, vec3(1,0,0) )
	
	local skin = spark:createCube( vec3(-2.5, .25, -2.5), vec3(5, 0.5, 5), self.tissueMat, "OpaquePass" )
	skin:rotate( 90, vec3(1,0,0) )

	local fontMgr = spark:getFontManager()
	local mainFontSize = 64
	local smallFontSize = 36
	fontMgr:addFont( "Sans", mainFontSize, "HelveticaNeueLight.ttf" );
	fontMgr:addFont( "Sans", smallFontSize, "HelveticaNeueLight.ttf" );

	----
	local readoutTextMat = spark:createMaterial( "TextShader" )
	readoutTextMat:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	readoutTextMat:setVec4( "u_color", vec4(0.8,0.8,0.8,1) )

	local wattageText = spark:createText( "Sans", mainFontSize, 
		readoutTextMat, "HUDPass", "45 watts" )
	wattageText:translate( 0.01, 0.85, 0 )

	local waveText = spark:createText( "Sans", mainFontSize, 
		readoutTextMat, "HUDPass", "Coag" )
	waveText:translate( 0.01, 0.75, 0 )

	self.activationText = spark:createText( "Sans", mainFontSize, 
		readoutTextMat, "HUDPass", "0 sec" )
	self.activationText:translate( 0.01, 0.65, 0 )

	local waveText = spark:createText( "Sans", smallFontSize, 
		readoutTextMat, "HUDPass", "Current task:\nDessicate region" )
	waveText:translate( 0.01, 0.25, 0 )

	--backgrd quad
	local bgAccentMat = spark:createMaterial( "constantColorShader" )
	bgAccentMat:setVec4( "u_color", vec4(1,1,1,0.33) )
	local bgQuad = spark:createQuad( vec2(0,0), vec2(0.225,1.0),
	bgAccentMat, "HUDPass" )
	bgQuad:translate( 0,0,-1 )
	------

	-- Load hook
	local metalMat = spark:createMaterial( "phongShader" )
	metalMat:addTexture( "s_color", "hook_cautery" )
	metalMat:setVec4( "u_light.position_camera", vec4(0,0,0,1) )
	metalMat:setVec4( "u_light.diffuse", vec4(0.3,0.3,0.3,1) )
	metalMat:setVec4( "u_ambientLight", vec4(0.4,0.2,0.2,1) )
	metalMat:setVec4( "u_ka", vec4(0.4,0.4,0.4,1) )
	metalMat:setVec4( "u_kd", vec4(1,1,1,1) )
	metalMat:setVec4( "u_ks", vec4(1,1,1,1) )
	metalMat:setFloat( "u_ns", 1000.0 )

	local hook = spark:loadMesh( "hook_cautery_new.3DS", metalMat, "OpaquePass" )
	hook:translate( 0,.36,0 )
	hook:rotate( 120,  vec3(0,0,1) )
	hook:rotate( 30,  vec3(0,1,0) )
	hook:scale( 0.02 )
end

function SimulationState:activate()
	print( "SimulationState:activate" )
	self.startTime = -1

	local camera = spark:getCamera()
	camera:cameraPos( 0.2, 1.2, -0.9 )
	camera:cameraTarget( 0.06, 0.1, 0.0 )
	camera:fov( 48 )
end

function SimulationState:update( dt )
	print( "SimulationState:update" )
	self.currTime = self.currTime + dt
	if self.startTime == -1 then
		self.startTime = self.currTime
	else
		actTime = (self.currTime - self.startTime) - self.startActivation
		if self.startActivation < (self.currTime - self.startTime) and self.stopActivation >  (self.currTime - self.startTime) then
			self.activationText:setText( string.format("%1.1f sec", actTime) )
			self.tissueMat:setFloat( "u_activationTime", actTime )
		else
			self.tissueMat:setFloat( "u_activationTime", 0 )
		end
	end
end

function SimulationState:fixedUpdate( dt )
	print( "SimulationState:fixedUpdate" )	


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

