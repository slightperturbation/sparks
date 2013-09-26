-----------------------------
--[[
	Tissue Simulation methods

--]]

local Sim = {}


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
	owner.wattDisplay:translate( 0.05, 0.95, 0 )
	wattUnitMsg = spark:createText( fontDesc.name, 
		                            smallFontSize, 
		                            fontDesc.material, 
		                            "HUDPass", 
		                            "Watts" )
	wattUnitMsg:translate( 0.0875, 0.945, 0 )
	-- Mode
	owner.modeDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             "[ESUMODE]" )
	owner.modeDisplay:translate( 0.25, 0.95, 0 )
	-- Activation Time
	owner.activationTimeDisplay = spark:createText( fontDesc.name, 
 			                             fontDesc.size, 
			                             fontDesc.material,
			                             "HUDPass", 
			                             string.format("%2.1f", owner.activationTime) )
	owner.activationTimeDisplay:translate( 0.5, 0.95, 0 )
	timeUnitMsg = spark:createText( fontDesc.name, 
		                            smallFontSize, 
		                            fontDesc.material, 
		                            "HUDPass", 
		                            "Seconds" )
	timeUnitMsg:translate( 0.54, 0.945, 0 )

	-- --Highlight GUI w/ quad
	local bgAccentMat = spark:createMaterial( "constantColorShader" )
	bgAccentMat:setVec4( "u_color", vec4(0,0,0,0.35) )
	local bgQuad = spark:createQuad( vec2(0,0.9), vec2(1.0,1.0),
		bgAccentMat, "HUDPass" )
	--bgQuad:translate( 0,0,-1 )
	----
end

function Sim.createTissue( owner, worldOffset )

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


		owner.tissueMat = spark:createMaterial( "tissueShader" ) --"tissueShader_procedural"  )
	
		owner.tissueMat:addTexture( "s_shadowMap", "light0_shadowMap" )

		owner.tissueMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
		owner.tissueMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
		owner.tissueMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
		owner.tissueMat:setVec4( "u_ka", vec4(1,1,1,1) )
		owner.tissueMat:setVec4( "u_kd", vec4(1,1,1,1) )
		owner.tissueMat:setVec4( "u_ks", vec4(1,1,1,1) )
		owner.tissueMat:setFloat( "u_ns", 100.0 )
		owner.tissueMat:setFloat( "u_activationTime", 0.0 )

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
		owner.tissue = spark:createCube( worldOffset + vec3(-0.5*tissueScale, 0, -0.5*tissueScale), tissueScale, owner.tissueMat, "OpaquePass" )

		if( isShadowOn ) then
			owner.tissue:setMaterialForPassName( "ShadowPass", shadowMaterial )
		end

		owner.tissue:rotate( 90, vec3(1,0,0) )

		-- show the target ring
		owner.tissueMat:setVec2( "u_targetCircleCenter", vec2( 0.7, 0.6 ) )
		owner.tissueMat:setFloat( "u_targetCircleOuterRadius", 0.025 )
		owner.tissueMat:setFloat( "u_targetCircleInnerRadius", 0.024 )
end





return Sim
