This project has been upgraded to include additional functions and is intending to replace earlier versions maintained by Oliver Gruetzmann [ogruetzmann/ModeS](https://github.com/ogruetzmann/ModeS) and Pierre Ferran [pierr3/ModeS](https://github.com/pierr3/ModeS).
Find the latest version in the section [Releases](https://github.com/kusterjs/CCAMS/releases) (see right menu).

# CCAMS (Centralised code assignment and management system) plugin

This plugin was designed to support controllers of the VATSIM network when assigning a squawk to aircraft, also especially when the local prescribed squawk ranges are exhausted. The web-based part of the application is acting as the central interface to manage all squawk assignments and will also find alternative, non-used squawks outside the preferential range. Local ops/tech staff may define and fine tune the rule setting for the squawk assignment to comply with regional and local squawk ranges and schemes. This collaboration is done by issue reporting on github.
This plugin includes Mode S functionalities as introduced previously by Oliver Gruetzmann and Pierre Ferran. It is fully capabale to correctly differentiate between FAA and ICAO flight plan format. Additionally, plugin settings are available to customise the plugin behaviour and limit certain functionalities to comply with the vACCs local needs and requirements.

This plugin provides as the core functionality:
* Manual code assignment options for all aircraft on ground (improved popup menu, tag functions)
* Automatic assignment of squawk 1000 for capable aircraft flying above FL245 without assigned squawk or with generic codes like 0000, 1200, 2000, 2200
* Tag items representing Mode S EHS (Enhanced Surveillance) data

## Available tag items
* Transponder type: Show wether the aircraft is mode S or not
* EHS Heading: Reported magnetic heading of the aircraft
* EHS Roll Angle: Reported roll angle (L for LEFT and R for RIGHT + value in degrees)
* EHS GS: Reported groundspeed of the aircraft in knots.
* Mode S quawk error: Indicator of incorrect use or assignment of squawk 1000

## Available tag functions
* Assign mode S squawk: assigns code 1000 to eligible aircraft with mode S transponder
* Auto assign squawk: intended to be used instead of the default ES (F9) function, assigns code 1000 if eligible, otherwise assigns a squawk according the implemented code scheme
* Open SQUAWK assign popup: improved options compared to the default ES functions, including the above assign mode S option and a specific VFR code assignment

## Available plugin settings
The following settings can be added to the Plugins.txt file to customise the plugins behaviour:
* Add "CCAMS:codeVFR:[your default VFR code]" to define the code used when assigning a VFR code (by the specific option in the SQUAWK assign popup (if not defined, 7000 is used))
* Add "CCAMS:acceptFPLformatICAO:0" to suppress the assignment of squawk 1000 to aircraft using an ICAO format flight plan
* Add "CCAMS:acceptFPLformatFAA:0" to suppress the assignment of squawk 1000 to aircraft using an FAA format flight plan
* Add "CCAMS:AutoAssign:0" to suppress the automatic assignment of squawk 1000 to eligible aircraft above FL 245

## EuroScope set up
* Load up the plugin
* Set your mode S string to ```HLEGWQS```
* Optionally: Remove the Squawk code for S-mode transponders (to ensure squawk 1000 is only assigned by the plugin logic)
* Add ```1000``` to your VFR squawk list (to prevent duplicates indications for squawk 1000)
* Select the "CCAMS / Open SQUAWK assign popup" function to replace the default one in your list and tag definitions

## Add a country to the mode S list

If you would like to add a region or set of airports to the list of Mode S capable ones, create an [issue](https://github.com/kusterjs/CCAMS/issues) on this GitHub with your request. You can see all the currently enabled areas in the file [ver.txt](https://raw.githubusercontent.com/kusterjs/CCAMS/master/CCAMS/ver.txt).

## Server data
The current data used by the web-based part of the application can be observed on https://ccams.kilojuliett.ch/.
For any change, please create an [issue](https://github.com/kusterjs/CCAMS/issues) on this GitHub.
