
local utilityPasses = require "utilityPasses"

print( "Begin testBoxScene.lua" );

frameBufferTarget = spark:getFrameBufferRenderTarget()
camera = spark:getCamera()

print( "\tSetting up render passes..." );

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

mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
spark:setMainRenderTarget( mainRenderTarget )
mainRenderTarget:setClearColor( vec4( 0.0,0.0,0.0,0 ) )

opaqueRenderPass = spark:createRenderPassWithProjection( 1.0, "OpaquePass", camera, mainRenderTarget )
opaqueRenderPass:setDepthWrite( true )
opaqueRenderPass:setDepthTest( true )
opaqueRenderPass:disableBlending()

glowPass = utilityPasses.setupGlowPass( "GlowRenderPass" )


boxMat = spark:createMaterial( "constantColorShader" )
boxMat:setVec4( "u_color", vec4(1.0,0.5,0.01,1) )
boxA = spark:createCube( vec3(0.0,0,0), vec3(0.25,0.25,0.25), boxMat, "OpaquePass" )

boxB = spark:createCube( vec3(-0.25,0,0), vec3(0.25,0.25,0.25), boxMat, "GlowRenderPass" )

boxD = spark:createCube( vec3(-0.125,-.25,0), vec3(0.25,0.25,0.25), boxMat, "OpaquePass" )
boxC = spark:createCube( vec3(-0.125,-.25,0), vec3(0.25,0.25,0.25), boxMat, "GlowRenderPass" )


camera = spark:getCamera()
camera:cameraPos( 0, 0.125, -1 )
camera:cameraTarget( 0, 0.125, 0 )
--camera:cameraPos( 0.2, 1.2, -0.9 )
--camera:cameraTarget( 0.06, 0.1, 0.0 )
camera:fov( 35 )

print( "end defaultScene.lua" );
