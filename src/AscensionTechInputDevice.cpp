
#include "AscensionTechInputDevice.hpp"


spark::AscensionTechInputDevice
::AscensionTechInputDevice( void )
: m_read( 0 ), m_write( 1 )
{
    // Initialize the ATC3DG driver and DLL
    LOG_DEBUG(g_log) << "Init ATC3DG (AscensionTech) Tracker";

    {
        int errorCode = InitializeBIRDSystem();
        handleErrorCode( errorCode );
    }
    // Setup System
    {
        int errorCode = GetBIRDSystemConfiguration( &m_systemConfig );
        handleErrorCode( errorCode );
    }
    {
        int isMetric = 1;
        int errorCode = SetSystemParameter(
            METRIC, // report in millimeters
            &isMetric, // address of data source buffer
            sizeof(isMetric) // size of source buffer
            );
        handleErrorCode( errorCode );
    }
    // Setup sensors
    {
        // Get the sensor configuration for each sensor
        m_sensorConfigs.resize( m_systemConfig.numberSensors );
        for( int i = 0; i < m_systemConfig.numberSensors; ++i )
        {
            int errorCode = GetSensorConfiguration( i, &(m_sensorConfigs[i]) );
            handleErrorCode( errorCode );
        }
    }
    // Setup Transmitter
    {
        // Get the transmitter configuration
        m_transmitterConfig.resize( m_systemConfig.numberTransmitters );
        for( int i = 0; i < m_systemConfig.numberTransmitters; ++i )
        {
            int errorCode = GetTransmitterConfiguration( i, &(m_transmitterConfig[i]) );
            handleErrorCode( errorCode );
        }
        // find first transmitter and turn it on
        for( short i = 0; i < m_systemConfig.numberTransmitters; ++i )
        {
            if( m_transmitterConfig[i].attached )
            {
                int errorCode = SetSystemParameter( SELECT_TRANSMITTER, &i, sizeof(i) );
                handleErrorCode( errorCode );
                break;
            }
        }
    }
    // Always use the "first" sensor for now
    const int sensorId = 0;
    {
        DATA_FORMAT_TYPE dataFormat = DOUBLE_POSITION_MATRIX;
        int errorCode = SetSensorParameter(
            sensorId, // index number of target sensor
            DATA_FORMAT, // command parameter type
            &dataFormat, // address of data source buffer
            sizeof(dataFormat) // size of source buffer
            );
        handleErrorCode( errorCode );
    }

    // set offset +x
    if( true )
    {
        DOUBLE_POSITION_RECORD positionOffset;
        positionOffset.x = 0; positionOffset.y = 0; positionOffset.z = 0;
        // NOTE must supply values in inches, even though using metric for measurements!
        const double mm_to_inches_conversion_factor = 0.0393701;
        // For "laparoscopic" tool
        positionOffset.x = 396.31 * mm_to_inches_conversion_factor;
        // for "open" tool
        //positionOffset.x = 88.65 * mm_to_inches_conversion_factor;
        
        int errorCode = SetSensorParameter(
            sensorId, // index number of target sensor
            SENSOR_OFFSET, // command parameter type
            &positionOffset, // address of data source buffer
            sizeof(positionOffset) // size of source buffer
            );
        handleErrorCode( errorCode );
    }
}

spark::AscensionTechInputDevice
::~AscensionTechInputDevice()
{

}

