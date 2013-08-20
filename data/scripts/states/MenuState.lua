

MenuState = {}

function MenuState:new()
	print( "MenuState:new" )
	newObj = { angle = 45, hasRunOnce = false, startTime = -1 }
	self.__index = self
	return setmetatable(newObj, self)
end

function MenuState:load()
	print( "MenuState:load" )

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

	boxMat = spark:createMaterial( "colorShader" )
	boxMat:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )
	boxA = spark:createCube( vec3(0,0,0), vec3(0.25,0.25,0.25), boxMat, "OpaquePass" )
	self.boxB = spark:createCube( vec3(-0.5,0,0), vec3(0.25,0.25,0.25), boxMat, "OpaquePass" )
	self.boxB:rotate( self.angle, vec3(0,1,0) )

end

function MenuState:activate()
	print( "MenuState:activate" )
	self.startTime = -1

	camera = spark:getCamera()
	camera:cameraPos( 0.2, 1.2, -0.9 )
	camera:cameraTarget( 0.06, 0.1, 0.0 )
	camera:fov( 48 )
end

function MenuState:update( dt )
	print( "MenuState:update" )
end

function MenuState:fixedUpdate( dt )
	print( "MenuState:fixedUpdate" )	
	self.boxB:rotate( -5, vec3(0,1,0) )
end

function MenuState:deactivate()
	print( "MenuState:deactivate" )
end

function MenuState:nextState( currTime )
	-- For now, theNextState global is used to pass
	-- the next desired state back to the app
	-- TODO should be changed to use the return value
	print( "MenuState:nextState( " .. currTime .. " )")
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

theState = MenuState:new()
theNextState = ""

