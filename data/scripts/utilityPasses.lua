
local utilityPasses = {}




-------------------------------------------------------------------------------------
-- Blur the texture in textureName and render the blurred result to renderTarget
-- basePriority is the starting priority of the first blur render pass (later passes
-- have lower priority by 0.0001).  The compositePriority is the priority of the rendered
-- pass that will add the blur to renderTarget
function utilityPasses.blurPassFromTextureToTarget( textureName, 
                                                    renderTarget, 
                                                    blurPriority, 
                                                    compositePriority, 
                                                    scaleFactor, 
                                                    blurRadius,
                                                    blurIntensity )
    print( "blurPassFromTextureToTarget called." )
      if( blurPriority < compositePriority ) then
            print( "Error: blurPriority before compositePriority in blurPassFromTextureToTarget")
            -- Todo, throw an exception
            -- Todo, log error (luabind log funcs)
      end
      local bgColor = vec4( 1,1,1,0 )

      -- Horizontal blur pass
      local blurHortMaterial = spark:createMaterial( "blurHortShader" )
      blurHortMaterial:addTexture( "s_color", textureName )
      blurHortMaterial:setFloat( "u_blurRadius", blurRadius )
      local blurHortRenderTarget = spark:createScaledTextureRenderTarget( textureName .. "_TmpBlurHortTexture",
            scaleFactor )
      blurHortRenderTarget:setClearColor( bgColor )
      hortPass = spark:createPostProcessingRenderPass( blurPriority, 
            textureName .. "_BlurHortPass",
            blurHortRenderTarget, 
            blurHortMaterial )
      hortPass:disableBlending()

      -- Vertical blur pass
      -- draw onto the renderTarget specified in arg
      local blurVertMaterial = spark:createMaterial( "blurVertShader" )
      blurVertMaterial:addTexture( "s_color", textureName .. "_TmpBlurHortTexture" )
      blurVertMaterial:setFloat( "u_blurRadius", blurRadius )
      blurVertMaterial:setVec4( "u_color", blurIntensity )
      vertPass = spark:createPostProcessingRenderPass( blurPriority - 0.0001,
            textureName .. "_blurVertPass",
            renderTarget,
            blurVertMaterial )
      return vertPass
end

-------------------------------------------------------------------------------------
-- setupGlowPass can be used to add glow effect that uses gaussian blur.
-- To use, assign Renderables glow materials for the glowRenderPassName render pass.
--
-- First, setup a texture to render to ("GlowRenderPassTexture1") and
-- attach it to a render pass that renders using the default camera
-- This new render pass (glowPass) assigns a default material to 
-- all objects but those that glow using the :useDefaultMaterial call.
-- Glow render passes:
--      10.9: spark -> glowPass -> glowTexture (0.5)
--      10.8: glowTexture -> horzBlurPass -> glowTexture2 (0.5)
--      10.7: glowTexture2 -> vertBlurPass -> glowTexture3 (0.5)
--      0.9 : glowTexture3 -> transparencyPass -> mainTarget
-- 
-- first render opaque (default material) objects to glowRenderPassName .. "_GlowPassTargetTexture" 
--  using depth write 
-- then render glow-texture objects 
--  using depth test but not depth write
--
function utilityPasses.setupGlowPass( glowRenderPassName )
      print( 'Setting up glow render pass: "' .. glowRenderPassName .. '"' )

      -- Render scene to _GlowPassTargetTexture
      local glowTargetScale = 0.25
      local glowRenderTarget = spark:createScaledTextureRenderTarget( glowRenderPassName .. "_GlowPassTargetTexture", glowTargetScale )
      glowRenderTarget:setClearColor( vec4(1,1,1,0) )

      local bgMaterial = spark:createMaterial( "constantColorShader" )
      bgMaterial:setVec4( "u_color", vec4(1,1,1,0) )
      local glowPass = spark:createRenderPass( 10.9, glowRenderPassName, glowRenderTarget )
      glowPass:useDefaultMaterial( bgMaterial )

-- TODO --
-- Need two passes, one for the "black" default material
-- and then the second pass for glow?
--
      
      local blurCompositePass = utilityPasses.blurPassFromTextureToTarget( 
                                    glowRenderPassName .. "_GlowPassTargetTexture", 
                                    mainRenderTarget, 
                                    10.8, -- glow render priority
                                    0.75, -- composite priority
                                    0.2, -- scale factor
                                    0.05, -- blur radius
                                    vec4(1,1,1,1) -- blur intensity
                                    )
      
      --blurCompositePass:setBlending( GL_SRC_ALPHA, GL_DST_ALPHA, GL_FUNC_ADD )
      --blurCompositePass:setBlending( GL_SRC_ALPHA, GL_DST_ALPHA, GL_MAX ) --GL_FUNC_ADD )
      blurCompositePass:setBlending( GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD )
      --blurCompositePass:setBlending( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA )
      -- GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_MAX 
      blurCompositePass:setDepthWrite( false )
      blurCompositePass:setDepthTest( true )
      print( "Created glow pass: '" .. glowRenderPassName .. "_GlowPassTargetTexture" .. "'" )
      return glowPass
end

return utilityPasses


