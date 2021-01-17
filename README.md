This project has been upgraded to include additional functions and is intending to replace earlier versions maintained by Oliver Gruetzmann [ogruetzmann/ModeS] and Pierre Ferran [pierr3/ModeS].

# CCAMS (Centralised code assignment and management system) plugin

This plugin was designed to support controllers of the VATSIM network when assigning a squawk to aircraft. The web-based part of the application is acting as the central interface to manage all squawk assignment. The "rules" covered initially are following the VATEUD code assignment scheme. Therefore, the plugin primarily is intended to serve VATEUD members. Local ops/tech staff may define and fine tune the rule setting for the squawk assignment in collaboration with VATEUD8.
This plugin includes Mode S functionalities as introduced previously by Oliver Gruetzmann and Pierre Ferran. Additionally, plugin settings have been introduced to customise the plugin behaviour and limit certain functionalities to comply with the vACCs local needs and requirements.
Manual code assignment is available for all aircraft on ground (an improved menu for squawk assignment is provided). Additionally, automatic assignment of squawk 1000 is available for capable aircraft above FL245 without assigned sqauwk or with generic codes like 0000, 1200, 2000, 2200.

## Available tag items
* Transponder type: Show wether the aircraft is mode S or not
* EHS Heading: Reported magnetic heading of the aircraft
* EHS Roll Angle: Reported roll angle (L for LEFT and R for RIGHT + value in degrees)
* EHS GS: Reported groundspeed of the aircraft in knots.

## Available tag functions
* Assign mode S squawk: assigns code 1000 to eligible aircraft with mode S transponder
* Auto assign squawk: intended to replace the default ES (F9) function, assigns code 1000 if eligible, otherwise assigns a squawk according the implemented code scheme
* Open SQUAWK assign popup: improved options compared to the default ES functions, including the above assign mode S option and a specific VFR code assignment

## Available plugin settings
The following settings can be added to the Plugins.txt file to customise the plugins behaviour:
* Add "CCMAS:codeVFR:[your default VFR code]" to define the code used for VFR aircraft (both by the auto assign tag function and the specific option in the SQUAWk assign popup), if not defined, 7000 is used
* Add "CCMAS:acceptFPLformatICAO:0" to suppress the assignment of squawk 1000 to aircraft using an ICAO format flight plan
* Add "CCMAS:acceptFPLformatFAA:0" to suppress the assignment of squawk 1000 to aircraft using an FAA format flight plan
* Add "CCMAS:AutoAssign:0" to suppress the automatic assignment of squawk 1000 to eligible aircraft above FL 245

## EuroScope set up
* Load up the plugin
* Set your mode S string to ```HLEGWQS```
* Add ```1000``` to your VFR squawk list (to prevent squawk duplicates)
* Set the "CCAMS / Open SQUAWK popup" function to replace the default

## Add a country to the mode S list

If you would like to add a region or set of airports to the list of Mode S capable ones, create an issue on this GitHub with your request, you can see all the currently enabled areas in the file "version.txt"
