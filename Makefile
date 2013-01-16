FUSE_LIBS=$(shell pkg-config fuse --libs)
VOLD_LIBS=$(shell pkg-config voldemort --libs)
FUSE_FLAG=$(shell pkg-config fuse --cflags)
VOLD_FLAG=$(shell pkg-config voldemort --cflags)

khan : khan.o log.o redis.o voldemort.o
	g++ -L/usr/local/lib -L/usr/lib khan.o redis.o voldemort.o log.o -o khan -lfuse -lvoldemort -lrt -lhiredis

khan.o : khan.cpp log.h redis.h khan.h 
	g++ $(FUSE_FLAG) $(VOLD_FLAG) -c khan.cpp

redis.o : redis.cpp redis.h
	g++ $(FUSE_FLAG) $(REDIS_FLAG) -c redis.cpp

voldemort.o : voldemort.cpp voldemort.h
	g++ $(FUSE_FLAG) $(VOLD_FLAG) -c voldemort.cpp

log.o : log.cpp log.h 
	g++ $(FUSE_FLAG) $(VOLD_FLAG) -c log.cpp

clean :
	rm -f khan *.o
	rm -rf logFile
