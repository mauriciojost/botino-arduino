# TODO

- Found 3.5 mA used only by a connected black LCD, need to find a command to switch it off
- Otherwise measurements on deep sleep mode is ~ 0.3 mA, calculate some battery life with such consumption (and ~130 mA when live)
- Need to handle the initialization of all non-generic properties via a wizard or something
- Improve memory use wrt JSON parsing (ParamStream is a memory hog, array + JsonBuffer, when actually
JsonBuffer could be used right away as a sink for a stream in httpGet and httpPost)). + prefer heap (big) over stack (4k), i.e. 
switch from heap
String json = readData(cocktailDataPath);
StaticJsonBuffer<2000> jsonBuffer;
JsonObject& root = jsonBuffer.parseObject(json);
to stack

DynamicJsonBuffer jsonBuffer(bufSize);  
File myFile = SD.open(cocktailDataPath);
JsonObject& root = jsonBuffer.parseObject(myFile);
- Make README mention main4ino main server
- Mention auto-update for firmware
- Mention deep sleep mode (in progress)



# DONE

- The LED that collides with the deepsleep mode should not be used to avoid problems
- Package electronics without breadboard
- Add smoke test that includes a whole architecture (to ensure it goes smootly for 100 cycles)
- Find better alternative to scripts and replace them (simulator, format, ...) < won't do
- Make body be able to call ifttt event in a pose
- Improve simulator
- Add one-shot timing setting
- Share settings between uploader and simulator
- Make a good pass and fix data types (avoid char, use better uint8 for instance)
- Fix warnings
- Improve documentation (use tables when convenient)
- Add a simulator
- Create script to set up wifi parameters for the first time.
- Version the docker images
- Add a move where you show a random fortune message -> Done (Quotes)
- Add repeat for routines -> Won't do
- Allow to combine moves (first move 0, then move 1 x 2 times) -> Won't do
- Create a way to ease the image creation -> spreadsheet
- Make key for WIFI pass encryption configurable
- Tell the WIFI & pass you should set up in your hotspot phone for botino to access the internet.
- Tell the name & chip ID during the init.
- Make the botino auto-document itself by publishing the instructions to a page -> Too complex, better simply improve the README.md
- Allow to define new images via API
- Set more meaningful default values -> Not clear after 2 weeks I wrote it, discarding
- Make button execute a given configurable move too!
- Improve extensibility of the amount of routines (today it's too much work)
- Regularly reinitialize LCD (too fragile for now) -> Not really needed if everything is well connected as now
- Make the servo startup smoothly
- Support button
- Add a pose where everything goes off (arms down, face off, lights off, fan off)
- Add more meaninfgul routine documentation (wXX)
- Make a collection of routines to be readable
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
