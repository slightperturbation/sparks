----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
----------------------------------------

--[[
	Prototypical Simulation State 

	Create simulation states based on this model.

	New simulation states must also be loaded in main()
	by adding the base name (without "State") to the simStates vector
--]]

ShadowExample = {}

function ShadowExample:new()
	print( "ShadowExample:new" )
	newObj = 
	{ 
		buttons = {}, 
 		startTime = -1, 
		currTime = 0, 
		theNextState = ""
	}
	self.__index = self
	return setmetatable(newObj, self)
end


function ShadowExample:load()
	print( "ShadowExample:load" )

	local isShadowOn = true

	Render:createDefaultRenderPasses( isShadowOn )


	local scale = 0.4
	local cursorMat = spark:createMaterial( "simpleShadowExampleShader" )
	cursorMat:setVec4( "u_color", vec4( 0.7, 0.2, 0.2, 1.0 ) );
	cursorMat:addTexture( "s_shadowMap", "light0_shadowMap" )

	-- Tmp -- 3D mouse cursor
	self.markerBox = spark:createCube( vec3( 0 + scale*2, 0, -scale/2.0 ), 
		scale, cursorMat, "OpaquePass" )
	if( isShadowOn ) then
		self.markerBox:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end
	
	local scale = 0.2
	local cursorMat2 = spark:createMaterial( "phongShader" )
	cursorMat2:addTexture( "s_color", "cloth" )
	cursorMat2:addTexture( "s_normal", "noise_normalMap" )
	cursorMat2:setFloat( "u_normalMapStrength", 0.5 )
	cursorMat2:setVec4( "u_light.position_camera", vec4(shadowSource.x, shadowSource.y, shadowSource.z ,1) ) --vec4(2,2,-1,1) )--
	cursorMat2:setVec4( "u_light.diffuse", vec4(0.3,0.3,0.3,1) )
	cursorMat2:setVec4( "u_ambientLight", vec4(0.2,0.2,0.2,1) )
	cursorMat2:setVec4( "u_ks", vec4(1,1,1,1) )
	cursorMat2:setFloat( "u_ns", 45 )
	--cursorMat2:setBool( "u_textureSwapUV", true )
	--cursorMat2:setVec2( "u_textureRepeat", vec2(0.5,1) )

	-- local cursorMat2 = spark:createMaterial( "simpleShadowExampleShader" )
	-- cursorMat2:setVec4( "u_color", vec4( 0.2, 0.7, 0.2, 1.0 ) );
	cursorMat2:addTexture( "s_shadowMap", "light0_shadowMap" )
	self.markerBox2 = spark:createCube( vec3( -scale/2.0, 0, -scale/2.0 ), 
		scale, cursorMat2, "OpaquePass" )
	if( isShadowOn ) then
		self.markerBox2:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end

	scale = 2.0
	local greyMat = spark:createMaterial( "simpleShadowExampleShader" )
	greyMat:setVec4( "u_color", vec4( 0.25, 0.25, 0.25, 1.0 ) );
	greyMat:addTexture( "s_shadowMap", "light0_shadowMap" )
	self.floor = spark:createCube( vec3( -scale/2.0, -0.2, -scale/2.0 ), scale, greyMat, "OpaquePass" )
	self.floor:scale( vec3(1,0.1,1) )
	--self.floor:translate( vec3(0,scale,0) )
	if( isShadowOn ) then
		self.floor:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end

	-- -- Debug display of shadow map in upper right
	-- self.shadowMapMaterial = spark:createMaterial( "texturedOverlayShader" )
	-- self.shadowMapMaterial:addTexture( "s_color", "light0_shadowMap" )
	-- self.shadowQuad = spark:createQuad( vec2( 0.01, 0.69 ), 
	-- 	                          vec2( 0.3*(9/16), 0.3 ), -- 9/16 is aspect ratio of display 
	-- 	                          self.shadowMapMaterial, "HUDPass" )


	
	-- local scale = 0.2
	-- local cursorMat = spark:createMaterial( "simpleShadowExampleShader" )
	-- cursorMat:setVec4( "u_color", vec4( 0.7, 0.2, 0.2, 1.0 ) );
	-- cursorMat:addTexture( "s_shadowMap", "light0_shadowMap" )

	-- -- Tmp -- 3D mouse cursor
	-- self.markerBox = spark:createCube( vec3( 0 + scale*2, 0, -scale/2.0 ), 
	-- 	scale, cursorMat, "OpaquePass" )
	-- if( isShadowOn ) then
	-- 	self.markerBox:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- end
	
	-- local cursorMat2 = spark:createMaterial( "simpleShadowExampleShader" )
	-- cursorMat2:setVec4( "u_color", vec4( 0.2, 0.7, 0.2, 1.0 ) );
	-- cursorMat2:addTexture( "s_shadowMap", "light0_shadowMap" )
	-- self.markerBox2 = spark:createCube( vec3( -scale/2.0, 0, -scale/2.0 ), 
	-- 	scale, cursorMat2, "OpaquePass" )
	-- if( isShadowOn ) then
	-- 	self.markerBox2:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- end

	-- scale = 2.0
	-- local greyMat = spark:createMaterial( "simpleShadowExampleShader" )
	-- greyMat:setVec4( "u_color", vec4( 0.5, 0.5, 0.5, 1.0 ) );
	-- greyMat:addTexture( "s_shadowMap", "light0_shadowMap" )
	-- self.floor = spark:createCube( vec3( -scale/2.0, -0.2, -scale/2.0 ), scale, greyMat, "OpaquePass" )
	-- self.floor:scale( vec3(1,0.1,1) )
	-- --self.floor:translate( vec3(0,scale,0) )
	-- if( isShadowOn ) then
	-- 	self.floor:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- end














	---------------------------------------------------------
	-- Debugging Markers

	-- local scale = 0.0025
	-- local redMat = spark:createMaterial( "constantColorShader" )
	-- redMat:setVec4( "u_color", vec4( 1, 0.2, 0.2, 1.0) );
	-- local greenMat = spark:createMaterial( "constantColorShader" )
	-- greenMat:setVec4( "u_color", vec4( 0.2, 1, 0.2, 1.0) );

	-- -- Tmp -- 3D mouse cursor
	-- self.useMouseCursorCube = true
	-- if( useMouseCursorCube ) then
	-- 	self.markerBox = spark:createCube( vec3( -scale/1.5, -scale/2.0, -scale/2.0 ), 
	-- 		scale, redMat, "OpaquePass" )
	-- 	if( isShadowOn ) then
	-- 		self.markerBox:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- 	end
		
	-- 	self.markerBox2 = spark:createCube( vec3( -scale/2.0, -scale/1.5, -scale/2.0 ), 
	-- 		scale, greenMat, "OpaquePass" )
	-- 	if( isShadowOn ) then
	-- 		self.markerBox2:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- 	end
	-- end

	-- Here's a nice little marker for the origin
	-- local useZeroMarker = true
	-- if( useZeroMarker ) then
	-- 	local scale = 0.01

	-- 	local rMat = spark:createMaterial( "constantColorShader" )
	-- 	rMat:setVec4( "u_color", vec4( 1, 0.1, 0.1, 0.5 ) )
	-- 	local boxX = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
	-- 		                          scale, rMat, "WirePass" )
	-- 	boxX:scale( vec3(1, 0.1, 0.1) )

	-- 	local gMat = spark:createMaterial( "constantColorShader" )
	-- 	gMat:setVec4( "u_color", vec4( 0.1, 1, 0.1, 0.5 ) )
	-- 	local boxY = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
	-- 		                          scale, gMat, "WirePass" )
	-- 	boxY:scale( vec3(0.1, 1, 0.1) )

	-- 	local bMat = spark:createMaterial( "constantColorShader" )
	-- 	bMat:setVec4( "u_color", vec4( 0.1, 0.1, 1, 0.5 ) )
	-- 	local boxZ = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
	-- 		                          scale, bMat, "WirePass" )
	-- 	boxZ:scale( vec3(0.1, 0.1, 1) )
	-- 	--box:translate( self.worldOffset )	
	-- 	if( isShadowOn ) then
	-- 		boxX:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- 		boxY:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- 		boxZ:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- 	end
	-- end

	-- local use10CMMarkers = true
	-- if use10CMMarkers then
	--     self.boxMat = spark:createMaterial( "colorShader" )
	--     self.boxMat:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )

	--     --self.boxA = spark:createCube( vec3(-0.0105, -0.15, -0.1172), 0.01, self.boxMat, "OpaquePass" )
	--     self.boxA = spark:createCube( vec3(-0.05, 0, 0), 0.01, self.boxMat, "OpaquePass" )
	-- 	if( isShadowOn ) then
	-- 		self.boxA:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- 	end
	-- 	self.boxB = spark:createCube( vec3( 0.05, 0, 0), 0.01, self.boxMat, "OpaquePass" )
	-- 	if( isShadowOn ) then
	-- 		self.boxB:setMaterialForPassName( "ShadowPass", shadowMaterial )
	-- 	end
	-- end


