# game_engine

My game engine I made for a project for CSC 562 at North Carolina State University, Spring 2025.

# Building

Currently building is only available on Windows. Build by:

```
make all
```

# Running

The project can be run by 

```
cd bin;
./app_2D <scene json file>.json
```
### NOTE: You must ALWAYS be at the same level of the application to run correctly in the terminal. 
### NOTE: This application can only be run from the terminal as of now. Double clicking it would lead to an error as we need to provide the scene name.

# How the engine looks for data

Make sure your data is laid out as so:
```
app_2D
glfw3.dll
lua51.dll
data
|-->res
|    |
|    ----><resource_name_1>
|    |      |
|    |      |----><resource_name_1>.bin // model file
|    |      |----><resource_name_1>.fs  // shaders
|    |      |----><resource_name_1>.vs
|    |      |----><resource_name_1>.png
|    |
|    |
|    ----><resource_name_2>
|    |      |
|    |      |----><resource_name_2>.bin 
|    |      |----><resource_name_2>.fs
|    |      |----><resource_name_2>.vs
|    |      |----><resource_name_2>.png
|    .
|    .
|    .
|
-->scripts
   |--><script_name_1>.lua
   |--><script_name_2>.lua

--> screenbuffer.fs // This file MUST be there

```

# Converting models to .bin

A converter script ```objconverted.py``` has been provided to convert obj model files to bin files to be used with the engine. Use it as so:
```
python3 objconverter.py <source_file>.obj <dest_file>.bin

```
