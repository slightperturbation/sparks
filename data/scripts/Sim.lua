-----------------------------
--[[
	Tissue Simulation methods

--]]

local Sim = {}

function Sim.computeJoules( dt, wattage, area )
	return wattage / area
end

--[[
	Creates self.wattDisplay, self.modeDisplay and self.activationTimeDisplay
	as text blocks in the owner table.
	Also adds units and a background tint.
--]]
function Sim.createHUDElements( owner )

	local fontMgr = spark:getFontManager()
	local fontDesc = {}
	fontDesc.name = "Sans"
	fontDesc.fontFilename = "HelveticaNeueLight.ttf"
	fontDesc.size = 48
	local smallFontSize = 12

	fontDesc.material = spark:createMaterial( "TextShader" )
	fontDesc.material:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.material:setVec4( "u_color", vec4( 0.8, 0.8, 0.8, 1 ) )

	fontDesc.rolloverMaterial = spark:createMaterial( "TextShader" )
	fontDesc.rolloverMaterial:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.rolloverMaterial:setVec4( "u_color", vec4( 0.8, 0.8, 0.8, 1 ) )

	fontMgr:addFont( fontDesc.name, fontDesc.size, fontDesc.fontFilename )
	fontMgr:addFont( fontDesc.name, smallFontSize, fontDesc.fontFilename )

	-- Wattage
	owner.wattDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             string.format("%2.0f", owner.currWattage) )
	owner.wattDisplay:translate( 0.05, 0.985, 0 )
	wattUnitMsg = spark:createText( fontDesc.name, 
		                            smallFontSize, 
		                            fontDesc.material, 
		                            "HUDPass", 
		                            "Watts" )
	wattUnitMsg:translate( 0.0875, 0.98, 0 )
	-- Mode
	owner.modeDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             "[ESUMODE]" )
	owner.modeDisplay:translate( 0.33, 0.985, 0.1 )
	-- Activation Time
	owner.activationTimeDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             string.format("%2.1f", owner.activationTime) )
	owner.activationTimeDisplay:translate( 0.64, 0.985, 0 )
	timeUnitMsg = spark:createText( fontDesc.name, 
		                            smallFontSize, 
		                            fontDesc.material, 
		                            "HUDPass", 
		                            "Seconds" )
	timeUnitMsg:translate( 0.675, 0.98, 0 )

	-- Contact Area
	owner.contactAreaDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             string.format("%2.1f", owner.contactArea) )
	owner.contactAreaDisplay:translate( 0.91, 0.985, 0 )
	areaUnitMsg = spark:createText( fontDesc.name, 
		                            smallFontSize, 
		                            fontDesc.material, 
		                            "HUDPass", 
		                            "mm^2" )
	areaUnitMsg:translate( 0.95, 0.98, 0 )


	-- --Highlight GUI w/ quad
	local bgAccentMat = spark:createMaterial( "constantColorShader" )
	bgAccentMat:setVec4( "u_color", vec4( 0, 0, 0, 0.8 ) )
	local bgQuad = spark:createQuad( vec2(0,0.915), vec2(1.0,1.0),
		bgAccentMat, "HUDUnderPass" )
	bgQuad:translate( 0,0,0 )
end


function Sim.createTable( owner, worldOffset )
	owner.clothMat = spark:createMaterial( "phongShader" )
	owner.clothMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
	owner.clothMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
	owner.clothMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
	owner.clothMat:setVec4( "u_ka", vec4(1,1,1,1) )
	owner.clothMat:setVec4( "u_kd", vec4(1,1,1,1) )
	owner.clothMat:setVec4( "u_ks", vec4(1,1,1,1) )
	owner.clothMat:setFloat( "u_ns", 100.0 )
	owner.clothMat:setFloat( "u_activationTime", 0.0 )
	owner.clothMat:addTexture( "s_color", "bgCloth" )      -- "cloth" );
	owner.clothMat:setVec2( "u_textureRepeat", vec2(4,4) )
	local table = spark:createCube( worldOffset + vec3(-0.5, -0.025, -0.5), 1, owner.clothMat, "OpaquePass" )
	table:rotate( 90, vec3(1,0,0) )
end

function Sim.createTissue( owner, worldOffset )
	useHeightMap = true
	if( useHeightMap ) then
		owner.tissueMat = spark:createMaterial( "tissueShader_heightMap" )

		-- TODO -- need new data texture!
		owner.tissueMat:addTexture( "s_heightMap", theTissueSim:getTempMapTextureName() )
		owner.tissueMat:setFloat( "u_heightScale", 0.1 )
	else
		owner.tissueMat = spark:createMaterial( "tissueShader" ) --"tissueShader_procedural"  )
	end
	owner.tissueMat:addTexture( "s_shadowMap", "light0_shadowMap" )

	owner.tissueMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
	owner.tissueMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
	owner.tissueMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
	owner.tissueMat:setVec4( "u_ka", vec4(.1,.1,.1,1) )
	owner.tissueMat:setVec4( "u_kd", vec4(.7,.7,.7,1) )
	owner.tissueMat:setVec4( "u_ks", vec4(1,1,1,1) )
	owner.tissueMat:setFloat( "u_ns", 100.0 )
	owner.tissueMat:setFloat( "u_activationTime", 0.0 )
	--owner.tissueMat:setVec2( "u_textureRepeat", vec2(3.3,3.3))

	owner.tissueMat:addTexture( "s_color", "tissueDiffuse" );
	owner.tissueMat:addTexture( "s_bump", "tissueBump" );
	owner.tissueMat:addTexture( "s_normal", "tissueNormal" );
	owner.tissueMat:addTexture( "s_ambient", "tissueAmbient" );
	owner.tissueMat:addTexture( "s_temperature", theTissueSim:getTempMapTextureName() )
	owner.tissueMat:addTexture( "s_condition", theTissueSim:getConditionMapTextureName() )

	owner.tissueMat_debug = spark:createMaterial( "tissueShader_debug" )
	owner.tissueMat_debug:addTexture( "s_temperature", theTissueSim:getTempMapTextureName() )
	owner.tissueMat_debug:addTexture( "s_condition", theTissueSim:getConditionMapTextureName() )

	-- Global theTissueSim is the tissue simulation, declared in C++
	owner.tissueMat:addTexture( "s_temperature", theTissueSim:getTempMapTextureName() )
	owner.tissueMat:addTexture( "s_condition", theTissueSim:getConditionMapTextureName() )
	local tissueScale = 0.25
	-- owner.tissue = spark:createCube( worldOffset + vec3(-0.5*tissueScale, 0, -0.5*tissueScale), 
	-- 	                               tissueScale, owner.tissueMat, "OpaquePass" )
	-- owner.tissue:rotate( 90, vec3(1,0,0) )

	owner.tissue = spark:createPlane( worldOffset + vec3(0,0.5*tissueScale,-0.5*tissueScale), 
		                              vec2(tissueScale, tissueScale), 
		                              ivec2(128,128),
	                                  owner.tissueMat, 
	                                  "WirePass" )
	                                  --"OpaquePass"  )
	
	-- debuging wireframe
	--owner.tissue:setMaterialForPassName( "WirePass", owner.tissueMat )


	if( isShadowOn ) then
		owner.tissue:setMaterialForPassName( "ShadowPass", shadowMaterial )
	end


	-- show the target ring
	owner.tissueMat:setVec2( "u_targetCircleCenter", vec2( 0.7, 0.6 ) )
	owner.tissueMat:setFloat( "u_targetCircleOuterRadius", 0.025 )
	owner.tissueMat:setFloat( "u_targetCircleInnerRadius", 0.024 )
end





return Sim
