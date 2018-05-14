# TODO

- Package electronics without breadboard
- Regularly reinitialize LCD (too fragile for now)
- Set more meaningful default values
- Improve extensibility of the amount of routines (today it's too much work)
- Add repeat for routines
- Add more meaninfgul routine documentation (wXX)
- Make a collection of routines to be readable
- Add a pose where everything goes off (arms down, face off, lights off, fan off)


# DONE

- Check why body.v0 routine gets limited to a few poses (rather than lots as expected) -> SerBot macro value
- Rename poses so that they are more meaningful
- Modify arms so that it receives a 0-9 value for each arm
- Figure out wifi setup
- Use 3rd char for poses
- LCD messages size can be controlled
- Make both arms operate simultaneously
- Send logs over the network
- Show more KPIs about the internals (as freeheap) in the logs
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
