--[[
Example of setting up and using a button with callbacks.
--]]

local Button = require "button"
-----------------------------

ButtonExampleState = {}
 
function ButtonExampleState:new()
	print( "ButtonExampleState:new" )
	newObj = { 
		buttons = {}, 
	}
	self.__index = self
	return setmetatable(newObj, self)
end

function ButtonExampleState:load()
	print( "ButtonExampleState:load" )

	spark:createPostProcessingRenderPass( 0.0, 
	      "MSAAFinalRenderPass",
	      "MainRenderTargetTexture", spark:getFrameBufferRenderTarget(), 
	      "texturedOverlayShader" )

	mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
	spark:setMainRenderTarget( mainRenderTarget )
	mainRenderTarget:setClearColor( vec4( 0.2,0.2,0.2,0.5 ) )

	HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", mainRenderTarget )
	HUDRenderPass:setDepthTest( false )
	HUDRenderPass:setDepthWrite( false )
	HUDRenderPass:useInterpolatedBlending()

------------
--[[
	Setup fonts and materials for buttons 
--]]
	local fontMgr = spark:getFontManager()

	fontMgr:addFont( Fonts.defaultFontName, Fonts.defaultFontButtonSize, Fonts.defaultFontFileName )
	fontMgr:addFont( Fonts.defaultFontName, Fonts.defaultFontSmallButtonSize, Fonts.defaultFontFileName )

	local fontDesc = {}
	fontDesc.name = Fonts.defaultFontName
	fontDesc.size = Fonts.defaultFontButtonSize

	-- Normal text color
	fontDesc.material = spark:createMaterial( "TextShader" )
	fontDesc.material:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.material:setVec4( "u_color", vec4( 0.7, 0.7, 0.7, 1 ) )
	-- rollover a bit red 
	fontDesc.rolloverMaterial = spark:createMaterial( "TextShader" )
	fontDesc.rolloverMaterial:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.rolloverMaterial:setVec4( "u_color", vec4( 1.0, 0.7, 0.7, 1 ) )
	-- show white when clicked
	fontDesc.pressedMaterial = spark:createMaterial( "TextShader" )
	fontDesc.pressedMaterial:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	fontDesc.pressedMaterial:setVec4( "u_color", vec4( 1.0, 1.0, 1.0, 1 ) )

	fontMgr:addFont( fontDesc.name, fontDesc.size, Fonts.defaultFontFileName );
--[[
	End setup fonts and materials for buttons 
--]]


--[[
	Create the button at position 0.35, 0.85
	Position is specified relative to the lower-left corner as 0,0
	and the upper-right corner as 1,1
	Note that the given position is for the upper-left corner of the text
--]]
	self.buttons["Tutorial"] = Button:new( 0.35, 0.85, string.format("Tutorial"), fontDesc )

--[[
	Setup optional callbacks for clicking or mousing
--]]
	self.buttons["Tutorial"].onClick = function () print( "Tutorial click" ) end
	self.buttons["Tutorial"].onClick2 = function () print( "Tutorial click2" ) end
	self.buttons["Tutorial"].onMouseOver = function () print( "Tutorial On Mouse Over" ) end
	self.buttons["Tutorial"].onMouseOut = function () print( "Tutorial On Mouse Out" ) end


--[[
	Simple "cursor" to track position
--]]
	local scale = 0.02
	local cursorMat = spark:createMaterial( "constantColorShader" )
	cursorMat:setVec4( "u_color", vec4( 0.2, 0.2, 0.2, 1.0) );
	self.markerBox = spark:createCube( vec3( -scale/2.0, -scale/2.0, -scale/2.0 ), 
		scale, cursorMat, "HUDPass" )
end

function ButtonExampleState:activate()
	print( "ButtonExampleState:activate" )
	self.startTime = -1

end

function ButtonExampleState:update( dt )

--[[
	Calculate cursor position
--]]	
	if( isWindows() ) then
		stylusPos = input:getPosition( "stylus" )
		--
		-- NOTE -- zSpace thinks of screen coords as 0,0 in upper left,
		-- spark defines screen coord 0,0 in lower left
		--
		-- stylusPosScreenCoords = input:getPositionInScreenCoords( "stylus" )
		stylusPos = stylusPos*2 + vec3(0.5,1,0.5)
		stylusMat = input:getTransform( "stylus" )
		buttonState = input:isButtonPressed( "stylus", 0 )
	else
		-- Debugging (and on non-zspace machines, use mouse)
		stylusPos = input:getPosition( "mouse" )
		stylusPos = spark:pixelsToScreenCoords( vec2(stylusPos.x, stylusPos.y) )
		stylusPosScreenCoords = vec2( stylusPos.x, stylusPos.y )
		stylusMat = mat4() --input:getTransform( "mouse" )
		buttonState = input:isButtonPressed( "mouse", 0 )
	end

	self.markerBox:setTransform( mat4() )
	self.markerBox:translate( vec3(stylusPos.x, stylusPos.y, 0) )

--[[
	Update all buttons
--]]
	for id, button in pairs( self.buttons ) do
		button:update( stylusPosScreenCoords.x, stylusPosScreenCoords.y, buttonState )
	end
end

function ButtonExampleState:deactivate()
	print( "ButtonExampleState:deactivate" )
end

function ButtonExampleState:nextState( currTime )
end

theState = ButtonExampleState:new()
theNextState = ""

