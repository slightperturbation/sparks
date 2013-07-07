
local utilityPasses = require "utilityPasses"
-- Change the standard error function so that it shows the debug traceback.

print( "Begin defaultScene.lua" );




print( "\tLoading textures..." );
textureManager:loadCheckerTexture( "checker", -1 );
textureManager:loadTestTexture( "test", -1 );
textureManager:loadTextureFromImageFile( "cat", "sample.png" );
textureManager:loadTextureFromImageFile( "skinColor", "skin_tile.png" );
textureManager:loadTextureFromImageFile( "sparkColor", "sparkCircularGradient.png" );
textureManager:loadTextureFromImageFile( "hook_cautery", "hook_cautery_noise.png" )
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
mainRenderTarget:setClearColor( vec4( 0,0,0,0 ) )

opaqueRenderPass = spark:createRenderPassWithProjection( 1.0, "OpaquePass", camera, mainRenderTarget )
opaqueRenderPass:setDepthWrite( true )
opaqueRenderPass:setDepthTest( true )
opaqueRenderPass:disableBlending()

transparencyRenderPass = spark:createRenderPassWithProjection( 0.5, "TransparencyPass", camera, mainRenderTarget )
transparencyRenderPass:setDepthTest( true )
transparencyRenderPass:setDepthWrite( false )
--transparencyRenderPass:useAdditiveBlending()

-- Make skin box
skinMat = spark:createMaterial( "phongShader" )
skinMat:setVec4( "u_light.position_camera", vec4(5,10,0,1) )
skinMat:setVec4( "u_light.diffuse", vec4(0.8,0.8,0.8,1) )
skinMat:setVec4( "u_ambientLight", vec4(0.3,0.1,0.1,1) )
skinMat:setVec4( "u_ka", vec4(1,1,1,1) )
skinMat:setVec4( "u_kd", vec4(1,1,1,1) )
skinMat:setVec4( "u_ks", vec4(1,1,1,1) )
skinMat:setFloat( "u_ns", 100.0 )
skinMat:addTexture( "s_color", "skinColor" )
skin = spark:createCube( vec3(-2.5, .25, -2.5), vec3(5, 0.5, 5), skinMat, "OpaquePass" )
skin:rotate( 90, vec3(1,0,0) )


-- Load hook
metalMat = spark:createMaterial( "phongShader" )
metalMat:addTexture( "s_color", "hook_cautery" )
metalMat:setVec4( "u_light.position_camera", vec4(0,0,0,1) )
metalMat:setVec4( "u_light.diffuse", vec4(0.3,0.3,0.3,1) )
metalMat:setVec4( "u_ambientLight", vec4(0.4,0.2,0.2,1) )
metalMat:setVec4( "u_ka", vec4(0.4,0.4,0.4,1) )
metalMat:setVec4( "u_kd", vec4(1,1,1,1) )
metalMat:setVec4( "u_ks", vec4(1,1,1,1) )
metalMat:setFloat( "u_ns", 1000.0 )

hook = spark:loadMesh( "hook_cautery_new.3DS", metalMat, "OpaquePass" )
hook:translate( 0,.36,0 )
hook:rotate( 120,  vec3(0,0,1) )
hook:rotate( 30,  vec3(0,1,0) )
hook:scale( 0.02 )

----------------
-- Spark
glowPass = utilityPasses.setupGlowPass( "GlowRenderPass" )

drawSparks = true
if( drawSparks ) then
    sparkMat = spark:createMaterial( "texturedSparkShader" )
    sparkMat:addTexture( "s_color", "sparkColor" )

    sparkConstantMat = spark:createMaterial( "constantColorShader" )
    sparkConstantMat:setVec4( "u_color", vec4(1,0.9,0.9,1) )

    print( "\tCreating sparks..." )
    for xpos = 0.05, -0.05, -0.05 do
          for zpos = -0.05, 0.05, 0.05 do
                print( "Creating spark to " .. xpos .. ", " .. zpos)
                lSpark = spark:createLSpark( 
                    vec3( xpos*0.1,    0.25+.08,  -0.02+zpos*0.1), 
                    --vec3( 0.1+xpos*2,   0.25 ,      -0.1+zpos*2    ), 
                    vec3( xpos,   0.25 , zpos    ), 
                      1.9, -- intensty
                      0.1,--0.2, -- scale/spread
                      7,   -- recursive depth
                      0.5,--0.9, -- fork prob
                      "GlowRenderPass", 
                      --"OpaquePass", 
                      --"TransparencyPass",
                      sparkConstantMat
                      --sparkMat
                )
                --lSpark:setMaterialForPassName( "TransparencyPass", sparkMat )
                lSpark:setRequireExplicitMaterial( true )
          end
    end
end

for boxX = -1, 1, 0.33 do
  boxZ = -.33
    box = spark:createCube( vec3(boxX,0.25,boxZ), vec3(0.15,0.15,0.15), metalMat, "OpaquePass" )
    box:rotate( 45, vec3(0,1,0) )
end

camera = spark:getCamera()
camera:cameraPos( 0.2, 1.2, -0.9 )
camera:cameraTarget( 0.06, 0.1, 0.0 )
camera:fov( 48 )

print( "end defaultScene.lua" );
