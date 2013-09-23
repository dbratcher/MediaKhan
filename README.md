#MediaKhan

A distributed fuse-based media filesystem.

## Contents

1. [Installation](https://github.com/dbratcher/MediaKhan#1-installation)
2. [Client Configuration](https://github.com/dbratcher/MediaKhan#2-client-configuration)
3. [Server Configuration](https://github.com/dbratcher/MediaKhan#3-server-configuration)
4. [Program Layout](https://github.com/dbratcher/MediaKhan#4-program-layout)
5. [Design and Implementation](https://github.com/dbratcher/MediaKhan#5-design-and-implementation)
6. [Acknowledgements](https://github.com/dbratcher/MediaKhan#6-acknowledgements)


### 1. Installation

- Install at least one database (Voldemort and/or Redis):
    - Install Voldemort (http://www.project-voldemort.com/voldemort/)
        - install oracle-jdk (or openjdk) if you don't have it  
        - install ant if you don't have it
        - run ant in voldemort directory
    - Install Redis (http://redis.io/)
- Install C Clients for Installed Databases:
    - Install Voldemort C Client (located in the voldemort package under clients)
        - on ubuntu (sudo apt-get)
            - install libtool
            - install libboost-all-dev
            - install libboost-system-dev
            - install libprotobuf-dev
            - install libboost-doc
            - install doxygen
            - install automake	
        - on mac (sudo port)
            - install libtool
            - brew install protobuf (mac ports doesn't seem to work for this one)
            - brew install boost
            - install automake
            - install doxygen
            - install autoconf
        - run ./autogen.sh 
        - run ./configure
        - update utils/Makefile LIBS to include -lboost_thread-mt
        - run make
        - run make install
    - Install Hiredis (https://github.com/redis/hiredis)
- Install Fuse (http://fuse.sourceforge.net/) or OSXFuse for macs (http://osxfuse.github.com/)
- Get latest source from https://github.com/dbratcher/MediaKhan.
    - update paths in stores.txt to point to media
    - cmake .
    - make
- Install id3lib (use to generate metadata from cloud files) - use sudo apt-get install on linux or sudo port install on mac (requires macports)
- Install mp3info (use to generate metadata -see filetypes.txt) - use sudo apt-get install on linux or sudo port install on mac (requires macports)
- Install mpg321 (use to play songs) - use sudo apt-get install on linux or sudo port install on mac (requires macports)

### 2. Client Configuration

1. Pick a Server
    - Run Voldemort Server. Khan assumes server is at localhost:6666
    - Or run Redis Server. Edit khan.cpp to reflect what sort of database you are using.
2. Create /tmp/fuse/ if necessary
3. Call ./khan /tmp/fuse/ -d. (will show debug messages for filesystem - type mkdir stats to see timing info) On MacOSX use -s instead of -d, or if you must see the debug output use -s and -d and only view the system through Finder. (the terminal causes it to crash occasionally when in debug mode)
4. Look at /tmp/fuse to see filesystem in another prompt.
5. shopt -u interactive_commands on ubuntu, interactive_comments on mac, allows for hashtag path support

### 3. Server Configuration

The instructions on how to start the servers is with the respective server code;
 it is a rather open ended part of the the architecture, you can set up the metadata 
servers in lots of different ways. 

If you look at ./NOTES in the voldemort package, there are instructions for a 
single node cluster as follows:
    ./bin/voldemort-server.sh config/single_node_cluster &

Now khan compiles for redis by default at the moment, so if you want to use 
voldemort, you need to change line 10 in khan.cpp.

To run a single redis server run ./src/redis-server in the redis package.

To use webdav, you mount the webdav directory and then specify it as a media server in khan.cpp on line 14.

### 4. Program Layout

Majority of code in khan.cpp.
Structures in khan.h

### 5. Design and Implementation

Designed for distributed robust file system activities. Based off voldemort and redis
but interface should be similarly applicabale to cassandra, drift, etc.

Meta data storage abstracted from the particular database types. Khan
just capitalizes on the mapping functionality. Plans to utilize EVPath for more
dynamic mappings and marshalling of data across network connections.

### 6. Acknowledgements

- Voldemort Paper
- Paper on modern filesystems.
- Paper on distributed filesystems
- Voldemort
- Redis
- EVPath
- FUSE
- Davfs
- Drift
- etc.
