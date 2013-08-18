

LoadingState = {}

function LoadingState:new()
	print( "LoadingState:new" )
	newObj = { angle = 45, theNextState = "" }
	self.__index = self
	return setmetatable(newObj, self)
end

function LoadingState:activate()
	print( "LoadingState:activate" )

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

	camera = spark:getCamera()
	camera:cameraPos( 0.2, 1.2, -0.9 )
	camera:cameraTarget( 0.06, 0.1, 0.0 )
	camera:fov( 48 )
end

function LoadingState:update( dt )
	print( "LoadingState:update" )
end

function LoadingState:fixedUpdate( dt )
	print( "LoadingState:fixedUpdate" )	
	self.boxB:rotate( 1, vec3(0,1,0) )
end

function LoadingState:deactivate()
	print( "LoadingState:deactivate" )
end

function LoadingState:nextState( currTime )
	-- For now, theNextState global is used to pass
	-- the next desired state back to the app
	-- TODO should be changed to use the return value
	print( "LoadingState:nextState( " .. currTime .. " )")
	if currTime > 10 then 
		theNextState = "menu" 
		print "Changing state to menu!"
	else
		theNextState = ""
	end
end

theState = LoadingState:new()
theNextState = ""


s