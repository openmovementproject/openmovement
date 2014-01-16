MATLAB Compiler

1. Prerequisites for Deployment 

. Verify the MATLAB Compiler Runtime (MCR) is installed and ensure you    
  have installed version 8.0 (R2012b).   

. If the MCR is not installed, do the following:
  (1) enter
  
      >>mcrinstaller
      
      at MATLAB prompt. The MCRINSTALLER command displays the 
      location of the MCR Installer.

  (2) run the MCR Installer.

Or download the Windows 64-bit version of the MCR for R2012b 
from the MathWorks Web site by navigating to

   http://www.mathworks.com/products/compiler/mcr/index.html
   
   
For more information about the MCR and the MCR Installer, see 
Distribution to End Users in the MATLAB Compiler documentation  
in the MathWorks Documentation Center.    


NOTE: You will need administrator rights to run MCRInstaller. 


2. Files to Deploy and Package

Files to package for Standalone 
================================
-plugin_v0p2.exe
-MCRInstaller.exe 
   -if end users are unable to download the MCR using the above  
    link, include it when building your component by clicking 
    the "Add MCR" link in the Deployment Tool
-This readme file 

3. Definitions

For information on deployment terminology, go to 
http://www.mathworks.com/help. Select MATLAB Compiler >   
Getting Started > About Application Deployment > 
Application Deployment Terms in the MathWorks Documentation 
Center.





