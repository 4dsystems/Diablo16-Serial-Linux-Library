![image](http://www.4dsystems.com.au/downloads/4DLogo.png)

SPE Library Diablo for Raspberry Pi
====================

4D Systems Raspberry Pi Library for SPE Diablo

Library for the Raspberry Pi to allow easy communication between 4D Intelligent Display modules running Serial Environment programmed from Workshop 4, and the Raspberry Pi.

This library is also required for the Raspberry Pi demo programs.


## Installation of SPE Library Diablo for Raspberry Pi
=======================================================

  make

  sudo make install
  
  sudo ldconfig -v

## To Uninstall SPE Library Diablo for Raspberry Pi
=================================

  sudo make uninstall
  


## Disable Linux from using the Pi Serial Port so the SPE-Diablo library can instead
=================================================================================
* From terminal, launch leafpad (or your chosen editor) with root:

  sudo leafpad

* Nagivate to /boot/cmdline.txt and remove the following text (LEAVE everything else intact)

  kgdboc=ttyAMA0,115200 console=tty1  
  
* Save the file, overwriting the existing one.
  
* Navigate and edit /etc/inittab
  
* Comment out the bottom line by putting a '#' symbol at the start of it, where the bottom line is:
  
  T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100
  
* Save the file, overwriting the existing one
  
* Reboot your Raspberry Pi
