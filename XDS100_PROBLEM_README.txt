IF YOU ARE HAVING PROBLEMS WITH "Error -151" (XDS100 CONNECTIVITY PROBLEMS), READ NEXT.


If you are having troubles with error -151, check xds100 connection options,
more specific "Debug Probe Selection".


To check "Debug Probe Selection" setup, do next:

1. Open file "targetConfigs.ccxml",
2. Click on "Advanced" tab (somewhere at the bottom),
3. You will see "Debug Probe Selection" parameter.

Now, there are two choises for "Debug Probe Selection" to select:
1. "Only one XDS100 installed" or
2. "Select by serial number".


Which setup to select, then?

There are again (at least) two choises. 
1. If you are operating with only one microcontroller unit (MCU), 
   it's better to select "Only one XDS100 installed".
   The CCS will find MCU automatically. There are usually no problems.
2. If you are operating with multiples MCUs
   (or you really want to learn the second option),
   select "Select by serial number" option.
   You will need to enter serial number (unique for every device) of the connected device, 
   and you will be able to download program only to device with specifed serial number.




How to obtain serial number of MCU?

1. Run command prompt (terminal). The shortcut for this is "Windows" + "R" (stand for RUN).
   "Windows" button is between CTRL and ALT,
2. Enter "cmd" and the command prompt will open,
3. Enter path e.g. C:\ti\ccsv6\ccs_base\common\uscif or something similar,
   where you have instaled Code Composer Studio (CCS).
4. Enter "xds100serial.exe" and if you are in the rigth folder and connected with MCU, 
   it should be printed something like this:
   
   VID/PID    Type            Serial #    Description
   0403/a6d0  XDS100v1/v2     TMB60203    Texas Instruments Inc.XDS100 Ver 2.0
   
5. Read the serial number (in this case: TMB60203)
   and rewrite it in box "Enter the serial number".
