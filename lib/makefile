#makefile

export CC  := gcc
export CPP := g++ 
export CXX := g++ 
export RM  := rm -rf
export MKDIR := mkdir -p
export AR  := ar



.SUFFIXES: .out .c .cc .o .h 

SRC = pgen-funcs.cc \
	  packet.cc \
	  eth.cc \
	  arp.cc \
	  ip.cc \
	  icmp.cc \
	  tcp.cc \
	  udp.cc \
	  netutil.cc
HDR = pgen.h \
	  packconf.h \
	  address.h \
	  pgen-funcs.h \
	  packet.h \
	  mytypes.h 
OBJ = $(SRC:.cc=.o)

CFLAGS = -Wall -fPIC


all: libpgen.a
#all: libpgen.so


libpgen.a: $(OBJ)
	$(RM) -f $@
	$(AR) rc $@ $(OBJ)



libpgen.so: $(OBJ)
	$(CPP) $(OBJ) -shared -o $@  $(CFLAGS)
#	$(RM) *.o




.cc.o: 
	$(CPP) -std=c++11 -fPIC -c  $<  -o $@  










#------------------------------OTHERS----------------------------------#

clean:
	$(RM) *.o *.out *.a *.so




