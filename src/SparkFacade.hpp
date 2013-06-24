#ifndef SPARK_SPARK_FACADE_HPP
#define SPARK_SPARK_FACADE_HPP

#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "FileAssetFinder.hpp"
#include "Scene.hpp"

namespace spark
{
    /// Wrapper for the Spark Rendering library
    /// Provides limited-dependency convience functions.
    class SparkFacade
    {
    public:
        SparkFacade( ScenePtr scene,
                      FileAssetFinderPtr finder,
                      TextureManagerPtr tm,
                      ShaderManagerPtr sm,
                      PerspectiveProjectionPtr camera,
                      FrameBufferRenderTargetPtr frameBufferTarget
                    )
        : m_scene( scene ),
          m_finder( finder ),
          m_textureManager( tm ),
          m_shaderManager( sm ),
          m_overlayPerspective( new OrthogonalProjection ),
          m_cameraPerspective( camera ),
          m_frameBufferTarget( frameBufferTarget )
        {
            
        }

//        RenderPassPtr createRenderPass( const RenderPassName& name,
//                              boost::shared_ptr< Projection > proj,
//                              )
//        void addPostPass( float aPriority,
//                          const RenderPassName& name
//                         )
//        {
//            
//        }
//
//        boost::shared_ptr< Material > createMaterial( )
//        {
//            
//        }
//        
//        void assignMaterial( boost::shared_ptr< Material > aMaterial,
//                            boost::shared_ptr< Renderable > aRenderable )
//        {
//            
//        }
//        
//        boost::shared_ptr< Mesh > loadMeshFile( const char* filename )
//        {
//            
//        }
        
    private:
        ScenePtr m_scene;
        FileAssetFinderPtr m_finder;
        TextureManagerPtr m_textureManager;
        ShaderManagerPtr m_shaderManager;
        // Common projections
        OrthogonalProjectionPtr m_overlayPerspective;
        PerspectiveProjectionPtr m_cameraPerspective;
        // FBOs
        FrameBufferRenderTargetPtr m_frameBufferTarget;
    };
    typedef spark::shared_ptr< SparkFacade > SparkFacadePtr;
 }

 #endif
