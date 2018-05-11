# TODO

- Package electronics without breadboard
- Figure out wifi setup
- Send logs over the network
- Make both arms operate simultaneously
- Use 3rd char for poses
- LCD messages size can be controlled
- Regularly reinitialize LCD (too fragile for now)

# DONE

- Make all images have black background for consumption
- Make easier the image creation
- Improve the API regarding moves: if HAPPY, then allow to have a full control on a move of the arms and the faces used
- Make the servo control more stable (sometimes it is laggy)
- Put all Buffer in Buffer.h so that can use Buffer<N> for any N
- Put Configurables & Actors into Actors
- Add a Buffer.sprintf("%d",s) for Buffer that is overflow-safe
- Refactoring on Bot & friends to remove configurables totally (API otherwise is confusing)
- Fix the position of the servo
- Create a library with standard reusable actors (Led, Arm, Lcd)
- Fix the entangling of the physical arm threads (separate physical channels for cables, and threads in main physical body)
