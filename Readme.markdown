
# Spark Rendering & Interaction Library

Features:

* Ordered rendering of primitives.  Ordering supports minimizing state changes for performance.
* Multi-pass rendering (see spark::RenderPass)
* RenderTarget support to enable shadows and deferred rendering
* Extensibility using Lua

## Dependencies -- MacOSX ##

HomeBrew is recommended.  Required packages:

* boost
* eigen
* glew
* devil
* assimp
* glm 
* lua 

Oculus Rift support requires the 

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
* AssImp -- Required 

### Optional dependencies ###
* Download the Hydra controller SDK is at: http://sixense.com/developers

### Windows Environment Variables ###

Building on Windows requires the following environment variables to be set to the directory containing headers and binaries libaries:

* BOOST_ROOT -- for Boost 1.53+ (including libraries for date_time, unit_test_framework, filesystem, system, thread and chrono)
* GLM_ROOT_DIR
* SOFMIS_DEVIL -- for the DEVIL image 

## Building - Windows, Visual Studio 2010, 64-bit ##

Only out-of-source builds are supported.

First, clone the git repository locally from:

https://bitbucket.org/vector/sparks

Then gather dependencies and create the Visual Studio project file:

	cd sparks
	mkdir build-VisualStudio10Win64
	cd build-VisualStudio10Win64
	cmake .. -G "Visual Studio 10 Win64"
	start sparks.sln

Build the entire project TWICE (the first time will download and build some dependencies, including Intel TBB).

The second build should complete, then you can run the sparksGui program.

Note that you'll need to build the everything (ALL_BUILD) as Release before bulding the RelWithDebug target, as the Release version of the GLFW libraries are used for RelWithDebug.

The path to the directory with webfiles is hard-coded in the mongoose.conf file.
Change C:\src\sparks\www to the appropriate dir.

## Building - MacOSX ##

Note:  MacOSX is only supported for testing, neither primary interface (zSpace nor TrakStar) is supported on OSX. Only out-of-source builds are supported.

	cd spark
	mkdir build-xcode
	cd build-xcode
	cmake .. -G Xcode
	open sparks.xcodeproj


### sparkGui ###

The sparkGui.exe runs with options:

* "-trace" enable detailed trace logging to ./sparks.log
* "-debug" enable "Legacy" (aka, non-core profile supported) error catching to report opengl errors.

The sparks/src/main.cpp file contains the main() entry point for sparkGui.

# Scripting #


## Adding script states ##

States can be created using Lua scripts, C++ class or a mix of both. 

The state entered on startup is determined by the sparks/data/scripts/states/StartupState.lua script.  (I.e., StartupState is always the first state run, and by convention it does nothing but swith to another State.)  Typically, StartupState should switch to LoadingState for normal operation.

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
