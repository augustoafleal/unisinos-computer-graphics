# Execution Result
This application renders parametric curves (Bezier and Hermite) alongside 3D models (Mars, Moon, and asteroids) using OpenGL and GLFW. It supports interactive controls for camera movement and object manipulation.

## Description
The program initializes OpenGL context using GLFW and GLAD, loads shaders, and imports 3D models. It draws parametric curves and animates objects moving along these curves. Background stars are rendered as a textured quad. User inputs control the camera and object transformations.

> To visualize the parametric curves that the objects follow, you need to uncomment the drawing calls in the main loop. For example:
> ```cpp
> //glDrawArrays(GL_LINE_STRIP, 0, bezierAsteroidsControlPoints.size());
> ```
> Similarly, uncomment the following lines to draw the curves:
> - `//glDrawArrays(GL_LINE_STRIP, 0, bezierControlPoints.size());`
> - `//glDrawArrays(GL_LINE_STRIP, 0, hermiteControlPoints.size());`
>
> This will render the curves as line strips so you can see the paths the objects move along.

## Controls

### Object Selection
| Key | Action       |
|-----|--------------|
| `0` | Select Mars  |
| `1` | Select Moon  |

### Movement
| Key | Direction        |
|-----|------------------|
| `W` | Forward (Z-)     |
| `S` | Backward (Z+)    |
| `A` | Left (X-)        |
| `D` | Right (X+)       |
| `I` | Up (Y+)          |
| `J` | Down (Y-)        |

### Scaling
| Key     | Action          |
|---------|-----------------|
| `-`     | Decrease scale  |
| `=`     | Increase scale  |

> Scale is clamped between `0.1` and `0.7`.

### Rotation (for the selected object)
| Key | Axis of Rotation |
|-----|------------------|
| `X` | Rotate around X  |
| `Y` | Rotate around Y  |
| `Z` | Rotate around Z  |

### Camera Movement
| Key         | Action           |
|-------------|------------------|
| `↑` (Up)    | Move forward     |
| `↓` (Down)  | Move backward    |
| `←` (Left)  | Move left        |
| `→` (Right) | Move right       |

> The camera direction changes based on mouse movement.

### Other
| Key       | Action                    |
|-----------|---------------------------|
| `ESC`     | Close the application     |

## Notes
- The selected object is the only one affected by transformations.
- Object positions and transformations are updated every frame.

### Animation
![Execution Result_1](img/parametric_curves.gif)

### Reference

- Planets and asteroid was downloaded from [Free3D](https://free3d.com/)
- Stars background was downloaded from  [Pixabay](https://pixabay.com/pt/photos/estrelas-c%C3%A9u-noite-estrelado-1654074/)