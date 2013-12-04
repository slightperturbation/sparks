----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Sim = require "Sim"
local ESUModel = require "ESUModel"
----------------------------------------

--[[
	Prototypical Simulation State 

	Create simulation states based on this model.

	New simulation states must also be loaded in main()
	by adding the base name (without "State") to the simStates vector
--]]

SimulationState = {}

function SimulationState:new()
	print( "SimulationState:new" )
	newObj = 
	{ 
		buttons = {}, 
		hasVibrated = false,
 		startTime = -1, 
		-- ESUModeLabels = { [ESUINPUT_CUT] = "[Cut]", 
		--                   [ESUINPUT_COAG] = "[Coag]", 
		--                   [ESUINPUT_BLEND] = "[Blend]" },
		activationTime = 0,
		currTime = 0, 
		contactArea = 0,  -- area of contact (m^2) between tissue and electrode
		tissueDistance = 0,  -- distance to tissue in mm
		theNextState = "",

	}
	self.__index = self
	return setmetatable(newObj, self)
end


function SimulationState:load()
	print( "SimulationState:load" )

	local isShadowOn = true

	Render:createDefaultRenderPasses( isShadowOn )

	Sim.load( self )

	ESUModel.theESUModel:createSpark()


	---------------------------------------------------------

	---------------------------------------------------------

	---------------------------------------------------------

	local fontMgr = spark:getFontManager()
	local fontDesc = {}
	fontDesc.name = "Sans"
	fontDesc.fontFilename = "HelveticaNeueLight.ttf"
	fontDesc.size = 32
	fontDesc.material = spark:createMaterial( "TextShader" )
	fontDesc.material:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.material:setVec4( "u_color", vec4( 1, 1, 0.8, 1 ) )

	fontMgr:addFont( fontDesc.name, fontDesc.size, fontDesc.fontFilename )
	self.mainText = spark:createText( fontDesc.name, 
									  fontDesc.size, 
									  fontDesc.material,
						"HUDPass", 
						"THis is some\nText that should be on the screen\nLike so." )
	self.mainText:translate( 0.025, 0.785, 0 )

	-- --Highlight GUI w/ quad
	local bgAccentMat = spark:createMaterial( "constantColorShader" )
	bgAccentMat:setVec4( "u_color", vec4( 0, 0, 0, 0.2 ) )
	local bgQuad = spark:createQuad( 
		vec2( 0.015, 0.4 ), -- position of lower-left corner, 0,0 is bottom-left of screen
		vec2( 0.25,0.5 ),  -- size
		bgAccentMat, "HUDUnderPass" )

	---------------------------------------------------------

	---------------------------------------------------------

	---------------------------------------------------------


	---------------------------------------------------------
	-- Debugging Markers

	local scale = 0.0025
	local redMat = spark:createMaterial( "constantColorShader" )
	redMat:setVec4( "u_color", vec4( 1, 0.2, 0.2, 1.0) );
	local greenMat = spark:createMaterial( "constantColorShader" )
	greenMat:setVec4( "u_color", vec4( 0.2, 1, 0.2, 1.0) );

	-- Tmp -- 3D mouse cursor
	useMouseCursorCube = false
	if( useMouseCursorCube ) then
		self.markerBox = spark:createCube( vec3( -scale/1.5, -scale/2.0, -scale/2.0 ), 
			scale, redMat, "OpaquePass" )
		if( isShadowOn ) then
			self.markerBox:setMaterialForPassName( "ShadowPass", shadowMaterial )
		end
		
		self.markerBox2 = spark:createCube( vec3( -scale/2.0, -scale/1.5, -scale/2.0 ), 
			scale, greenMat, "OpaquePass" )
		if( isShadowOn ) then
			self.markerBox2:setMaterialForPassName( "ShadowPass", shadowMaterial )
		end
	end

	-- Here's a nice little marker for the origin
	local useZeroMarker = false
	if( useZeroMarker ) then
		local scale = 0.01

		local rMat = spark:createMaterial( "constantColorShader" )
		rMat:setVec4( "u_color", vec4( 1, 0.1, 0.1, 0.5 ) )
		local boxX = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
			                          scale, rMat, "WirePass" )
		boxX:scale( vec3(1, 0.1, 0.1) )

		local gMat = spark:createMaterial( "constantColorShader" )
		gMat:setVec4( "u_color", vec4( 0.1, 1, 0.1, 0.5 ) )
		local boxY = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
			                          scale, gMat, "WirePass" )
		boxY:scale( vec3(0.1, 1, 0.1) )

		local bMat = spark:createMaterial( "constantColorShader" )
		bMat:setVec4( "u_color", vec4( 0.1, 0.1, 1, 0.5 ) )
		local boxZ = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
			                          scale, bMat, "WirePass" )
		boxZ:scale( vec3(0.1, 0.1, 1) )
		--box:translate( self.worldOffset )	
		if( isShadowOn ) then
			boxX:setMaterialForPassName( "ShadowPass", shadowMaterial )
			boxY:setMaterialForPassName( "ShadowPass", shadowMaterial )
			boxZ:setMaterialForPassName( "ShadowPass", shadowMaterial )
		end
	end

	local use10CMMarkers = false
	if use10CMMarkers then
	    self.boxMat = spark:createMaterial( "colorShader" )
	    self.boxMat:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )

	    --self.boxA = spark:createCube( vec3(-0.0105, -0.15, -0.1172), 0.01, self.boxMat, "OpaquePass" )
	    self.boxA = spark:createCube( vec3(-0.05, 0, 0), 0.01, self.boxMat, "OpaquePass" )
		if( isShadowOn ) then
			self.boxA:setMaterialForPassName( "ShadowPass", shadowMaterial )
		end
		self.boxB = spark:createCube( vec3( 0.05, 0, 0), 0.01, self.boxMat, "OpaquePass" )
		if( isShadowOn ) then
			self.boxB:setMaterialForPassName( "ShadowPass", shadowMaterial )
		end
	end


end

function SimulationState:activate()
	print( "SimulationState:activate" )
	self.startTime = -1

	Sim.activate( self )
end


function SimulationState:update( dt )
	-- Convey updates from the UI to the current ESU settings
 	ESUModel.theESUModel:updateInput( theESUInput )

 	Sim.update( self, dt )

end

function SimulationState:deactivate()
	print( "SimulationState:deactivate" )


end

function SimulationState:nextState( currTime )
	theNextState = self.theNextState
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
	if( isWindows() ) then
		if input:isButtonPressed( "stylus", 2 ) then
			theNextState = "Menu"
		end
	end
end

theState = SimulationState:new()
theNextState = ""

