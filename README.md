cpu rayTracing in pure c++ displayed in console using regular characters.

features:
-
  - [Lombert's cosine law](https://en.wikipedia.org/wiki/Lambert%27s_cosine_law)
  - multithreading
  - shadows
  - flexible and customizable (not fully) `edit settings in main.cpp`
  - great performance `i get around 250fps on my dusty i5-6400 and 300fps on raspberry pi 5`

limitations (for now): 
-
  - one source of light
  - post processing `looping through entire image again may be very inefficient`
  - no soft shadows
  - no occlusion culling
  - hardcoded settings `some settings like amount of colors are hardcoded bc im still learning`
    
i intend on adressing all of those limitations. except post processing idk how to do it without huge drop in performance.

buliding
-
1. clone this repo
2. compile with `g++ main.cpp rayTracer.cpp` 
3. run `./a.out`

if it looks wrong you probably have too high resolution. try maximasing the window and scaling down the font or change the resolution in `main.cpp` (line 7 and 8)

if you want to play with some parameters, all of them are in main.cpp

![here are codes for ANSI coloring and other effects](https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences)

pro tips
-
here are all scene objects for now:
- Plane(Vec3 position, Vec3 direction, float size)
- Sphere(Vec3 position, float radius)
- Ring(Vec3 position, Vec3 direction, float inner_radius, float outer_radius)

you can animate any parameters in the main loop inside `main.cpp`

some terminals are better then others, i recommend something like kitty or ghosty becouse they are able to display high fps
![rayTracer_crt](https://github.com/user-attachments/assets/48df6ea0-d79f-4216-97e1-bfa36dca8887)
```//frame from cool retro term```
