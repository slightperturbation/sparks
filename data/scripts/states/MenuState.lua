----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
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
	self.explanationText = spark:createText( "Sans", 
		                                     32, 
		                                     explanationMat, 
		                            		 "HUDPass", "..." )
	self.explanationText:translate( vec3( 0.5, 0.75, 0 ) ) 
	self.explanationText:setText( "Select a lesson from the list to the left." )

	-------------------------------------------------
	-- Buttons
	local xpos = 0.1
	local ypos = 0.9
	local lineHeight = 0.15
	self.buttons["Introduction"] = Button:newLargeButton( xpos, ypos, "Introduction" )
	self.buttons["Introduction"].onClick = function () self.theNextState = "Introduction" end
	self.buttons["Introduction"].onMouseOver = function () 
		self.explanationText:setText(
[[A brief overview of the VEST hand's-on 
electosurgery simulator.]])
	end

	ypos = ypos - lineHeight
	self.buttons["ESU Power"] = Button:newLargeButton( xpos, ypos, "ESU Power" )
	self.buttons["ESU Power"].onClick = function () 
		self.theNextState = "ESUPower" 
		print("Switching to ESUPower state")
end
	self.buttons["ESU Power"].onMouseOver = function ()
		self.explanationText:setText( 
[[The power setting (wattage) has the
and most direct impact on tissue temperature.]])
	end

	ypos = ypos - lineHeight
	self.buttons["ESU Modes"] = Button:newLargeButton( xpos, ypos, "ESU Modes" )
	self.buttons["ESU Modes"].onClick = function () self.theNextState = "ESUModes" end
	self.buttons["ESU Modes"].onMouseOver = function ()
		self.explanationText:setText(
[[Learn the role of each of the three modes of 
operation of the ESU:
	
	Coag, Cut and Blend]])
	end

	ypos = ypos - lineHeight
	self.buttons["Contact Area"] = Button:newLargeButton( xpos, ypos, "Contact Area" )
	self.buttons["Contact Area"].onClick = function ()  self.theNextState = "" end
	self.buttons["Contact Area"].onMouseOver = function () 
		self.explanationText:setText(
[[The area of contact between the electrode (bovie) 
and the tissue has a surprisingly large impact on the
heating effect.]])
	end

	ypos = ypos - lineHeight
	self.buttons["Freestyle"] = Button:newLargeButton( xpos, ypos, "Freestyle" )
	self.buttons["Freestyle"].onClick = function ()  self.theNextState = "Simulation" end
	self.buttons["Freestyle"].onMouseOver = function () 
		self.explanationText:setText( 
[[Experiment freely on simulated tisse.]])
	end

	self.cursorScale = 0.02
	local cursorMat = spark:createMaterial( "constantColorShader" )
	cursorMat:setVec4( "u_color", vec4( 1, 0.5, 0.5, 0.5) );
	self.cursor = spark:loadMesh( "sphere.obj", cursorMat, "HUDPass" )

end

function MenuState:activate()
	print( "MenuState:activate" )
	self.startTime = -1

end

function MenuState:update( dt )
	if( isWindows() ) then
		inputDeviceName = "stylus"
	else
		inputDeviceName = "mouse"
	end
	stylusPos = input:getPosition( inputDeviceName )
	stylusScreenPos = input:getScreenPosition( inputDeviceName )
	stylusMat = input:getTransform( inputDeviceName )
	buttonState = input:isButtonPressed( inputDeviceName, 0 )

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
	-- For now, theNextState global is used to pass
	-- the next desired state back to the app
	-- TODO should be changed to use the return value
	-- print( "MenuState:nextState( " .. currTime .. " )")
	-- if self.startTime == -1 then
	-- 	self.startTime = currTime
	-- end

	-- if (currTime - self.startTime) > 10 then 
	-- 	theNextState = "Loading" 
	-- 	print( "Changing state to menu!" )
	-- else
	-- 	theNextState = ""
	-- end
end

theState = MenuState:new()
theNextState = ""

