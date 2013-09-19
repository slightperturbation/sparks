--ShadowExampleState.lua

local Button = require "button"
-----------------------------

function abs( a ) 
	if( a < 0 ) then return -a end
	return a
end


ShadowExampleState = {}

function ShadowExampleState:new()
	print( "ShadowExampleState:new" )
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


function ShadowExampleState:load()
	print( "ShadowExampleState:load" )

	spark:createPostProcessingRenderPass( 0.0, 
	      "MSAAFinalRenderPass",
	      "MainRenderTargetTexture", spark:getFrameBufferRenderTarget(), 
	      "texturedOverlayShader" )
 
	mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
	spark:setMainRenderTarget( mainRenderTarget )
	mainRenderTarget:setClearColor( vec4( 0.1, 0.05, 0.05, 1.0 ) )
 
 	-- Pass ordering (or depth testing?) doesn't work correctly 
 	-- when the mainRenderTarget is the frameBuffer rather than a texture
	-- mainRenderTarget = spark:getFrameBufferRenderTarget()
	-- spark:setMainRenderTarget( mainRenderTarget ) -- facade which target to use
	-- mainRenderTarget:setClearColor( vec4( 0.1, 0.05, 0.05, 1.0 ) )

	opaqueRenderPass = spark:createRenderPass( 1.0, "OpaquePass", mainRenderTarget )
	opaqueRenderPass:setDepthWrite( true )
	opaqueRenderPass:setDepthTest( true )
	opaqueRenderPass:disableBlending()

	HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", mainRenderTarget )
	HUDRenderPass:setDepthTest( false )
	HUDRenderPass:setDepthWrite( false )
	HUDRenderPass:useInterpolatedBlending()


	isShadowOn = true
	if( isShadowOn ) then

		--Shader uniform must be set:  s_shadowMap = "light0_shadowMap"
		shadowTarget = spark:createDepthMapRenderTarget( "light0_shadowMap", 1024, 1024 )
		
		-- Either ortho (=directional light) or perspective (=local/spot light) can be used
		--shadowCamera = spark:createOrthogonalProjection( -1, 1, -1, 1, 0, 5, vec3( 0.5, 2, 2 ) )
		shadowCamera = spark:createPerspectiveProjection( vec3( 1, 1.345, 1.222 ), vec3(0,0,0), vec3(0,1,0), 30.0, 0.5, 5.0 ) -- good

		-- The RenderPass's projection is used to compute the shadowmap and
		-- fill shadowTarget with depth info.
		shadowPass = spark:createRenderPassWithProjection( 1.1, "ShadowPass", shadowCamera, shadowTarget )
		shadowPass:addShadowLight( vec4(1,1,1,1), shadowCamera )
		-- And the same projection is used on the OpaquePass to render the shadow
		-- by calculating the depth of pixels relative to the shadow-casting light
		opaqueRenderPass:addShadowLight( vec4(1,1,1,1), shadowCamera )

		shadowMaterial = spark:createMaterial( "shadowCasterShader" )
		-- shadowMaterial:setFloat( "u_shadowBias", 0.01 )

		-- Debug display of shadow map in upper right
		self.shadowMapMaterial = spark:createMaterial( "texturedOverlayShader" )
		self.shadowMapMaterial:addTexture( "s_color", "light0_shadowMap" )
		self.shadowQuad = spark:createQuad( vec2( 0.01, 0.69 ), 
			                          vec2( 0.3*(9/16), 0.3 ), -- 9/16 is aspect ratio of display 
			                          self.shadowMapMaterial, "HUDPass" )
	end
	
	local scale = 0.2
	local cursorMat = spark:createMaterial( "simpleShadowExampleShader" )
	cursorMat:setVec4( "u_color", vec4( 0.7, 0.2, 0.2, 1.0 ) );
	cursorMat:addTexture( "s_shadowMap", "light0_shadowMap" )

	-- Tmp -- 3D mouse cursor
	self.markerBox = spark:createCube( vec3( 0 + scale*2, 0, -scale/2.0 ), 
		scale, cursorMat, "OpaquePass" )
	if( isShadowOn ) then
		self.markerBox:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end
	
	local cursorMat2 = spark:createMaterial( "simpleShadowExampleShader" )
	cursorMat2:setVec4( "u_color", vec4( 0.2, 0.7, 0.2, 1.0 ) );
	cursorMat2:addTexture( "s_shadowMap", "light0_shadowMap" )
	self.markerBox2 = spark:createCube( vec3( -scale/2.0, 0, -scale/2.0 ), 
		scale, cursorMat2, "OpaquePass" )
	if( isShadowOn ) then
		self.markerBox2:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end

	scale = 2.0
	local greyMat = spark:createMaterial( "simpleShadowExampleShader" )
	greyMat:setVec4( "u_color", vec4( 0.5, 0.5, 0.5, 1.0 ) );
	greyMat:addTexture( "s_shadowMap", "light0_shadowMap" )
	self.floor = spark:createCube( vec3( -scale/2.0, -0.2, -scale/2.0 ), scale, greyMat, "OpaquePass" )
	self.floor:scale( vec3(1,0.1,1) )
	--self.floor:translate( vec3(0,scale,0) )
	if( isShadowOn ) then
		self.floor:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end


end

function ShadowExampleState:activate()
	print( "ShadowExampleState:activate" )
	self.startTime = -1

	local camera = spark:getCamera()
	camera:cameraPos( 0.0, 1.345, 1.222 )
	camera:cameraTarget( 0.0, 0.0, 0.0 )
	camera:cameraUp( 0,1,0 )
end


function ShadowExampleState:update( dt )
end

function ShadowExampleState:deactivate()
	print( "ShadowExampleState:deactivate" )
end

function ShadowExampleState:nextState( currTime )

end

theState = ShadowExampleState:new()
theNextState = ""

