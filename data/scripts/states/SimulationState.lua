

local Button = require "button"
-----------------------------


SimulationState = {}

function SimulationState:new()
	print( "SimulationState:new" )
	newObj = 
	{ 
		buttons = {}, 
		angle = 45, 
		hasRunOnce = false, 
		startTime = -1, 
		currWattage = 20,
		startActivation = 4, 
		stopActivation = 5.22, 
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
		simState.buttons["wattage"].text:setText( string.format("%2.0f watts", simState.currWattage) )
	end
end
function decrWattage( simState )
	print( "Change wattage up" )
	if( simState.currWattage > 15 ) then
		simState.currWattage = simState.currWattage - 10
		print( "New Wattage: "..simState.currWattage )
		simState.buttons["wattage"].text:setText( string.format("%2.0f watts", simState.currWattage) )
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
	mainRenderTarget:setClearColor( vec4( 0.2,0.2,0.2,1.0 ) )

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

	local mainFontSize = 24
	local smallFontSize = 12
	local fontMgr = spark:getFontManager()
	fontMgr:addFont( "Sans", mainFontSize, "HelveticaNeueLight.ttf" );
	fontMgr:addFont( "Sans", smallFontSize, "HelveticaNeueLight.ttf" );

	local fontDesc = {}
	fontDesc.name = "Sans"
	fontDesc.size = mainFontSize

	fontDesc.material = spark:createMaterial( "TextShader" )
	fontDesc.material:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.material:setVec4( "u_color", vec4( 0.7, 0.7, 0.7, 1 ) )

	fontDesc.rolloverMaterial = spark:createMaterial( "TextShader" )
	fontDesc.rolloverMaterial:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.rolloverMaterial:setVec4( "u_color", vec4( 1.0, 0.7, 0.7, 1 ) )

	local fontMgr = spark:getFontManager()
	fontMgr:addFont( fontDesc.name, fontDesc.size, "HelveticaNeueLight.ttf" );


	self.buttons["wattage"] = Button:new( 0.02, 0.85, 
		string.format("%2.0f watts", self.currWattage), fontDesc )
	self.buttons["wattage"].onClick = incrWattage
	self.buttons["wattage"].onClick2 = decrWattage

	self.buttons["waveText"] = Button:new( 0.02, 0.75, "Cut", fontDesc )
	self.buttons["activation"] = Button:new( 0.02, 0.5, "0.0 secs", fontDesc )
	self.buttons["task"] = Button:new( 0.02, 0.25, "Dessicate", fontDesc )


	--Highlight GUI side w/ quad
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

	if( input:isButtonPressed("mouse", 0) ) then
		for name,button in pairs(self.buttons) do
			if( button:isOver( input:getPosition("mouse") ) ) then
				button.onClick(self)
			end
		end
	elseif( input:isButtonPressed("mouse", 1) ) then
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

	--print( "SimulationState:update" )
	-- self.currTime = self.currTime + dt
	-- if self.startTime == -1 then
	-- 	self.startTime = self.currTime
	-- else
	-- 	actTime = (self.currTime - self.startTime) - self.startActivation
	-- 	if self.startActivation < (self.currTime - self.startTime) and self.stopActivation >  (self.currTime - self.startTime) then
	-- 		self.activationText:setText( string.format("%1.1f sec", actTime) )
	-- 		self.tissueMat:setFloat( "u_activationTime", actTime )
	-- 	else
	-- 		self.tissueMat:setFloat( "u_activationTime", 0 )
	-- 	end
	-- end

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

