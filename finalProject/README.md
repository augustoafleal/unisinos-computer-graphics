# Execution Result
This application renders parametric curves (Bezier and Hermite) alongside 3D objects such as a basketball and hoop using OpenGL and GLFW. It includes two main scenes: an object selection scene and a gameplay scene, where players can throw a ball following a Bezier trajectory toward the hoop. The application features interactive camera controls, dynamic lighting, and collision detection.

## Description
The program initializes the OpenGL context using GLFW and GLAD, loads shaders and 3D models, and displays interactive scenes. In the selection scene, users choose a ball model that will be used during gameplay. In the gameplay scene, the ball follows a parametric (Bezier or directional) trajectory and interacts physically with the environment (hoop, backboard, ground). Parametric curves are optionally rendered, and background elements (like stars and floor) are drawn with shaders and textured quads. The score is visually represented by 3D number models animated along a Hermite curve.

> To visualize the parametric curves in the gameplay scene, you must enable them manually. Open the basketball_config.json file and set the following key to true:
> ```json
> "showParametricCurves": true
> ```
> 
## Controls

### Object Selection
| Key | Action            |
|-----|-------------------|
| `0` | Select Basketball |
| `1` | Select Orange     |
| `2` | Select Pumpkin    |

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

> Camera movement is constrained within a fixed range along the x, y, and z axes to keep the scene navigation within viewable limits.

### Throwing
| Mouse Button      | Action                        |
|-------------------|-------------------------------|
| Left Click        | Normal (straight) throw       |
| Right Click       | Curved throw (via Bézier path)|

### Other
| Key       | Action                    |
|-----------|---------------------------|
| `ESC`     | Close the application     |

## Notes
- The selected object is the only one affected by transformations.
- Object positions and transformations are updated every frame.
- After scoring three baskets, a flash scene is triggered and the score resets to 0.

### Animation
#### Scene 1
![Execution Result_1](img/cena_1.gif)
#### Scene 2
![Execution Result_1](img/cena_2_1.gif)
![Execution Result_1](img/cena_2_2.gif)


### Reference
- Objects was downloaded from [Free3D](https://free3d.com/)
- Stars and floor backgrounds was downloaded from  [Pixabay](https://pixabay.com/)
