
print( "Begin defaultScene.lua" );
textureManager:loadCheckerTexture( "checker", -1 );
textureManager:loadTestTexture( "test", -1 );
textureManager:loadTextureFromImageFile( "cat", "sample.png" );
textureManager:loadTextureFromImageFile( "skinColor", "skin_tile.jpg" );
textureManager:loadTextureFromImageFile( "sparkColor", "sparkCircularGradient.png" );
textureManager:logTextures();
print( "\tTextures loaded." );

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
print( "\tShaders loaded." );


--mainRenderTarget =
--camera = 
--opaquePass = scene:createRenderPass( 1.0, "OpaquePass", mainRenderTarget, camera );
--opaquePass.depthWrite = true;
--opaquePass:disableBlending()

print( "end defaultScene.lua" );
