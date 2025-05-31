# Execution Result
This project presents a 3D simulation of the Moon and Mars using **OpenGL** and **GLFW**. Users can switch between the two celestial bodies, control their position, scale, and rotation along different axes, and navigate the scene using a free-look camera controlled via mouse and keyboard. The scene includes a dynamic starfield background.

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
![Execution Result_1](img/planets.gif)

### Reference

- Planets was downloaded from [Free3D](https://free3d.com/)
- Stars background was downloaded from  [Pixabay](https://pixabay.com/pt/photos/estrelas-c%C3%A9u-noite-estrelado-1654074/)