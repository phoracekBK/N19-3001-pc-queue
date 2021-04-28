# csv_maker

Application for buffering glass records receaved from MES system used for project N19-3001. Whole application is programmed in plain C programming language 

# Dependecies
There is several main dependecies:
* aclib - library of dynamic data structures, mainly string, linked list, array list, ...
* s7lib - library based on snap7 library, creating high-level interface for comunication throught S7 library of the Siemens PLCs
* POSIX interface
* gcc - translator for c language
* make
* pkg-config

The best way for set up build enviroment on Windows platform is using MSYS2 system.

# Compilation/Instalation
Instalation is prety easy, if the enviroment prepared (see previous chapter), then just run foloving command:

$ make

For testing purpose it is possible compile and run application automaticly:

$ make run

For clean up project directory is used command:

$ make clean
