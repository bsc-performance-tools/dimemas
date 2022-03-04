This package contains the source code of Dimemas simulator, its graphical 
user interface and the Paraver to Dimemas trace translator, 'prv2dim'.

You can find the old Dimemas trace format (.trf) to new format (.dim) translator 
'trf2dim' until Dimemas package version 5.3.0

## Dependencies

To compile the you will also need the Boost libraries. Version 1.36 or later
are required. You can obtain this set of libraries at:
   * Boost C++ Libraries 
        http://www.boost.org


The graphical user interface is codified using Java and packaged as a JAR
file. By this reason, you will need to have installed on your system a SDK
distribuation as well as the Jar Archiver. We use the official Oracle/Sun Java 
distribution (even not being 100% open source), and we encourage using it. It
can be obtained at:

	* SDN Developer Resources for Java Technology
        http://java.sun.com

Java 6 (SDK 1.6) or newer is required to properly generate the Dimemas user
interface.


## Specific Compilation Options

There is a wide range of compilation options, to fine tune the simulation.
Check 'configure --help' for specific compilation options.

In order to adjust the compiler in terms of optimization or debugging options, 
you must provide specific compiler parameters using the CFLAGS and CXXFLAGS 
environment variables.

Rest of the options usually regard to very specific simulation facts.Do not
hesitate to contact us to solve any doubts regarding the specific Dimemas 
tune.

## Dimemas external communications model support
   
   To extend the Dimemas simulator by using an external communication library,
please refer to the README file include in the Simulator/lib_extern_model in
the source code packae, or $DIMEMAS_HOME/share/lib_extern_model_example in the
installation directory.

   ******* Dimemas contact e-mail: tools@bsc.es ********


