rem Crossfire
rem This is the installer for the windows server crossfire32.exe
rem Run this bat file
rem compile the server with VC when needed
rem and unpack the maps in \share\ folder
rem c) Michael Toennies 2001
cd ..
md tmp
md var
md var\players
md var\unique-items
md var\template-maps
md var\datafiles
md share
md share\help
md share\plugins
copy lib\*. share\*.*
copy lib\*.path share\*.path
copy lib\crossfire.0 share\*.*
copy lib\crossfire.1 share\*.*
copy lib\help\*.* share\help\*.*
copy lib\treasures.bld share\treasures

echo off
echo .
echo ** unpack maps in \share before you run the server! **
