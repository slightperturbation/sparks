-- Load Common Shaders

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

shaderManager:loadShaderFromFiles( "TextShader",
                                    "textVertexShader.glsl",
                                    "distanceFieldFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "texturedSparkShader", 
                                    "baseVertexShader.glsl",
                                    "texturedSparkFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "densityShader",
                                    "base3DVertexShader.glsl",
                                    "density3DFragmentShader.glsl" );

shaderManager:loadShaderFromFiles( "rayCastVolumeShader",
                                    "rayCastVertexShader.glsl",
                                    "rayCastFragmentShader.glsl" );

