# Memory Manager

NASA core Flight System Memory Manager Application

## Description

The Memory Manager application (MM) is a core Flight System (cFS) application that is a plug in to the Core Flight Executive (cFE) component of the cFS.

The cFS is a platform and project independent reusable software framework and set of reusable applications developed by NASA Goddard Space Flight Center. This framework is used as the basis for the flight software for satellite data systems and instruments, but can be used on other embedded systems. More information on the cFS can be found at [http://cfs.gsfc.nasa.gov](http://cfs.gsfc.nasa.gov)

The MM application is used for the loading and dumping system memory. MM provides an operator interface to the memory manipulation functions contained in the PSP (Platform Support Package) and OSAL (Operating System Abstraction Layer) components of the cFS. MM provides the ability to load and dump memory via command parameters, as well as, from files. Supports symbolic addressing.

MM requires use of the [cFS application library](https://github.com/nasa/cfs_lib).

## License

This software is licensed under the NASA Open Source Agreement. http://ti.arc.nasa.gov/opensource/nosa