void
spark::AscensionTechInputDevice
::update( double dt )
{
    //glm::vec3 screenSpaceOffset( 0, 0.25, 0 ); // TODO
    glm::vec3 screenSpaceOffset( 0, -0.25, 0 ); // TODO

    static DOUBLE_POSITION_MATRIX_RECORD data;
    int errorCode = GetAsynchronousRecord( 0, &data, sizeof( data ) ); 
    if( errorCode != BIRD_ERROR_SUCCESS )
    {
        handleErrorCode( errorCode );
        return;
    }
    double scaleFactor = 1.0/1000.0; // convert mm to meters
    glm::vec3 positionOffsetInWorldCoords( 0, 0.15, -0.4 );
    //glm::vec3 positionOffsetInWorldCoords( 0, 0, -0.4 );
    m_position[m_write].x = - data.y * scaleFactor;
    m_position[m_write].y = - data.z * scaleFactor;
    m_position[m_write].z =   data.x * scaleFactor;
    m_position[m_write] += positionOffsetInWorldCoords;
    m_position[m_write] += screenSpaceOffset;

    // Translate from TrakStar coordinates to OpenGL (y-up, +z toward camera)
    m_transform[m_write][0][0] =   data.s[1][1];
    m_transform[m_write][0][1] =   data.s[1][2];
    m_transform[m_write][0][2] = - data.s[1][0];
    m_transform[m_write][0][3] =   0;

    m_transform[m_write][1][0] =   data.s[2][1];
    m_transform[m_write][1][1] =   data.s[2][2];
    m_transform[m_write][1][2] = - data.s[2][0];
    m_transform[m_write][1][3] =   0;

    m_transform[m_write][2][0] = - data.s[0][1];
    m_transform[m_write][2][1] = - data.s[0][2];
    m_transform[m_write][2][2] =   data.s[0][0];
    m_transform[m_write][2][3] =   0;

    m_transform[m_write][3][0] = m_position[m_write].x; //- data.y * scaleFactor;
    m_transform[m_write][3][1] = m_position[m_write].y; //- data.z * scaleFactor;
    m_transform[m_write][3][2] = m_position[m_write].z; //  data.x * scaleFactor;
    m_transform[m_write][3][3] =   1.0;

    // orient to x+ for stylus direction

    //m_transform = glm::translate( m_transform, screenSpaceOffset );
    //m_transform = glm::translate( m_transform, positionOffsetInWorldCoords );

    //m_transform = glm::rotate( m_transform, -90.0f, glm::vec3(0,1,0) );
    //m_transform = glm::scale( m_transform, glm::vec3(0.002) );
    
    //m_transform = glm::rotate( m_transform, 180.0f, glm::vec3(0,1,0) );
 
    // TODO -- project to screen coordinates
    double screenScaleFactor = scaleFactor; // TODO ?
    m_screenPosition[m_write].x = data.x;
    m_screenPosition[m_write].y = data.y;

    // Done writing, swap
    {
        boost::lock_guard< boost::mutex > lock( m_mutex );
        size_t tmp = m_read; m_read = m_write; m_write = tmp;
    }
}

glm::vec3
spark::AscensionTechInputDevice
::getPosition( void ) const
{
    boost::lock_guard< boost::mutex > lock( m_mutex );
    return m_position[m_read];
}

glm::vec2
spark::AscensionTechInputDevice
::getScreenPosition( void ) const
{
    boost::lock_guard< boost::mutex > lock( m_mutex );
    return m_screenPosition[m_read];
}

glm::mat4
spark::AscensionTechInputDevice
::getTransform( void ) const
{
    boost::lock_guard< boost::mutex > lock( m_mutex );
    return m_transform[m_read];
}

void
spark::AscensionTechInputDevice
::handleErrorCode( int errorCode )
{
    if( errorCode == BIRD_ERROR_SUCCESS )
    {
        return;
    }
    char			buffer[1024];
    char			*pBuffer = &buffer[0];
    int				numberBytes;
    while( errorCode != BIRD_ERROR_SUCCESS )
    {
        errorCode = GetErrorText( errorCode, pBuffer, sizeof(buffer), SIMPLE_MESSAGE );
        numberBytes = strnlen( buffer, 1024 );
        buffer[numberBytes] = '\n';		// append a newline to buffer
        LOG_ERROR(g_log) << "AscensionTech: " << buffer;
        std::cerr << "AscensionTech: " << buffer << "\n";
        throw "AscensionTech failed to initialize";
    }
}


/////////////////////////////////////////////////////////////////////////

spark::AscensionTechInputFactory
::AscensionTechInputFactory( void )
{

}

spark::AscensionTechInputFactory
::~AscensionTechInputFactory() 
{

}

std::unique_ptr<spark::InputDevice> 
spark::AscensionTechInputFactory
::createDevice( int index ) const 
{
    std::unique_ptr<InputDevice> device( new AscensionTechInputDevice() );
    return std::move( device );
}


