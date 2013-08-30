
# Spark VR Rendering & Interaction Library #

Features:

* Ordered rendering of primitives.  Ordering supports minimizing state changes for performance.
* Multi-pass rendering (see spark::RenderPass)
* RenderTarget support to enable shadows and deferred rendering
* Extensibility using Lua

## Dependencies -- Windows ##

### Required dependencies settings for Windows ###

* Boost -- set environment variable "BOOST_ROOT" to the boost directory holding "boost/any.hpp"
* Eigen3 -- Note! May need to set directory in CmakeLists.txt
* Intel Thread Building Blocks (TBB) -- automatically downloads from threadbuildingblocks.org
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

Only out-of-source builds are supported.

> cd spark
> mkdir build-xcode
> cd build-xcode
> cmake .. -G Xcode
> open sparks.xcodeproj

## Running ##

Options:

* "-trace" enable detailed trace logging to ./sparks.log
* "-debug" enable "Legacy" (aka, non-core profile supported) error catching to report opengl errors.

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
    
     function ExampleState:fixedUpdate( dt )
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

