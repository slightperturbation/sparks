-----------------------------
--[[
	Setup standard rendering environment

--]]

local Render = {}

function Render:createDefaultRenderPasses( isShadowOn )

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
	
	transRenderPass = spark:createRenderPass( 0.9, "TransparentPass", mainRenderTarget )
	transRenderPass:setDepthWrite( false )
	transRenderPass:setDepthTest( true )
	transRenderPass:useInterpolatedBlending()

	HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", mainRenderTarget )
	HUDRenderPass:setDepthTest( false )
	HUDRenderPass:setDepthWrite( false )
	HUDRenderPass:useInterpolatedBlending()

	HUDUnderRenderPass = spark:createOverlayRenderPass( 0.26, "HUDUnderPass", mainRenderTarget )
	HUDUnderRenderPass:setDepthTest( false )
	HUDUnderRenderPass:setDepthWrite( false )
	HUDUnderRenderPass:useInterpolatedBlending()

	wireRenderPass = spark:createRenderPass( 0.95, "WirePass", mainRenderTarget )
	wireRenderPass:setDepthWrite( true )
	wireRenderPass:setDepthTest( true )
	wireRenderPass:disableBlending()
	wireRenderPass:setWireframe( true )

	local isShadowDebugDisplayOn = false
	if( isShadowOn ) then
		--Shader uniform must be set:  s_shadowMap = "light0_shadowMap"
		shadowTarget = spark:createDepthMapRenderTarget( "light0_shadowMap", 2048, 2048 )
		
		-- Either ortho (=directional light) or perspective (=local/spot light) can be used
		--shadowCamera = spark:createOrthogonalProjection( -0.5, 0.5, -0.5, 0.5, 0, 5, vec3( -0.5, 2, 1 ) )
		--shadowCamera = spark:createPerspectiveProjection( vec3( -1, 1.345, 1.222 ), vec3(0,0,0), vec3(0,1,0), 10.0, 0.5, 5.0 )
		shadowCamera = spark:createPerspectiveProjection( vec3( 0.25, 0.5, 0.05 ), -- source
			                                              vec3( 0, 0, 0 ),        -- target
			                                              vec3( 0, 1, 0 ),        -- up
			                                              28.0, -- FOV
			                                              0.5,  -- near plane
			                                              5 )--3 )   -- far plane

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
		if isShadowDebugDisplayOn then
			self.shadowMapMaterial = spark:createMaterial( "texturedOverlayShader" )
			self.shadowMapMaterial:addTexture( "s_color", "light0_shadowMap" )
			self.shadowQuad = spark:createQuad( vec2( 0.01, 0.69 ), 
				                          vec2( 0.3*(9/16), 0.3 ), -- 9/16 is aspect ratio of display 
				                          self.shadowMapMaterial, "HUDPass" )
		end
	end
end


-------------------------------------------------------
return Render
-------------------------------------------------------
