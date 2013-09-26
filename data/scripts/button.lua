-----------------------------

local Button = {}

local largeFontSize = 48
local mediumFontSize = 32
Button.fontMgr = spark:getFontManager()

Button.defaultFontDesc = {}
Button.defaultFontDesc.fontFilename = "HelveticaNeueLight.ttf"
Button.defaultFontDesc.name = "Sans"
Button.defaultFontDesc.size = largeFontSize

Button.defaultFontDesc.material = spark:createMaterial( "TextShader" )
Button.defaultFontDesc.material:addTexture( "s_color", Button.fontMgr:getFontAtlasTextureName() )
Button.defaultFontDesc.material:setVec4( "u_color", vec4( 0.7, 0.7, 0.7, 0.8 ) )

Button.defaultFontDesc.rolloverMaterial = spark:createMaterial( "TextShader" )
Button.defaultFontDesc.rolloverMaterial:addTexture( "s_color", Button.fontMgr:getFontAtlasTextureName() )
Button.defaultFontDesc.rolloverMaterial:setVec4( "u_color", vec4( 0.9, 0.9, 0.9, 0.8 ) )

Button.defaultFontDesc.pressedMaterial = spark:createMaterial( "TextShader" )
Button.defaultFontDesc.pressedMaterial:addTexture( "s_color", Button.fontMgr:getFontAtlasTextureName() )
Button.defaultFontDesc.pressedMaterial:setVec4( "u_color", vec4( 1.0, 1.0, 1.0, 1 ) )

Button.fontMgr:addFont( Button.defaultFontDesc.name, Button.defaultFontDesc.size, Button.defaultFontDesc.fontFilename );
Button.fontMgr:addFont( Button.defaultFontDesc.name, mediumFontSize, Button.defaultFontDesc.fontFilename );

function Button:newLargeButton( posX, posY, msg, onClickFunc )
	Button.defaultFontDesc.size = largeFontSize
	return Button:newButton( posX, posY, msg, Button.defaultFontDesc )
end

function Button:newButton( posX, posY, msg, argFontDesc, onClickFunc, onMouseOverFunc, onMouseOutFunc )
	newObj = 
	{ 
		x = posX, 
		y = posY,
		isPressed = false, 
		isIn = false,
		fontDesc = argFontDesc,
		onClick = onClickFunc, 
		onMouseOver = onMouseOverFunc,
		onMouseOut = onMouseOutFunc,
		renderPassName = "HUDPass"
	}
	if( newObj.onClick == nil ) then 
		newObj.onClick = function(self) 
			print( "nil onClick " ) 
		end
	end
	if( newObj.onClick2 == nil ) then 
		newObj.onClick2 = function(self) 
			print( "nil onClick2 " ) 
		end
	end
	if( newObj.onMouseOver == nil ) then 
		newObj.onMouseOver = function(self) 
			self.text:setMaterialForPassName( self.renderPassName, self.fontDesc.rolloverMaterial )
		end
	end
	if( newObj.onMouseOut == nil ) then 
		newObj.onMouseOut = function(self) 
			self.text:setMaterialForPassName( self.renderPassName, self.fontDesc.material )
		end
	end
	newObj.text = spark:createText( newObj.fontDesc.name, 
		                            newObj.fontDesc.size, 
		                            newObj.fontDesc.material, 
		                            newObj.renderPassName, 
		                            msg )
	newObj.text:translate( newObj.x, newObj.y, 0 )
	self.__index = self
	return setmetatable( newObj, self )
end

function Button:setText( msg )
	self.text:setText( msg )
end

function Button:update( projectedMousePosX, projectedMousePosY, buttonState )
	-- padding around button for near-misses
	local buttonPadding = vec2( 10,15 )
	local buttonSizeInScreenCoords = spark:pixelsToScreenCoords( self.text:getSizeInPixels() + buttonPadding )
	buttonSizeInScreenCoords.y = 1 - buttonSizeInScreenCoords.y -- pixels to screen coords thinks 

	local isHitX = (projectedMousePosX < (self.x + buttonSizeInScreenCoords.x)) and (projectedMousePosX > self.x)
	local isHitY = (projectedMousePosY < self.y) and (projectedMousePosY > (self.y - buttonSizeInScreenCoords.y) ) 
	local isHit = isHitX and isHitY 

	-- Handle rollover visual effect
	if( isHit ) then
		if( not self.isIn ) then
			-- only send event on first mouse entering
			self:onMouseOver()
			self.isIn = true
			if self.fontDesc and self.fontDesc.rolloverMaterial then
				self.text:setMaterialForPassName( self.renderPassName, self.fontDesc.rolloverMaterial )
			end
		end
	else
		if( self.isIn ) then
			-- only send event on first mouse leaving
			self:onMouseOut()
			self.isIn = false
			self.text:setMaterialForPassName( self.renderPassName, self.fontDesc.material )
		end
	end

	-- Accept button clicks
	if( buttonState and isHit and self.onClick ) then
		-- mouse is down in the button for the first time
		if self.fontDesc and self.fontDesc.pressedMaterial then
			self.text:setMaterialForPassName( self.renderPassName, self.fontDesc.pressedMaterial )
		end
		self.isPressed = true
	end
	-- isPressed is going down while in the button
	--if( buttonState and isHit and not self.isPressed)
	if( not buttonState and isHit and self.isPressed and self.onClick ) then
		-- mouse is up inside the button for first time -- a click!
		self.text:setMaterialForPassName( self.renderPassName, self.fontDesc.material )
		self:onClick()
		self.isPressed = false
	end
	-- Handle releases outside the button
	if( not buttonState ) then
		self.isPressed = false
	end
end


-------------------------------------------------------
return Button
-------------------------------------------------------
