#MediaKhan

A distributed fuse-based media filesystem.

## Contents

1. Setup and Installation
2. Running a khan client.
3. Setting up metadata and media servers.
4. Program Layout
5. Design and Implementation
6. Acknowledged Papers and Projects


### 1. Installation

1. Install Voldemort
 - install oracle-jdk (or openjdk) if you don't have it  
 - install ant if you don't have it
 - run ant in voldemort directory
2. Install Voldemort C Client.
 - install libtool
 - install libboost-all-dev
 - install libboost-system-dev
 - install libprotobuf-dev
 - install libboost-doc
 - install doxygen
 - install automake	
 - run ./autogen.sh 
 - run ./configure
 - update utils/Makefile LIBS to include -lboost_thread-mt
 - run make
 - run make install
3. Install Redis.
4. Install Hiredis.
5. Install Fuse.
6. Get latest source from svn.research.cc.gatech.edu/kaos/khan/branches/media.
 - update paths in khan.h and khan.cpp to point to media (line 57 and 14 respectively)
 - Make
9. Install mp3info (use to generate metadata -see filetypes.txt)
10. Install mpg321 (use to play songs)

### 2. The Client

1. Pick a Server
 - Run Voldemort Server. Khan assumes server is at localhost:6666
 - Or run Redis Server. Edit khan.cpp to reflect what sort of database you are using.
2. Create /tmp/fuse/ if necessary
3. Call ./khan /tmp/fuse/ -d. (will show debug messages for filesystem - type mkdir stats to see timing info)
4. Look at /tmp/fuse to see filesystem in another prompt.

### 3. Media and Metadata Servers

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

### 6. Acknowldgements

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
