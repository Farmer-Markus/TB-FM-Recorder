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

To change to one of the other stations search for these lines in the `main.cpp` file:

        const char* url = "https://listener1.mp3.tb-group.fm/tb.mp3";
        const char* apiUrl = "https://www.technobase.fm/index.php?option=com_broadcast&task=current.playhistory&format=json&station=technobase.fm";

You can get the first url by going onto the tb website of your choice (example: trancebase.fm) and click on the `streams` tab. Then just copy the url of the first audio stream link and replace the old one in the file.
To get the right songs you also need to change the link (only at the end at "station=technobase.fm") Just replace the technobase.fm with your wanted station (exambple "station=trancebase.fm") <br>
You **will** need to compile the program again cuz I'm to lazy to create a command line option for choosing the station. <br>
Maybe I'll do that when rewriting the recorder... Maybe
