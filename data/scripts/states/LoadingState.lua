--[[

Loading state loads the resources that take substantial time,
e.g., fonts, big meshes, etc.

]]
 
LoadingState = {}

function LoadingState:new()
	print( "LoadingState:new" )
	local newObj = { 
		angle = 45, 
		hasRunOnce = false
	}
	self.__index = self
	return setmetatable(newObj, self)
end

function LoadingState:load()
	print( "LoadingState:load" )
	spark:createPostProcessingRenderPass( 0.0, 
	      "MSAAFinalRenderPass",
	      "MainRenderTargetTexture", spark:getFrameBufferRenderTarget(), 
	      "texturedOverlayShader" )

	self.mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
	spark:setMainRenderTarget( self.mainRenderTarget )
	self.mainRenderTarget:setClearColor( vec4( 0,1,0,0.5 ) )

	self.opaqueRenderPass = spark:createRenderPass( 1.0, "OpaquePass", self.mainRenderTarget )
	self.opaqueRenderPass:setDepthWrite( true )
	self.opaqueRenderPass:setDepthTest( true )
	self.opaqueRenderPass:disableBlending()

	self.HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", self.mainRenderTarget )
	self.HUDRenderPass:setDepthTest( false )
	self.HUDRenderPass:setDepthWrite( false )
	self.HUDRenderPass:useInterpolatedBlending()

	textureManager:loadTextureFromImageFile( "cemsimLogo", "cemsimlogo-regular.jpg" )
	self.logoMaterial = spark:createMaterial( "texturedOverlayShader" )
	self.logoMaterial:addTexture( "s_color", "cemsimLogo" )
	local logoX = 2592.0
	local logoY = 864.0
	local aspect = (logoY / logoX) / .75 -- .75 is aspect ratio of display
	self.size = 1
	local size = self.size
	self.logo = spark:createQuad( vec2(0, 1.0-size*aspect), 
		                          vec2( size, -size*aspect), 
		                          self.logoMaterial, "HUDPass" )

	self.boxMat = spark:createMaterial( "colorShader" )
	self.boxMat:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )
	self.boxA = spark:createCube( vec3(0,0,0), vec3(0.25,0.25,0.25), self.boxMat, "OpaquePass" )
	self.boxB = spark:createCube( vec3(-0.5,0,0), vec3(0.25,0.25,0.25), self.boxMat, "OpaquePass" )
	self.boxB:rotate( self.angle, vec3(0,1,0) )
end

function LoadingState:activate()
	print( "LoadingState:activate" )

	local camera = spark:getCamera()
	camera:cameraPos( 0.2, 1.2, -0.9 )
	camera:cameraTarget( 0.06, 0.1, 0.0 )
	camera:fov( 48 )
end

function LoadingState:update( dt )
	print( "LoadingState:update" )

	local xform = self.logo:getTransform()

	if not self.hasRunOnce then
		print( "\tLoading cemsim logo" )
		self.hasRunOnce = true
	end
end

function LoadingState:fixedUpdate( dt )
	print( "LoadingState:fixedUpdate" )	
	self.boxB:rotate( 10, vec3(0,1,0) )
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
		theNextState = "Menu" 
		print( "Changing state to menu!" )
	else
		theNextState = ""
	end
end

theState = LoadingState:new()
theNextState = ""

