TARGET		= AnalysisResolution

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

SETDEBUG=1

ROOTSYSDIR	= ${ROOTSYS}
CXXFLAGS        += -I${ROOTSYSDIR}/include
LDFLAGS       	+= -lm -L${ROOTSYSDIR}/lib

CXXFLAGS     += $(ROOTCFLAGS)
LDFLAGS      += $(ROOTLDFLAGS)
LIBS          = $(ROOTLIBS) $(SYSLIBS)
GLIBS         = $(ROOTGLIBS) $(SYSLIBS)

LDFLAGS 	+= -lMinuit

ifndef SBLMCDIR
SBLMCDIR = $(SBLMC)
endif

IDIR        	+= $(SBLMCDIR)/include
CXXFLAGS	+= -I$(IDIR)
LDFLAGS  += $(shell root-config --libs) $(SBLMC)/dict.cxx
	 
SDIR       	:= $(SBLMCDIR)/src
HDIR       	:= $(SBLMCDIR)/include

SRCS		:= $(SDIR)/Event.cc 

HDRSTMP		= $(subst src,include,$(SRCS))
HDRS		:= $(HDRSTMP:%.cc=%.hh)

VPATH = $(SDIR):$(HDIR)

OBJS = $(SRCS:.cc=.o)

all: examples

#dict.cxx :
#	rootcint -f dict.cxx -c include/Event.hh include/EventLinkDef.h
 
#libEventLib.so: dict.cxx src/Event.cc
#	g++ -shared -o$@ -fPIC `root-config --ldflags` $(CPPFLAGS) -I$(ROOTSYS)/include $^

EXAMPLE		= ${TARGET}.o

examples: ${OBJS} ${EXAMPLE}
	${CXX} ${CXXFLAGS}  -o ${TARGET} ${EXAMPLE} ${HDRS} ${OBJS} $(LIBS) $(GLIBS) $(LDFLAGS)

%.${SUFFIX}.o: %.cc
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm -f *core ${OBJS} *.o ${TARGET}

TAGS: $(SRCS) $(HDRS)
	etags -t $(SRCS) $(HDRS)



