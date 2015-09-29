CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -std=c++11 -pthread -D_FILE_OFFSET_BITS=64  

SOBJS = comnode.o Util.o Client_node.o Server_node.o

LIBS = -lpthread

TARGET = comnode

$(TARGET):	$(SOBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOBJS) $(LIBS)

all:	 $(TARGET) $(CTARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(CTARGET)  $(SOBJS) $(COBJS)