end

function ShadowExample:activate()
	print( "ShadowExample:activate" )
	self.startTime = -1

	local camera = spark:getCamera()
	local inputDeviceName = input:getDefaultDeviceName()

	-- Default view
	camera:cameraPos( 0.0, 0.345, 0.222 )
	--camera:cameraPos( 0.0, 0.125, 0.18 ) -- close-up, good for screen shots
	camera:cameraTarget( 0, -0.02, 0 )
	camera:cameraUp( 0,1,0 )

	self.theNextState = ""
end


function ShadowExample:update( dt )

	-- Get control inputs
	inputDeviceName = input:getDefaultDeviceName()
	if inputDeviceName == "NO_DEFAULT_DEVICE_SPECIFIED" then
		-- ERROR
		print( "Error: "..inputDeviceName )
	end

	-- -- green cube on final pos & orient
	-- if self.useMouseCursorCube then 
	-- 	-- green as the full, unmodified transform
	-- 	self.markerBox2:setTransform( stylusMat )
	-- 	--markerBox2:applyTransform( stylusMat )
	-- 	-- red block on base position
	-- 	self.markerBox:setTransform( mat4() )
	-- 	self.markerBox:translate( self.stylusPos )
	-- end


	-- if( input:isButtonPressed( "mouse", 0 ) ) then
	-- 	isActivated = true
	-- end

end

function ShadowExample:deactivate()
	print( "ShadowExample:deactivate" )
end

function ShadowExample:nextState( currTime )
	-- if input:isKeyDown( KEY_KP_ENTER ) then
	-- 	self.theNextState = "Menu"
	-- end
 	-- 	if input:isButtonPressed( "stylus", 2 ) then
	-- 	self.theNextState = "Menu"
	-- end
	theNextState = self.theNextState
end

theState = ShadowExample:new()
theNextState = ""

