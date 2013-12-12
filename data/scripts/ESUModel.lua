local ESUModel = {}
--ESUModel.__index = ESUModel -- failed table lookups go to methods

function ESUModel:new()
	print( "ESUModel:new()" )
	newObj = 
	{
		cutWattage = 30,
		coagWattage = 30,
		mode = ESUINPUT_COAG,
		electrode = ESUINPUT_SPATULA,
		hasCreatedSpark = false,
		ESUModeLabels = { [ESUINPUT_CUT] = "[Cut]", 
						  [ESUINPUT_COAG] = "[Coag]", 
						  [ESUINPUT_BLEND] = "[Blend]" }
	}
	self.__index = self
	return setmetatable( newObj, self )

	-- local self = setmetatable( self, ESUModel )
	-- self.cutWattage = 30
	-- self.coagWattage = 30
	-- self.mode = spark.ESUINPUT_BLEND
	-- self.electrode = spark.ESUINPUT_SPATULA

	-- self.sparkMat = spark:createMaterial( "texturedSparkShader" )
 --    self.sparkIntensity = 1.0
 --    self.sparkMat:setVec4( "u_color", vec4(1.0, 1.0, 0.5, self.sparkIntensity) )
 --    self.mySpark = spark:createLSpark( vec3(1.0, 0, -5), vec3(-1.0, 0, -5),
 --                                         1.0, --intensity
 --                                         0.25, --scale 
 --                                         3, --recursiveDepth
 --                                         0.5, --forkProb
 --                                         "TransparentPass",
 --                                         self.sparkMat )
 --    self.sparkActivationTime = 0
 --    self.sparkPeriod = 0.75

	-- return self
end

function ESUModel:createSpark( )
	self.sparkMat = spark:createMaterial( "texturedSparkShader" )
    self.sparkMat:setVec4( "u_color", vec4( 1.0, 1.0, 0.5, 1.0 ) )
    self.mySpark = spark:createLSpark( vec3(-0.05, 0, 0),
    	                          vec3( 0.05, 0, 0),
                                         1.0, --intensity
                                         0.25, --scale 
                                         3, --recursiveDepth
                                         0.5, --forkProb
                                         "TransparentPass",
                                         self.sparkMat )
    self.sparkActivationTime = 0
    self.sparkPeriod = 0.05
    self.hasCreatedSpark = true
end

function ESUModel:setCutWattage( wattage )
	self.cutWattage = wattage
end

function ESUModel:setCoagWattage( wattage )
	self.coagWattage = wattage
end

function ESUModel:setMode( mode )
	self.mode = mode
end

function ESUModel:update( dt )
	if self.hasCreatedSpark then
		self.mySpark:update( dt )
	end
end

function ESUModel:updateInput( esuInput )
	self.cutWattage = esuInput:cutWattage()
	self.coagWattage = esuInput:coagWattage()
	self.electrode = esuInput:electrode()
	self.mode = esuInput:mode()
end

-- Precondition:  the mode and wattages (coag/cut) have already been set.
function ESUModel:activate( theTissueSim, xpos, ypos, stylusPos, tissueContactPos, distFromTissue, radius, dt )

	local touchThreshold = 0.0004 -- meters

	if( self.mode == ESUINPUT_CUT ) then
		dutyCycle = 0.75
		current = self.cutWattage * dutyCycle 
		voltage = self.cutWattage / dutyCycle
	end
	if( self.mode == ESUINPUT_COAG ) then
		dutyCycle = 0.2
		current = self.coagWattage * dutyCycle
		voltage = self.coagWattage / dutyCycle
	end
	if( self.mode == ESUINPUT_BLEND ) then
		dutyCycle = 0.5
		current = (self.coagWattage + self.cutWattage ) * dutyCycle
		voltage = (self.coagWattage + self.cutWattage ) / dutyCycle
	end

	-- sparkThreshold is proportional to voltage
	-- 10 - 120
	-- min cut, v = 5, max cut = 60
	-- min coag, v = 100, max coag = 1200
	-- @1000V, spark threshold =ex 0.01
	-- @10V,  spark threshold = 0
	local sparkThreshold = touchThreshold + voltage * 0.02/1000 -- meters/volts

	if abs( distFromTissue ) < touchThreshold then
		-- Contact heating
		-- contact area proportional to the depth of penetration

		local widthOfInstrument = 0.005 -- TODO -- get from electrode type

		-- Random jitter for electricity placement
		xpos = xpos + widthOfInstrument * math.random()
		ypos = ypos + widthOfInstrument * math.random()

		-- print(string.format("Touch energy: (%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f)",
		-- 	xpos, ypos, 
		-- 	voltage, current, 
		-- 	dutyCycle, 
		-- 	radius, 
		-- 	dt ) )
		theTissueSim:accumulateElectricalEnergy( xpos, ypos, 
			voltage, current, 
			dutyCycle, 
			radius, 
			dt )
	elseif abs(distFromTissue) < sparkThreshold then
		-- Non-Contact heating
		-- create visual spark from worldCoord  

		-- local spreadAngleRadians = 90.0 * math.pi / 180.0 -- 90 degrees in radians
		-- local spreadSlope = math.tan( spreadAngleRadians * 0.5 )
		-- xpos = xpos + distFromTissue * math.random() * spreadSlope
		-- ypos = ypos + distFromTissue * math.random() * spreadSlope

		local widthOfInstrument = 0.005 -- TODO -- get from electrode type
		if( self.mode == ESUINPUT_COAG ) then
			-- Hacky -- when in coag, always have larger spread
			widthOfInstrument = widthOfInstrument * 1.5
		end
		-- Random jitter for electricity placement
		xpos = xpos + widthOfInstrument * math.random()
		ypos = ypos + widthOfInstrument * math.random()

		-- Draw the spark
		if self.hasCreatedSpark then
			-- self.sparkActivationTime = self.sparkActivationTime + dt
			-- if self.sparkActivationTime > self.sparkPeriod then
			--        self.sparkActivationTime = 0
	        self.mySpark:reseat( stylusPos, --vec3(-0.05, 0, 0),
	        	                 tissueContactPos, --vec3( 0.05, 0, 0),
	                             1.0, --intensity
	                             0.33, --scale 
	                             4, --3, --recursiveDepth
	                             0.4 --forkProb
	                             )
		    -- end

			-- print(string.format("Spark energy: (%2.4f, %2.4f, %2.4f) (%2.4f, %2.4f, %2.4f)",
			-- 	stylusPos.x, stylusPos.y, stylusPos.z,  
			-- 	tissueContactPos.x, tissueContactPos.y, tissueContactPos.z  
			-- ) )
		end


		-- print(string.format("Spark energy: (%2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f, %2.2f)",
		-- 	xpos, ypos, 
		-- 	voltage, current, 
		-- 	dutyCycle, 
		-- 	radius, 
		-- 	dt ) )
		theTissueSim:accumulateElectricalEnergy( xpos, ypos, 
			voltage, current, 
			dutyCycle, 
			radius, 
			dt )
	end


end

-- Shared ESUModel 
ESUModel.theESUModel = ESUModel:new()

return ESUModel