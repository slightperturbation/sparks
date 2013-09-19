#ifndef SPARK_ILLUMINATIONMODEL_HPP
#define SPARK_ILLUMINATIONMODEL_HPP

#include "Spark.hpp"

#include <glm/glm.hpp>

namespace spark
{
    class IlluminationModel;

    /// Abstract superclass for Lights held by IlluminationModel.
    class Light
    {
    public:
        friend class IlluminationModel;
        virtual ~Light() {}
        virtual std::string name( void ) const = 0;
    private:
        virtual void setShaderUniforms( MaterialPtr material,
            ConstRenderablePtr renderable ) const = 0;
    };
    typedef spark::shared_ptr< Light > LightPtr;
    typedef spark::shared_ptr< const Light > ConstLightPtr;

    /// Simple light that emits a uniform color in all directions.
    class AmbientLight : public Light
    {
    public:
        friend class IlluminationModel;
        virtual ~AmbientLight();

        virtual std::string name( void ) const;
    private:
        virtual void setShaderUniforms( MaterialPtr material, 
            ConstRenderablePtr renderable ) const override;
        AmbientLight( const glm::vec4& color );
    private:
        glm::vec4 m_color;
    };
    typedef spark::shared_ptr< AmbientLight > AmbientLightPtr;

    /// Light that can be used to cast shadows, sets .projViewModelMat
    /// and .color uniforms for shadow shaders.
    /// Shader uniforms given as a ShadowLight struct:
    /// 
    /// GLSL Vertex Shader example:
    /// 
    /// struct ShadowLight 
    /// {
    ///     mat4 projViewModelMat;
    ///     vec4 color;
    /// }
    /// uniform ShadowLight u_shadowLight[4];
    /// uniform int u_currLightIndex = 0;
    /// 
    class ShadowLight : public Light
    {
    public:
        friend class IlluminationModel;
        virtual ~ShadowLight();
        virtual std::string name( void ) const ;
        /// Returns the Projection that casts the light for shadow mapping.
        ProjectionPtr getProjection( void ) { return m_projection; }
        /// Set the Projection used to cast the light, particularly for
        /// shadow mapping.
        void setProjection( ProjectionPtr projection )
        { m_projection = projection; }
    private:
        virtual void setShaderUniforms( MaterialPtr material,
            ConstRenderablePtr renderable ) const override;
        ShadowLight( unsigned int index,
                          const glm::vec4& color,
                          ProjectionPtr projection );
    private:
        unsigned int m_index;
        glm::vec4 m_color;
        ProjectionPtr m_projection;
    };
    typedef spark::shared_ptr< ShadowLight > ShadowLightPtr;

    std::ostream& operator<<( std::ostream& out, const IlluminationModel& ill );
    /// Illumination model; Holds all of the lights for a scene.
    class IlluminationModel
    {
    public:
        void addAmbientLight( glm::vec4 color );
        void addShadowLight( glm::vec4 color, ProjectionPtr projection );
        void setShaderUniforms( MaterialPtr material, 
                                ConstRenderablePtr renderable ) const;
        friend std::ostream& operator<<( std::ostream& out, const IlluminationModel& ill );
    private:
        std::vector< LightPtr > m_lights;
    };
    typedef spark::shared_ptr< IlluminationModel > IlluminationModelPtr;
    typedef spark::shared_ptr< const IlluminationModel > ConstIlluminationModelPtr;
    ///////////
}

#endif
