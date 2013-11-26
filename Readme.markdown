
# Spark VR Rendering & Interaction Library {#mainpage}

Features:

* Ordered rendering of primitives.  Ordering supports minimizing state changes for performance.
* Multi-pass rendering (see spark::RenderPass)
* RenderTarget support to enable shadows and deferred rendering
* Extensibility using Lua

## Dependencies -- Windows ##

### Required dependencies settings for Windows ###

* Boost -- set environment variable "BOOST_ROOT" to the boost directory holding "boost/any.hpp"
* Eigen3 -- Note! May need to set directory in CmakeLists.txt
* Intel Thread Building Blocks (TBB) -- cmake build automatically downloads from threadbuildingblocks.org
* FreeType-GL -- automatically downloads from googlecode.com.  Caution!  Until defect 62 is fixed, may need to fix texture-atlas.c after download.
* FreeType -- source included
* Lua -- source included
* Luabind -- source included
* GLFW -- automatically downloads version 3.01
* GLEW -- Required GLEW_ROOT_DIR environment variable
* GLM  -- Required GLM_ROOT_DIR environment variable
* DevIL -- Required
* AssImp -- Optional 

### Optional dependencies ###
* To use zSpace devices, the VS2010 zSpace bin dir must be in the PATH.  (e.g., C:\zSpace\zSpaceSDK\2.10.0.19412\Win64\VS2010\Bin).  The zSpace SDK installs can be downloaded: http://zspace.com/download-the-zspace-sdk-current/
* Note: zSpace SDK has problems setting the path, this may need to be done manually.
* Download the Hydra controller SDK is at: http://sixense.com/developers

## Dependencies -- MacOSX ##

HomeBrew is recommended.  Required packages:

* boost
* eigen
* glew
* devil
* assimp
* glm 

## Building - Windows, Visual Studio 2010, 64-bit ##

Only out-of-source builds are supported.

> cd spark
> mkdir build-vs2010
> cd build-vs2010
> cmake .. -G "Visual Studio 10 Win64"
> start sparks.sln

## Building - MacOSX ##

Note:  MacOSX is only supported for testing, neither primary interface (zSpace nor TrakStar) is supported on OSX. Only out-of-source builds are supported.

> cd spark
> mkdir build-xcode
> cd build-xcode
> cmake .. -G Xcode
> open sparks.xcodeproj

## Running ##

The VEST simulator consists of two programs on the simulator computer. The main program is sparkGui.exe and the supporting program is a CGI program called ESUServerCGI.cgi.

### sparkGui ###

The sparkGui.exe runs with options:

* "-trace" enable detailed trace logging to ./sparks.log
* "-debug" enable "Legacy" (aka, non-core profile supported) error catching to report opengl errors.

The sparks/src/main.cpp file contains the main() entry point for sparkGui.

### ESUServerCGI.cgi

The ESUServerCGI.cgi is called by a webserver to provide interaction with the touch interface for the ESU on the iPad Mini.  The CGI script can be run by any webserver that can server static files from sparks/www and CGI scripts from sparks/www/cgi.

The Mongoose webserver can be used to server both static and CGI.  A trivial script for starting the Mongoose server is located in sparks/bin-win32/startWebserver.bat.  This script uses the spark/bin-win32/mongoose.conf configuration file to find the static and CGI directories.

The CGI script communicates with the main simulator program (sparkGui) through a shared memory section "SPARK_SharedMemory" and the object "CurrentESUSettings".  These constants are defined in spark/src/ESUInputFromSharedMemory.cpp.

The main() entry point for the CGI program is in spark/src/ESUServerCGI.cpp.

# Hardware Setup & Requirements #


## iPad Mini ##

The iPad Mini acts as a touch interface for the ESU.  Once the iPad is connected to the NETGEAR52 wifi, and once the local webserver is running on the simulator computer, the iPad should be able to connect and get the ESU interface webpage.  The simulator computer is assigned an IP dynamically, so you may need to use ipconfig to find its current address in the 192.168.1.x subnet.

## Wifi Router ##

The wireless router provides connectivity between the iPad mini and the simulation computer.  Using a separate router allows the simulator to be independent of the local network, which is critical because the simulation sets up an HTTP server on port 8080 and corporate networks often block those packets.

The wireless router is setup with name "NETGEAR52" and has the password "sillyoctopus52".  The created network is the 0.0.0.255 subnet from 192.168.1.1.  The router can be configured through HTTP on 192.168.1.1:80 with username admin.

The simulation computer must be connected to one of the Ethernet ports on the router.  An external/Internet connection is not needed.  

## Foot Pedal ##

The Kinesis foot pedal uses the factory-default setting, so it sends right and left mouse clicks.  No setup is required.

## zSpace ##

Drivers and example programs are available from http://zspace.com

## AscensionTech TrakStar ##

Drivers and documentation are available from ftp://ftp.ascension-tech.com/

The sensor embedded in the instrument should be plugged into the "1" input.

## Numeric Keypad ##

The keypad is used to select from options on the screen.  It requires two disposable AAA batteries.  The tiny USB dongle must be plugged into the simulation computer.

## Speakers ##

Standard speakers are needed for the ESU sounds.


# Development #
## Adding script states ##

States can be created using Lua scripts, C++ class or a mix of both. 

### Lua State Scripts ###

* Add new file to data/scripts/states/XXXState.lua (Note that filename ending in State.lua is required by loader)
* A script must include methods: new, load, activate, update, fixedUpdate, deactivate, nextState
* A global "theNextState" is required and tested after the call to nextState.  If a non-empty string, the indicated state will be switched to.

Example Lua state script:

    :::lua
     local ExampleState = {}
    
     function ExampleState:new()
    	  newObj = { angle = 45 }
    	  self.__index = self
    	  return setmetatable(newObj, self)
     end
    
     function ExampleState:load()
     end
    
     function ExampleState:activate()
     end
    
     function ExampleState:update( dt )
     end
    
     function ExampleState:deactivate()
     end
    
     function ExampleState:nextState( currTime )
       	if currTime > 20 then
           theNextState = "nextStateName"
         else
           theNextState = "" -- Keep current state
         end
     end
    
     theState = ExampleState:new()
     theNextState = ""


### C++ States ###

States can be implemented in C++ by subclassing State.  If desired, ScriptState can be subclassed to use or augment a Lua script state.  Well suited for States that do non-trival computation in update methods.  See include/states/SimulationState.hpp for an example.

# Troubleshooting / Common Problems / Known Bugs

* On Windows, building any config other than "Debug" and "Release" depends on libs built in "Debug" or "Release".  E.g., To build "RelWithDebInfo", first build "Release", which builds libs (GLFW3) for release, then try building "RelWithDebInfo".
