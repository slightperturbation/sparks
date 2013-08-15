-- Create common RenderPasses


-- Create the final pass, render texture MainRenderTargetTexture using
-- the standard texturedOverlayShader.  Provides supersampling
-- based on the size of the source texture (MainRenderTargetTexture)
--
-- MSAAFinalRenderPass: MainRenderTargetTexture -> frameBufferTarget
--
spark:createPostProcessingRenderPass( 0.0, 
      "MSAAFinalRenderPass",
      "MainRenderTargetTexture", spark:getFrameBufferRenderTarget(), 
      "texturedOverlayShader" )

mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
spark:setMainRenderTarget( mainRenderTarget )
mainRenderTarget:setClearColor( vec4( 0,1,0,0.5 ) )

opaqueRenderPass = spark:createRenderPass( 1.0, "OpaquePass", mainRenderTarget )
opaqueRenderPass:setDepthWrite( true )
opaqueRenderPass:setDepthTest( true )
opaqueRenderPass:disableBlending()

HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", mainRenderTarget )
HUDRenderPass:setDepthTest( false )
HUDRenderPass:setDepthWrite( false )
HUDRenderPass:useInterpolatedBlending()