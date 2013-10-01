-- Load Common Shaders

shaderManager:loadShaderFromFiles( "colorShader",
                                    "base.vert",
                                    "color.frag" );

shaderManager:loadShaderFromFiles( "constantColorShader",
                                    "base.vert",
                                    "constantColor.frag" );

shaderManager:loadShaderFromFiles( "shadowCasterShader",
                                    "shadowCaster.vert",
                                    "shadowCaster.frag" );

shaderManager:loadShaderFromFiles( "phongShader",
                                    "base.vert",
                                    "phong.frag" );

shaderManager:loadShaderFromFiles( "texturedOverlayShader",
                                    "base.vert",
                                    "texturedOverlay.frag" );

shaderManager:loadShaderFromFiles( "simpleShadowExampleShader",
                                    "simpleShadowExample.vert",
                                    "simpleShadowExample.frag" );

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

-- Tissue Shaders
shaderManager:loadShaderFromFiles( "tissueShader",
                                    "tissue.vert",
                                    "tissue_bump.frag" );

shaderManager:loadShaderFromFiles( "tissueShader_heightMap",
                                    "heightMap.vert",
                                    "tissue_bump.frag" );

shaderManager:loadShaderFromFiles( "tissueShader_singleSample",
                                    "tissue_singleSample.vert",
                                    "tissue_singleSample.frag" );

shaderManager:loadShaderFromFiles( "tissueShader_debug",
                                    "tissue_debug.vert",
                                    "tissue_debug.frag" );
-- Deprecated - procedural
shaderManager:loadShaderFromFiles( "tissueShader_procedural",
                                    "tissue.vert",
                                    "tissue.frag" );

