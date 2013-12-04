----------------------------------------
-- Include standard libraries
local Button = require "button"
local Render = require "render"
----------------------------------------

ModeInstructionState = {}
 
function ModeInstructionState:new()
	print( "ModeInstructionState:new" )
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

function ModeInstructionState:load()
	print( "ModeInstructionState:load" )

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
	explanationMat:setVec4( "u_color", vec4( 0.9, 0.9, 0.9, 1.0 ) )
	self.explanationText = spark:createText( "Sans", 
		                                     32, 
		                                     explanationMat, 
		                            		 "HUDPass", "..." )
	self.explanationText:translate( vec3( 0.5, 0.75, 0 ) ) 
	self.explanationText:setText( 
		[[The two major ESU (electrosurgical unit) modes
 are 'cut' and 'coag'.

Choose a mode.
 ]] )

	-------------------------------------------------
	-- Buttons
	local xpos = 0.1
	local ypos = 0.9
	local lineHeight = 0.15
	self.buttons["Cut Mode"] = Button:newLargeButton( xpos, ypos, KEY_KP_1, "[1] Cut Mode" )
	self.buttons["Cut Mode"].onClick = function () self.theNextState = "CutMode" end
	self.buttons["Cut Mode"].onMouseOver = function () 
		self.explanationText:setText(
[[The CUT mode provides a 100% duty cycle,
allowing vaporization with less dessication.

The CUT lesson asks you to:

  1) Create a spark from the tool to the tissue,
     you should learn that electrical arcs are 
     more difficult to create in cut mode than coag mode.

  2) Vaporize a small region of tissue with minimal
		thermal spread.
     You should learn that cut minimizes spread 
     compared to coag.
]])
	end

	ypos = ypos - lineHeight
	self.buttons["Coag Mode"] = Button:newLargeButton( xpos, ypos, KEY_KP_2, "[2] Coag Mode" )
	self.buttons["Coag Mode"].onClick = function () 
		self.theNextState = "CoagMode" 
		print("Switching to Simulation state")
end
	self.buttons["Coag Mode"].onMouseOver = function ()
		self.explanationText:setText( 
[[The COAG mode outputs high-voltage
bursts, allowing easier arcing to nearby tissue.
Often used to fulgurate a broad area of tissue.

The COAG lesson asks you to:

  1) Create a spark from the tool to the tissue,
     you should learn that electrical arcs are 
     much easier to create in coag mode than cut mode,
     and that the electricity can arc further.

  2) Vaporize a small region of tissue with minimal
		thermal spread.
     You should learn that coag is more difficult to
     limit the spread compared to cut.
]])
	end

-- 	ypos = ypos - lineHeight
-- 	self.buttons["Blend Mode"] = Button:newLargeButton( xpos, ypos, KEY_KP_3, "Blend Mode" )
-- 	self.buttons["Blend Mode"].onClick = function () self.theNextState = "Simulation" end
-- 	self.buttons["Blend Mode"].onMouseOver = function ()
-- 		self.explanationText:setText(
-- [[Provides a blend that
-- has features of both Cut and Coag.
-- ]])
-- 	end


	self.cursorScale = 0.02
	local cursorMat = spark:createMaterial( "constantColorShader" )
	cursorMat:setVec4( "u_color", vec4( 1, 0.5, 0.5, 0.8) );
	self.cursor = spark:loadMesh( "sphere.obj", cursorMat, "HUDPass" )

end

function ModeInstructionState:activate()
	print( "ModeInstructionState:activate" )
	self.startTime = -1

end

function ModeInstructionState:update( dt )
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

function ModeInstructionState:deactivate()
	print( "ModeInstructionState:deactivate" )
	self.theNextState = ""
end

function ModeInstructionState:nextState( currTime )
	theNextState = self.theNextState
	-- For now, theNextState global is used to pass
	-- the next desired state back to the app
	-- TODO should be changed to use the return value
	-- print( "ModeInstructionState:nextState( " .. currTime .. " )")
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

theState = ModeInstructionState:new()
theNextState = ""

