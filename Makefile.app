## your application name here
APP=main
SRC=$(APP).cpp Kmeans.cpp Kmeans.h Cluster.cpp Cluster.h
## extra object files for your app here
OBJ=

prefix = /home/nikhil/Indri/installation
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
INCPATH=-I$(includedir)
LIBPATH=-L$(libdir)
CXXFLAGS=-DPACKAGE_NAME=\"Indri\" -DPACKAGE_TARNAME=\"indri\" -DPACKAGE_VERSION=\"5.6\" -DPACKAGE_STRING=\"Indri\ 5.6\" -DPACKAGE_BUGREPORT=\"project@lemurproject.org\" -DINDRI_STANDALONE=1 -DHAVE_LIBM=1 -DHAVE_LIBPTHREAD=1 -DHAVE_LIBZ=1 -DHAVE_LIBIBERTY=1 -DHAVE_NAMESPACES= -DISNAN_IN_NAMESPACE_STD= -DHAVE_FSEEKO=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_EXT_ATOMICITY_H=1 -DP_NEEDS_GNU_CXX_NAMESPACE=1 -DHAVE_MKSTEMP=1 -DHAVE_MKSTEMPS=1 -DNDEBUG=1   -g -O3 $(INCPATH) -std=c++0x
CPPLDFLAGS  =  -lindri -liberty -lz -lpthread -lm 

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(APP) $(OBJ) $(LIBPATH) $(CPPLDFLAGS)

clean:
	rm -f $(APP)



