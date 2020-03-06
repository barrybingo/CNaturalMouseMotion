## CNaturalMouseMotion ##

Move the mouse like a human via code.

This is a C++11 header only port of Joonas Vali's Java NatualMouseMotion at https://github.com/JoonasVali/NaturalMouseMotion

Distinctions:

  * Less factories
  * No getters/setters
  * Classes that wrap single functions have been replaced by std::function, or structs with operator() that can be contained in std::function
  * No ScreenAdjustedNature - TODO possibly add later; for multiple screens could add a screen parameter to nature or move(x,y,screen)
  * Move is a single function that takes a nature and coordinates, so everything not coordinates resides in MotionNature

Features:

  * **Deviation**: Deviation leads the mouse away from direct trajectory, creating and arc instead of straight line
  * **Noise**: Noise creates errors in the movement, this can simulate hand shakiness, someone using a non accurate mouse or bad surface under the mouse.
  * **Speed** and **flow**: Speed and flow are defining the progressing of the mouse at given time, for example it's possible that movement starts slow and then gains speed, or is just variating.
  * **Overshoots**: Overshoots happen if user is not 100% accurate with the mouse and hits an area next to the target instead, requiring to adjust the cursor to reach the actual target.
  * **Coordinate translation**: Coordinate translation allows to specify offset and dimensions to restrict a movement in a different area than the screen or in a virtual screen inside the real screen.

## Usage: ##

Moving the mouse requires **destination cooridnates** and a **nature of movement**.
Destination cooridnates are self exmplanatory though require a little thinking about with multiple screens.


```cpp
#include "NaturalMouseMotion.h"
int main(int argc, char** argv)
{
    auto nature = NaturalMouseMotion::DefaultNature::NewFastGamerNature();
    NaturalMouseMotion::Move(&nature, 250, 250);
    return 0;
}
```

## Building Tests and Example: ##

Linux:
------

```bash
mkdir build
cd build
cmake ..
make
# Run tests
./Test/NaturalMouseMotion_test

# Run the example CLI
./Example/NaturalMouseMotion -i -f -x 500 -y 500
```

Windows:
-------

Easy way is to use VSCode with a cmake plugin (CMake Tools) and a gtest plugin (GoogleTest Adapter)


## Precooked Natures ##

  * **GrannyMotionNature** is a slow and clumsy mouse movement with a lot of noise.
  * **FastGamerMotionNature** is a fast and persistent movement with low noise, simulating behavior of someone who knows where the click should land, like a professional gamer.
  * **AverageComputerUserMotionNature** is a medium speed movement with average settings, slower than fast gamer, but much faster than granny motion.
  * **RobotMotionNature** is featureless constant speed robotic movement with no mistakes.

## TODO ##

  * ScreenAdjustedNature or at least support multiple screens
  * Remove throws