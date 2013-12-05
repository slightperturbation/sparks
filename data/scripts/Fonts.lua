
--[[
Load Common Fonts

Including a font here causes it to be loaded
in the initial build of FontManager's atlas
It's not required, and later addFont() calls
will still be honored, but they can be slow
thus it's genenerally better to build the atlas
only once.

Usage:
	local Fonts = require "Fonts"
	Fonts:init()
--]]

local Fonts = {}

-- System-wide constants, choose platform native fonts and suffer with 
-- bad formatting
Fonts.defaultFontFileName = "Vera.ttf"
if isApple() then
	Fonts.defaultFontFileName = "FiraSans-Regular.ttf" -- "FiraSans-Light.ttf" 
end
--Fonts.defaultFontFileName = "HelveticaNeueLight.ttf" 

Fonts.defaultFontName = "Sans"

Fonts.defaultFontHeaderSize = 64

Fonts.defaultFontButtonSize = 48
Fonts.defaultFontSmallButtonSize = 32

Fonts.defaultFontLabelSize = 48
Fonts.defaultFontUnitsSize = 16

Fonts.defaultFontTextSize = 24

--[[
	Fonts.init() is idpotent-- calling it many times is unproductive, but fine.
	It's real purpose is to prepare the list of fonts so the atlas only gets
	created once.
--]]
function Fonts.init()

	local fontMgr = spark:getFontManager()

	fontMgr:addFont( "Sans", Fonts.defaultFontHeaderSize, Fonts.defaultFontFileName )

	fontMgr:addFont( "Sans", Fonts.defaultFontButtonSize, Fonts.defaultFontFileName )
	fontMgr:addFont( "Sans", Fonts.defaultFontSmallButtonSize, Fonts.defaultFontFileName )

	fontMgr:addFont( "Sans", Fonts.defaultFontLabelSize, Fonts.defaultFontFileName )
	fontMgr:addFont( "Sans", Fonts.defaultFontUnitsSize, Fonts.defaultFontFileName )

	fontMgr:addFont( "Sans", Fonts.defaultFontTextSize, Fonts.defaultFontFileName )
end

return Fonts
