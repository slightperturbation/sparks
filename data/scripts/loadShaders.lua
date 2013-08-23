-- Load Common Shaders

shaderManager:loadShaderFromFiles( "colorShader",
                                    "base.vert",
                                    "color.frag" );

shaderManager:loadShaderFromFiles( "constantColorShader",
                                    "base.vert",
                                    "constantColor.frag" );

shaderManager:loadShaderFromFiles( "phongShader",
                                    "base.vert",
                                    "phong.frag" );

shaderManager:loadShaderFromFiles( "texturedOverlayShader",
                                    "base.vert",
                                    "texturedOverlay.frag" );

shaderManager:loadShaderFromFiles( "blurHortShader",
                                    "blurHort.vert",
                                    "blur.frag" );

shaderManager:loadShaderFromFiles( "blurVertShader",
                                    "blurVert.vert",
                                    "blur.frag" );

shaderManager:loadShaderFromFiles( "TextShader",
                                    "text.vert",
                                    "distanceField.frag" );

shaderManager:loadShaderFromFiles( "texturedSparkShader", 
                                    "base.vert",
                                    "texturedSpark.frag" );

shaderManager:loadShaderFromFiles( "densityShader",
                                    "base3D.vert",
                                    "density3D.frag" );

shaderManager:loadShaderFromFiles( "rayCastVolumeShader",
                                    "rayCast.vert",
                                    "rayCast.frag" );

shaderManager:loadShaderFromFiles( "tissueShader",
                                    "tissue.vert",
                                    "tissue.frag" );

