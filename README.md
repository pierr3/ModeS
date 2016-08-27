# This project is now maintained by Oliver Gruetzmann on [ogruetzmann/ModeS](https://github.com/ogruetzmann/ModeS), please download the plugin from there. This repository is only here for archiving purposes.

# Mode S plugin

This plugin was designed to simulate mode S correlation on EuroScope, for use on the VATSIM Network. Just load the plugin in EuroScope, and squawk 1000 will be assigned for all aircrafts on the ground flying within the mode S airspace.

## Available tag items

* Transponder type: Show wether the aircraft is mode S or not
* Mode S: Reported Heading: Reported magnetic heading of the aircraft
* Mode S: Roll Angle: Reported roll angle (L for LEFT and R for RIGHT + value in degrees)
* Mode S: Reported GS: Reported groundspeed of the aircraft in knots.

## EuroScope set up
* Load up the plugin
* Set your mode S string to ```HLEGWQS```
* Add ```1000``` to your VFR squawk list (to prevent squawk duplicates)

## Add a country to the mode S list

If you would like to add a series of airports to the mode S, create an issue on this GitHub with your request, you can see all the currently enabled areas in the file "version.txt"

The current mode S airspace for the plugin is as follows:

![Mode S Map](https://s3.eu-central-1.amazonaws.com/pithos/ShareX/2015/12/23.12.2015.modeS.gif)
