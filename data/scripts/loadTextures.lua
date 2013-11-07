-- Load Common Textures

textureManager:loadCheckerTexture( "checker", -1 );
textureManager:loadTestTexture( "test", -1 );
textureManager:loadTextureFromImageFile( "cat", "sample.png" );
-- textureManager:loadTextureFromImageFile( "skinColor", "skin_tile.png" );
textureManager:loadTextureFromImageFile( "sparkColor", "sparkCircularGradient.png" );
textureManager:loadTextureFromImageFile( "hook_cautery", "hook_cautery_noise.png" )
textureManager:loadTextureFromImageFile( "cloth", "cloth.jpg" )

-- textureManager:loadTextureFromImageFile( "tissueDiffuse", "Leather0013_1_L.jpg" )
-- textureManager:loadTextureFromImageFile( "tissueNormal", "liver1024_bump.bmp" )
-- textureManager:loadTextureFromImageFile( "tissueBump", "liver1024_bump.bmp" )
-- textureManager:loadTextureFromImageFile( "tissueSpecular", "liverTexture_specularStrength.png" )
-- textureManager:loadTextureFromImageFile( "tissueAmbient", "liverTexture_ambientOcclusion.png" )

-- textureManager:loadTextureFromImageFile( "tissueDiffuse", "liverTexture_diffuse.png" )
-- textureManager:loadTextureFromImageFile( "tissueNormal", "liverTexture_normal.png" )
-- textureManager:loadTextureFromImageFile( "tissueBump", "liverTexture_bump.png" )
-- textureManager:loadTextureFromImageFile( "tissueSpecular", "liverTexture_specularStrength.png" )
-- textureManager:loadTextureFromImageFile( "tissueAmbient", "liverTexture_ambientOcclusion.png" )

-- textureManager:loadTextureFromImageFile( "tissueDiffuse", "liver.bmp" )
-- textureManager:loadTextureFromImageFile( "tissueNormal", "liver1024_bump.bmp" )
-- textureManager:loadTextureFromImageFile( "tissueBump", "liverTexture_bump.png" )
-- textureManager:loadTextureFromImageFile( "tissueSpecular", "liverTexture_specularStrength.png" )
-- textureManager:loadTextureFromImageFile( "tissueAmbient", "liverTexture_ambientOcclusion.png" )

-- textureManager:loadTextureFromImageFile( "tissueDiffuse", "liver.bmp" )
-- textureManager:loadTextureFromImageFile( "tissueNormal", "skin_normal.jpg" )
-- textureManager:loadTextureFromImageFile( "tissueBump", "liver1024_bump.bmp" )
-- textureManager:loadTextureFromImageFile( "tissueSpecular", "liverTexture_specularStrength.png" )
-- textureManager:loadTextureFromImageFile( "tissueAmbient", "liverTexture_ambientOcclusion.png" )

textureManager:loadTextureFromImageFile( "tissueDiffuse", "liver1024.bmp" )
textureManager:loadTextureFromImageFile( "tissueBump", "liver1024_bump.bmp" )
textureManager:loadTextureFromImageFile( "tissueCharNormal", "ground-normal.jpg" )
-- textureManager:loadTextureFromImageFile( "tissueNormal", "liver1024_bump.jpg" )

-- textureManager:loadTextureFromImageFile( "tissueSpecular", "liverTexture_specularStrength.png" )
-- textureManager:loadTextureFromImageFile( "tissueAmbient", "liverTexture_ambientOcclusion.png" )


textureManager:loadTextureFromImageFile( "bgCloth", "seamlesstexture18_1200.jpg" )

textureManager:logTextures();
