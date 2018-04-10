# TODO

- Improve the API regarding moves: if HAPPY, then allow to have a full control on a move of the arms and the faces used
- Make the servo control more stable (sometimes it is laggy)
- Create a library with standard reusable actors (Led, Arm, Lcd)

# DONE

- Put all Buffer in Buffer.h so that can use Buffer<N> for any N
- Put Configurables & Actors into Actors
- Add a Buffer.sprintf("%d",s) for Buffer that is overflow-safe
- Refactoring on Bot & friends to remove configurables totally (API otherwise is confusing)
