----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
local Fonts = require "Fonts"
Fonts:init()
----------------------------------------

MenuState = {}
 
function MenuState:new()
	print( "MenuState:new" )
	newObj = { 
		angle = 45, 
		hasRunOnce = false, 
		startTime = -1,
		buttons = {}, 
		theNextState = ""
	}
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
	mainRenderTarget:setClearColor( vec4( 0.2,0.2,0.2,0.5 ) )

	HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", mainRenderTarget )
	HUDRenderPass:setDepthTest( false )
	HUDRenderPass:setDepthWrite( false )
	HUDRenderPass:useInterpolatedBlending()


	local fontMgr = spark:getFontManager()

	explanationMat = spark:createMaterial( "TextShader" )
	explanationMat:addTexture( "s_color", fontMgr:getFontAtlasTextureName() )
	explanationMat:setVec4( "u_color", vec4( 0.8, 0.8, 0.8, 0.8 ) )
	self.explanationText = spark:createText( Fonts.defaultFontName, 
		                                     Fonts.defaultFontTextSize, 
		                                     explanationMat, 
		                            		 "HUDPass", "..." )
	self.explanationText:translate( vec3( 0.5, 0.75, 0 ) ) 
	self.explanationText:setText( "Select a lesson from the list to the left." )

	-------------------------------------------------
	-- Buttons
	local xpos = 0.1
	local ypos = 0.9
	local lineHeight = 0.15
-- 	self.buttons["Introduction"] = Button:newLargeButton( xpos, ypos, "1", "Introduction" )
-- 	self.buttons["Introduction"].onClick = function () self.theNextState = "Introduction" end
-- 	self.buttons["Introduction"].onMouseOver = function () 
-- 		self.explanationText:setText(
-- [[A brief overview of the VEST hands-on 
-- electosurgery simulator.]])
-- 	end

	ypos = ypos - lineHeight
	self.buttons["ESU Power"] = Button:newLargeButton( xpos, ypos, KEY_KP_1, "[1] ESU Power" )
	self.buttons["ESU Power"].onClick = function () 
		self.theNextState = "ESUPower" 
		print("Switching to ESUPower state")
end
	self.buttons["ESU Power"].onMouseOver = function ()
		self.explanationText:setText( 
[[The power setting (wattage) has the
and most direct impact on tissue effects.]])
	end

	ypos = ypos - lineHeight
	self.buttons["ESU Modes"] = Button:newLargeButton( xpos, ypos, KEY_KP_2, "[2] ESU Modes" )
	self.buttons["ESU Modes"].onClick = function () self.theNextState = "ModeInstruction" end
	self.buttons["ESU Modes"].onMouseOver = function ()
		self.explanationText:setText(
[[Learn the effects of the modes of 
operation of the ESU:
	
	Coag, Cut and Blend]])
	end

-- 	ypos = ypos - lineHeight
-- 	self.buttons["Contact Area"] = Button:newLargeButton( xpos, ypos, KEY_KP_3, "[3] Contact Area" )
-- 	self.buttons["Contact Area"].onClick = function ()  self.theNextState = "" end
-- 	self.buttons["Contact Area"].onMouseOver = function () 
-- 		self.explanationText:setText(
-- [[The area of contact between the electrode 
-- and the tissue has a surprisingly large impact on the
-- heating effect.]])
-- 	end

-- 	ypos = ypos - lineHeight
-- 	self.buttons["Freestyle"] = Button:newLargeButton( xpos, ypos, KEY_KP_4, "[4] Simulation" )
-- 	self.buttons["Freestyle"].onClick = function ()  self.theNextState = "Simulation" end
-- 	self.buttons["Freestyle"].onMouseOver = function () 
-- 		self.explanationText:setText( 
-- [[Experiment freely on simulated tisse.]])
-- 	end

	self.cursorScale = 0.025
	local cursorMat = spark:createMaterial( "constantColorShader" )
	cursorMat:setVec4( "u_color", vec4( 1, 0.5, 0.5, 0.5) );
	self.cursor = spark:loadMesh( "sphere.obj", cursorMat, "HUDPass" )

end

function MenuState:activate()
	print( "MenuState:activate" )
	self.startTime = -1
	self.theNextState = ""
end

function MenuState:update( dt )
	stylusPos = input:getDefaultDevicePosition()
	stylusScreenPos = input:getDefaultDeviceScreenPosition( )
	stylusMat = input:getDefaultDeviceTransform( )
	buttonState = input:isDefaultDeviceButtonPressed( 0 )

	self.cursor:setTransform( mat4() )
	self.cursor:translate( vec3(stylusScreenPos.x, stylusScreenPos.y, 0.1) )
	self.cursor:scale( self.cursorScale )
	self.cursor:scale( vec3(1080/1920, 1, 1) )
	for id, button in pairs( self.buttons ) do
		button:update( stylusScreenPos.x, stylusScreenPos.y, buttonState )
	end
end

function MenuState:deactivate()
	print( "MenuState:deactivate" )
	self.theNextState = ""
end

function MenuState:nextState( currTime )
	theNextState = self.theNextState
end

theState = MenuState:new()
theNextState = ""

