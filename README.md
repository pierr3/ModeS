This project has been upgraded to include additional functions and is intending to replace earlier versions maintained by Oliver Gruetzmann [ogruetzmann/ModeS] and Pierre Ferran [pierr3/ModeS].

# CCAMS (Centralised code assignment and management system) plugin

This plugin was designed to support controllers of the VATSIM network when assigning a squawk to aircraft. The web-based part of the application is acting as the central interface to manage all squawk assignment. The "rules" covered initially are following the VATEUD code assignment scheme. Therefore, the plugin primarily is intended to serve VATEUD members. Local ops/tech staff may define and fine tune the rule setting for the squawk assignment in collaboration with VATEUD8.
This plugin includes Mode S functionalities as introduced previously by Oliver Gruetzmann and Pierre Ferran. Additionally, plugin settings have been introduced to customise the plugin behaviour and limit certain functionalities to comply with the vACCs local needs and requirements.
Manual code assignment is available for all aircraft on ground (an improved menu for squawk assignment is provided). Additionally, automatic assignment of squawk 1000 is available for capable aircraft above FL245 without assigned sqauwk or with generic codes like 0000, 1200, 2000, 2200.

## Available tag items

to be completed

## Available tag functions

to be completed

## EuroScope set up
* Load up the plugin
* Set your mode S string to ```HLEGWQS```
* Add ```1000``` to your VFR squawk list (to prevent squawk duplicates)
* Set the "CCAMS / Open SQUAWK popup" function to replace the default

## Add a country to the mode S list

If you would like to add a region or set of airports to the list of Mode S capable ones, create an issue on this GitHub with your request, you can see all the currently enabled areas in the file "version.txt"
