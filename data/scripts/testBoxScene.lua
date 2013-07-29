
local utilityPasses = require "utilityPasses"

print( "--Begin testBoxScene.lua" );

print( "\tAvailable textures:" );
textureManager:logTextures();

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

mainRenderTarget = spark:createTextureRenderTarget( "MainRenderTargetTexture" )
spark:setMainRenderTarget( mainRenderTarget )
mainRenderTarget:setClearColor( vec4( 0,0,0,0 ) )

opaqueRenderPass = spark:createRenderPass( 1.0, "OpaquePass", mainRenderTarget )
opaqueRenderPass:setDepthWrite( true )
opaqueRenderPass:setDepthTest( true )
opaqueRenderPass:disableBlending()

HUDRenderPass = spark:createOverlayRenderPass( 0.25, "HUDPass", mainRenderTarget )
HUDRenderPass:setDepthTest( false )
HUDRenderPass:setDepthWrite( false )
HUDRenderPass:useInterpolatedBlending()
--HUDRenderPass:disableBlending()



-- Make floor box
-- floorMat = spark:createMaterial( "phongShader" )
-- floorMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
-- floorMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
-- floorMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
-- floorMat:setVec4( "u_ka", vec4(1,1,1,1) )
-- floorMat:setVec4( "u_kd", vec4(1,1,1,1) )
-- floorMat:setVec4( "u_ks", vec4(1,1,1,1) )
-- floorMat:setFloat( "u_ns", 100.0 )
-- floorMat:addTexture( "s_color", "skinColor" )
-- skin = spark:createCube( vec3(-2.5, 0, -2.5), vec3(5, 0.1, 5), floorMat, "OpaquePass" )
-- skin:rotate( 90, vec3(1,0,0) )

boxMat = spark:createMaterial( "colorShader" )
boxMat:setVec4( "u_color", vec4(1.0,0.3,0.3,1.0) )
boxA = spark:createCube( vec3(0,0,0), vec3(0.25,0.25,0.25), boxMat, "OpaquePass" )
boxB = spark:createCube( vec3(-0.5,0,0), vec3(0.25,0.25,0.25), boxMat, "OpaquePass" )
boxB:rotate( 45, vec3(0,1,0) )


camera = spark:getCamera()
camera:cameraPos( 0.2, 1.2, -0.9 )
camera:cameraTarget( 0.06, 0.1, 0.0 )
camera:fov( 48 )

print( "end defaultScene.lua" );
