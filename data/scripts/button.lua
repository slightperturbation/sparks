-----------------------------

local Button = {}

function Button:new( posX, posY, msg, argFontDesc, onClickFunc, onMouseOverFunc, onMouseOutFunc )
	newObj = 
	{ 
		x = posX, 
		y = posY, 
		fontDesc = argFontDesc,
		onClick = onClickFunc, 
		onMouseOver = onMouseOverFunc,
		onMouseOut = onMouseOutFunc
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
			self.text:setMaterialForPassName( "HUDPass", self.fontDesc.rolloverMaterial )
		end
	end
	if( newObj.onMouseOut == nil ) then 
		newObj.onMouseOut = function(self) 
			self.text:setMaterialForPassName( "HUDPass", self.fontDesc.material )
		end
	end
	newObj.text = spark:createText( newObj.fontDesc.name, 
		                            newObj.fontDesc.size, 
		                            newObj.fontDesc.material, 
		                            "HUDPass", msg )
	newObj.text:translate( newObj.x, newObj.y, 0 )
	self.__index = self
	return setmetatable( newObj, self )
end

function Button:isOver( mousePos )
	local buttonSizeInPixels = self.text:getSizeInPixels()
	local hudPass = spark:getRenderPassByName( "HUDPass" )
	local hudSizeInPixels = hudPass:targetSize()
	-- convert button LL corner to pixels
	local buttonX = self.x * hudSizeInPixels.x
	local buttonY = (self.y * hudSizeInPixels.y) - 10
	--local buttonY = hudSizeInPixels.y - ((self.y * hudSizeInPixels.y) - buttonSizeInPixels.y)
	local mx = mousePos.x
	local my = hudSizeInPixels.y - mousePos.y
	if( false ) then
		print( string.format( "Mouse      : %f, %f", mx, my ) )
		print( string.format( "Button Size: %f, %f", buttonSizeInPixels.x, buttonSizeInPixels.y ) )
		print( string.format( "ButtonA    : %f, %f", buttonX, buttonY ) )
		print( string.format( "ButtonB    : %f, %f", buttonX + buttonSizeInPixels.x, buttonY - buttonSizeInPixels.y ) )
	end
	local isHitX = false
	local isHitY = false
	if( (mx > buttonX) and (mx < (buttonX + buttonSizeInPixels.x)) ) then
		isHitX = true
	end
	if( (my < buttonY) and (my > (buttonY - buttonSizeInPixels.y)) ) then
		isHitY = true
	end
	return( isHitX and isHitY )
end

return Button


