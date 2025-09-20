### Technobase FM Recorder for Linux
This project was created in around 2 days so the code is **trash!** <br>
I think I will rewrite everything at given time but at least it should work.

## Compiling
You will need cmake and OpenSSL installed!
I should omly work on Linux and maybe on macos but windows is not supported at all

    mkdir build && cd build
    cmake ..
    make

This should build the project

## Using
You should be able to execute the program with a few command line arguments:

    --help                 | Shows help menu
    --umin-time <time>     | Set minimum record time
    --umax-time <time>     | Set maximum record time
    --udj-change <dj-name> | Set name of dj to stop recording after 
