# spotify_floating_lyrics

![Language](https://img.shields.io/badge/language-c++-green.svg?style=plastic)

This is a simple c++ script to display a floating lyrics of the currently playing song on Spotify.

* Uses [Spotify developer application API](https://developer.spotify.com/) to get the currently playing song.
* Displays the lyrics line by line using QT with transparent window on top of screen.
* mouse right click on the floating lyrics can drag and change the lyrics position.
* mouse left click on the floating lyrics can close the lyrics window.
* changing config/config.json can custom colour for the lyrics

## Build

* QT(6.4.1) and QT Creater(9.0.0) is used to auto build and deploy(using windeployqt.exe) the application

## Libraries

* [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)

> API used: [akashrchandran/spotify-lyrics-api](https://github.com/akashrchandran/spotify-lyrics-api)
