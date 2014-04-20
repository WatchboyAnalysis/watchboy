TARGET		= analysisTemplate
#nTOFSimPlots

# Linux with egcs, gcc 2.9x, gcc 3.x (>= RedHat 5.2)
CXX           = g++
CXXFLAGS      = -O -Wall -fPIC
LD            = g++
LDFLAGS       = -O
SOFLAGS       = -shared

ObjSuf        = o
SrcSuf        = cxx
ExeSuf        =
DllSuf        = so
OutPutOpt     = -o # keep whitespace after "-o"

ROOTCONFIG   := root-config

ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --libs)
ROOTGLIBS    := $(shell $(ROOTCONFIG) --glibs)
#HASTHREAD    := $(shell $(ROOTCONFIG) --has-thread)


# debug mode trigger
#  SETDEBUG=0 : Optimization (default)
#  SETDEBUG=1 : Debug mode 
SETDEBUG=1

ROOTSYSDIR	= ${ROOTSYS}
CXXFLAGS        += -I${ROOTSYSDIR}/include
LDFLAGS       	+= -lm -L${ROOTSYSDIR}/lib

CXXFLAGS     += $(ROOTCFLAGS)
LDFLAGS      += $(ROOTLDFLAGS)
LIBS          = $(ROOTLIBS) $(SYSLIBS)
GLIBS         = $(ROOTGLIBS) $(SYSLIBS)

LDFLAGS 	+= -lMinuit

#HDRS            = ExternalFunctions.hh FitFunction.hh LikelihoodFunction.hh
#SRCS            = ExternalFunctions.cc FitFunction.cc LikelihoodFunction.cc
#HDRS            = ${KLDIR}/KLFitFunctions.hh ${KLDIR}/KLPreTriggEffCorr.hh  ${KLDIR}/SphericalVolume.hh ${KLDIR}/kuFUNC.hh
#SRCS            = ${KLDIR}/KLFitFunctions.cc ${KLDIR}/KLPreTriggEffCorr.cc  ${KLDIR}/SphericalVolume.cc ${KLDIR}/kuFUNC.cc

OBJS = $(SRCS:.cc=.o)
#OBJS	 	= libMinuit.so
#OBJLISTS	= KVFVirtualEvent.o KVFWriter.o KVFReader.o
#OBJLISTS	+= KVFConstHandler.o
#OBJS=$(patsubst %.o,%.${SUFFIX}.o,$(OBJLISTS))

#KLFitFunctions.o : ${KLDIR}/KLFitFunctions.cc ${KLDIR}/KLFitFunctions.hh

all: examples

EXAMPLE		= ${TARGET}.o

examples: ${OBJS} ${EXAMPLE}
	${CXX} ${CXXFLAGS}  -o ${TARGET} ${EXAMPLE} ${HDRS} ${OBJS} $(LIBS) $(GLIBS) $(LDFLAGS)

%.${SUFFIX}.o: %.cc
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm -f core ${OBJS} *.o

TAGS: $(SRCS) $(HDRS)
	etags -t $(SRCS) $(HDRS)


CXX           = g++
CXXFLAGS      = -O -Wall -fPIC
LD            = g++
LDFLAGS       = -O
SOFLAGS       = -shared

ObjSuf        = o
SrcSuf        = cxx
ExeSuf        =
DllSuf        = so
OutPutOpt     = -o # keep whitespace after "-o"

ROOTCONFIG   := root-config

ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --libs)
ROOTGLIBS    := $(shell $(ROOTCONFIG) --glibs)
#HASTHREAD    := $(shell $(ROOTCONFIG) --has-thread)

# debug mode trigger
#  SETDEBUG=0 : Optimization (default)
#  SETDEBUG=1 : Debug mode 
SETDEBUG=1

ROOTSYSDIR	= ${ROOTSYS}
CXXFLAGS        += -I${ROOTSYSDIR}/include
CXXFLAGS        += -I./include

CXXFLAGS     += $(ROOTCFLAGS)
LDFLAGS      += $(ROOTLDFLAGS)
LIBS          = $(ROOTLIBS) $(SYSLIBS)
GLIBS         = $(ROOTGLIBS) $(SYSLIBS)

LDFLAGS       	+= -lm -L${ROOTSYSDIR}/lib
LDFLAGS 	+= -lMinuit

#AOBJS           = kuFUNC.o  SolarSpectra.o  ElasticScattCrossSect.o KLFitFunctions.o
#OBJS            = ${addprefix obj/, ${AOBJS}}


all: examples

examples: ${OBJS} ${EXAMPLE}
	${CXX} ${CXXFLAGS}  -o ${TARGET} ${EXAMPLE} $(LIBS) $(GLIBS) $(LDFLAGS)


obj/%.o: src/%.cc
	${CXX} ${CXXFLAGS} $(LIBS) $(GLIBS) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf ./obj/*.o *~ ${TARGET} ${LIBNAME} ./include/*~ ./src/*~

