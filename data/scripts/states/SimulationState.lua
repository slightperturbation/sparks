

SimulationState = {}

function SimulationState:new()
	print( "SimulationState:new" )
	newObj = { angle = 45, hasRunOnce = false, startTime = -1 }
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

	HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", mainRenderTarget )
	HUDRenderPass:setDepthTest( false )
	HUDRenderPass:setDepthWrite( false )
	HUDRenderPass:useInterpolatedBlending()


	-- Make skin box
	local tissueMat = spark:createMaterial( "tissueShader" )
	tissueMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
	tissueMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
	tissueMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
	tissueMat:setVec4( "u_ka", vec4(1,1,1,1) )
	tissueMat:setVec4( "u_kd", vec4(1,1,1,1) )
	tissueMat:setVec4( "u_ks", vec4(1,1,1,1) )
	tissueMat:setFloat( "u_ns", 100.0 )
	tissueMat:addTexture( "s_color", "skinColor" )
	local skin = spark:createCube( vec3(-2.5, .25, -2.5), vec3(5, 0.5, 5), tissueMat, "OpaquePass" )
	--skin:rotate( 90, vec3(1,0,0) )


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
end

function SimulationState:fixedUpdate( dt )
	print( "SimulationState:fixedUpdate" )	
	self.boxB:rotate( -5, vec3(0,1,0) )
end

function SimulationState:deactivate()
	print( "SimulationState:deactivate" )
end

function SimulationState:nextState( currTime )
	-- For now, theNextState global is used to pass
	-- the next desired state back to the app
	-- TODO should be changed to use the return value
	print( "SimulationState:nextState( " .. currTime .. " )")
	if self.startTime == -1 then
		self.startTime = currTime
	end

	if (currTime - self.startTime) > 10 then 
		theNextState = "Loading" 
		print( "Changing state to menu!" )
	else
		theNextState = ""
	end
end

theState = SimulationState:new()
theNextState = ""

