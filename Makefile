CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -std=c++11 -pthread -D_FILE_OFFSET_BITS=64  

COBJS = client.o
SOBJS = server.o

LIBS = -lpthread

TARGET =	server
CTARGET =	client

$(TARGET):	$(SOBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOBJS) $(LIBS)
$(CTARGET):	$(COBJS)
	$(CXX) $(CXXFLAGS) -o $(CTARGET) $(COBJS) $(LIBS)

all:	 $(TARGET) $(CTARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(CTARGET)  $(SOBJS) $(COBJS)
