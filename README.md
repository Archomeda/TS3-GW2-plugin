TeamSpeak 3 Guild Wars 2 Plugin
===============================

This is a plugin that allows TeamSpeak to show some additional information about clients that are currently in-game in Guild Wars 2. It helps to identify people that are not using their in-game account name or character name.
In order to get information about someone else on the TeamSpeak server, that person has to install this plugin as well. The information is visible in the right panel of the TeamSpeak client (where the information of a channel or client is found when selected).


Features
----------------
- View character name and profession
- View current map, region and world name
- View name, direction and chat link of the closest waypoint nearby
- Check for updates automatically


Requirements
------------
- Windows 32-bit or 64-bit (Guild Wars 2 apparently only uses Mumble Link on Windows due to some restrictions on OS X)
- TeamSpeak 3.0.9 or higher (API version 19)
- Guild Wars 2 (obviously) 


Compiling notes
---------------
Compilation of this plugin has been tested in Visual Studio 2010 only. Newer versions should work, but it is not guaranteed that it will. Older versions will most likely not work though.

Inside the dependencies folder you can find a couple of third party header files: the TeamSpeak 3 Client Plugin SDK and [rapidjson](http://code.google.com/p/rapidjson/).


Legal stuff
-----------
In order to get the up-to-date information from Guild Wars 2, this plugin uses the Mumble Link API (which Guild Wars 2 officially supports). This method does **not** interact with the game in any way and therefore it should be safe to use. Check the [thread on the Guild Wars 2 forums](https://forum-en.guildwars2.com/forum/community/api/Map-API-Mumble-Mashup/first#post2256444) for more information.

