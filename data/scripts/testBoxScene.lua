-- Change the standard error function so that it shows the debug traceback.

print( "Begin testBoxScene.lua" );

print( "\tLoading textures..." );
textureManager:loadCheckerTexture( "checker", -1 );
textureManager:loadTestTexture( "test", -1 );
textureManager:loadTextureFromImageFile( "cat", "sample.png" );
textureManager:loadTextureFromImageFile( "skinColor", "skin_tile.png" );
textureManager:loadTextureFromImageFile( "sparkColor", "sparkCircularGradient.png" );
textureManager:loadTextureFromImageFile( "hook_cautery", "hook_cautery_noise.png" )
textureManager:logTextures();


print( "\tLoading shaders..." );
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
mainRenderTarget:setClearColor( vec4( 0,0,0,0 ) )

opaqueRenderPass = spark:createRenderPassWithProjection( 1.0, "OpaquePass", camera, mainRenderTarget )
opaqueRenderPass:setDepthWrite( true )
opaqueRenderPass:setDepthTest( true )
opaqueRenderPass:disableBlending()

transparencyRenderPass = spark:createRenderPassWithProjection( 0.5, "TransparencyPass", camera, mainRenderTarget )
transparencyRenderPass:setDepthTest( true )
transparencyRenderPass:setDepthWrite( false )
--transparencyRenderPass:useAdditiveBlending()

-- Make floor box
floorMat = spark:createMaterial( "phongShader" )
floorMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
floorMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
floorMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
floorMat:setVec4( "u_ka", vec4(1,1,1,1) )
floorMat:setVec4( "u_kd", vec4(1,1,1,1) )
floorMat:setVec4( "u_ks", vec4(1,1,1,1) )
floorMat:setFloat( "u_ns", 100.0 )
floorMat:addTexture( "s_color", "skinColor" )
skin = spark:createCube( vec3(-2.5, 0, -2.5), vec3(5, 0.1, 5), floorMat, "OpaquePass" )
skin:rotate( 90, vec3(1,0,0) )

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
