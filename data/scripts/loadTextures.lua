-- Load Common Textures

textureManager:loadCheckerTexture( "checker", -1 );
textureManager:loadTestTexture( "test", -1 );
textureManager:loadTextureFromImageFile( "cat", "sample.png" );
-- textureManager:loadTextureFromImageFile( "skinColor", "skin_tile.png" );
textureManager:loadTextureFromImageFile( "sparkColor", "sparkCircularGradient.png" );
textureManager:loadTextureFromImageFile( "hook_cautery", "hook_cautery_noise.png" )
textureManager:loadTextureFromImageFile( "noise_normalMap", "noise_normalMap.jpg" )
textureManager:loadTextureFromImageFile( "cloth", "cloth.jpg" )

textureManager:loadTextureFromImageFile( "tissueDiffuse", "liver1024.bmp" )
textureManager:loadTextureFromImageFile( "tissueBump", "liver1024_bump.bmp" )
textureManager:loadTextureFromImageFile( "tissueNormal", "ground-normal.jpg" )
textureManager:loadTextureFromImageFile( "tissueCharNormal", "ground-normal.jpg" )

textureManager:loadTextureFromImageFile( "bgCloth", "seamlesstexture18_1200.jpg" )

textureManager:logTextures();
