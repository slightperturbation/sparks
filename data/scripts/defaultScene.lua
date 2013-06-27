-- Change the standard error function so that it shows the debug traceback.

print( "Begin defaultScene.lua" );


print( "\tDefine utility functions." )
-------------------------------------------------------------------------------------
-- Blur the texture in textureName and render the blurred result to renderTarget
-- basePriority is the starting priority of the first blur render pass (later passes
-- have lower priority by 0.0001).  The compositePriority is the priority of the rendered
-- pass that will add the blur to renderTarget
function blurPassFromTextureToTarget( textureName, renderTarget, blurPriority, compositePriority )
    print( "blurPassFromTextureToTarget called." )
      if( blurPriority < compositePriority ) then
            print( "Error: blurPriority before compositePriority in blurPassFromTextureToTarget")
            -- Todo, throw an exception
            -- Todo, log error (luabind log funcs)
      end
      local blurHortTarget = spark:createPostProcessingRenderPassAndTarget( blurPriority, 
            textureName .. "_BlurHortPass",
            textureName, textureName .. "_TmpBlurHortTexture", 
            "blurHortShader" )
      blurHortTarget:setClearColor( blackColor )

      local blurVertTarget = spark:createPostProcessingRenderPassAndTarget( blurPriority - 0.0001, 
            textureName .. "_BlurVertPass",
            textureName .. "_TmpBlurHortTexture", textureName .. "_tmpBlurVertTexture", 
            "blurVertShader" )
      blurVertTarget:setClearColor( blackColor )

      local blurCompositePass = spark:createPostProcessingRenderPass( compositePriority, 
            textureName .. "_BlurCompositePass",
            textureName .. "_tmpBlurVertTexture", renderTarget, 
            "texturedOverlayShader" )
      return blurCompositePass
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
function setupGlowPass( glowRenderPassName )
      print( 'Setting up glow render pass: "' .. glowRenderPassName .. '"' )
      local glowRenderTarget = spark:createScaledTextureRenderTarget( glowRenderPassName .. "_GlowPassTargetTexture", 1 )
      glowRenderTarget:setClearColor( vec4(1,1,1,0) )

      local blackMaterial = spark:createMaterial( "constantColorShader" )
      blackMaterial:setVec4( "u_color", vec4(0,0,0,0) )
      local glowPass = spark:createRenderPass( 10.9, glowRenderPassName, glowRenderTarget )
      --glowPass:useAdditiveBlending()
      glowPass:useDefaultMaterial( blackMaterial )

      local blurCompositePass = blurPassFromTextureToTarget( glowRenderPassName .. "_GlowPassTargetTexture", 
                                                       mainRenderTarget, 
                                                       10.8, 0.01 )
      --blurCompositePass:useAdditiveBlending()
      print( "Created glow pass: '" .. glowRenderPassName .. "_GlowPassTargetTexture" .. "'" )
      return glowPass
end


print( "\tLoading textures..." );
textureManager:loadCheckerTexture( "checker", -1 );
textureManager:loadTestTexture( "test", -1 );
textureManager:loadTextureFromImageFile( "cat", "sample.png" );
textureManager:loadTextureFromImageFile( "skinColor", "skin_tile.jpg" );
textureManager:loadTextureFromImageFile( "sparkColor", "sparkCircularGradient.png" );
textureManager:logTextures();


print( "\tLoading shaders..." );
shaderManager:loadShaderFromFiles( "densityShader",
                                    "base3DVertexShader.glsl",
                                    "density3DFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "rayCastVolumeShader",
                                    "rayCastVertexShader.glsl",
                                    "rayCastFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "colorShader",
                                    "baseVertexShader.glsl",
                                    "colorFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "constantColorShader",
                                    "baseVertexShader.glsl",
                                    "constantColorFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "phongShader",
                                    "baseVertexShader.glsl",
                                    "phongFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "texturedOverlayShader",
                                    "baseVertexShader.glsl",
                                    "texturedOverlayFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "texturedSparkShader", 
                                    "baseVertexShader.glsl",
                                    "texturedSparkFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "blurHortShader",
                                    "blurHortVertexShader.glsl",
                                    "blurFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "blurVertShader",
                                    "blurVertVertexShader.glsl",
                                    "blurFragmentShader.glsl" );

print( "\tSetting up render passes..." );

frameBufferTarget = spark:getFrameBufferRenderTarget()
camera = spark:getCamera()

-- Create the final pass, render texture MainRenderTargetTexture using
-- the standard texturedOverlayShader.  Provides supersampling
-- based on the size of the source texture (MainRenderTargetTexture)
--
-- MSAAFinalRenderPass: MainRenderTargetTexture -> frameBufferTarget
--
spark:createPostProcessingRenderPass( 0.0, 
      "MSAAFinalRenderPass",
      "MainRenderTargetTexture", frameBufferTarget, 
      "texturedOverlayShader" )
--u_color

mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
spark:setMainRenderTarget( mainRenderTarget )
blackColor = vec4( 0,0,0,0 )
mainRenderTarget:setClearColor( blackColor )

opaqueRenderPass = spark:createRenderPassWithProjection( 1.0, "OpaquePass", camera, mainRenderTarget )
opaqueRenderPass:setDepthWrite( true )
opaqueRenderPass:setDepthTest( true )
opaqueRenderPass:disableBlending()

transparencyRenderPass = spark:createRenderPassWithProjection( 0.9, "TransparencyPass", camera, mainRenderTarget )
transparencyRenderPass:setDepthTest( true )
transparencyRenderPass:setDepthWrite( false )
transparencyRenderPass:useInterpolatedBlending()

glowPass = setupGlowPass( "GlowRenderPass" )



print( "end defaultScene.lua" );
