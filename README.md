# core Flight System (cFS) Memory Manager Application (MM)

## Introduction

The Memory Manager application (MM) is a core Flight System (cFS) application 
that is a plug in to the Core Flight Executive (cFE) component of the cFS.  
  
The MM application is used for the loading and dumping system memory. MM 
provides an operator interface to the memory manipulation functions contained
in the PSP (Platform Support Package) and OSAL (Operating System Abstraction 
Layer) components of the cFS. MM provides the ability to load and dump memory 
via command parameters, as well as, from files. If the operating system 
supports symbolic addressing, MM allows specifying the memory address using a 
symbolic address.   

The MM application is written in C and depends on the cFS Operating System
Abstraction Layer (OSAL) and cFE components.  There is additional MM application
specific configuration information contained in the application user's guide.

User's guide information can be generated using Doxygen (from top mission directory):
```
  make prep
  make -C build/docs/mm-usersguide mm-usersguide
```

## Software Required

cFS Framework (cFE, OSAL, PSP)

A demonstration bundle of the Core Flight System including the cFE, OSAL, and PSP can be obtained at https://github.com/nasa/cfs

For information about a mission ready cFS bundle, see: https://github.com/nasa/cFS#cfs-gov-mission-ready-version

## Known issues

See all [open issues](https://github.com/nasa/MM/issues) and closed to milestones later than this version.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
