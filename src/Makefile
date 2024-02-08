# Option nodebug=1 ignores all asserts
# If a debug version is made (e.g. "make"), run "make clean"
#   before making a nodebug version (e.g. "make nodebug=1").
# The same applies in the opposite case.


mhs_wcsp: mhs_wcsp.o wcsp.o function.o MHV_cpx.o csp_sat.o wcsp_solver.o
	$(CCC) $(CCFLAGS) -o mhs_wcsp mhs_wcsp.o wcsp.o function.o MHV_cpx.o csp_sat.o wcsp_solver.o $(LIBCADICAL) $(CCLNFLAGS)

mhs_wcsp.o: mhs_wcsp.cc config.hh wcsp.hh function.hh MHV_cpx.hh wcsp_solver.hh utils.cc
	$(CCC) $(CCFLAGS) -c mhs_wcsp.cc

wcsp_solver.o: wcsp_solver.hh wcsp_solver.cc
	$(CCC) $(CCFLAGS) -c wcsp_solver.cc

wcsp.o: wcsp.hh wcsp.cc
	$(CCC) $(CCFLAGS) -c wcsp.cc

function.o: function.hh function.cc
	$(CCC) $(CCFLAGS) -c function.cc

MHV_cpx.o: MHV_cpx.cc MHV_cpx.hh
	$(CCC) $(CCFLAGS) -c MHV_cpx.cc


CADICAL = sat-cadical
LIBCADICAL = $(CADICAL)/build/libcadical.a

csp_sat.o: csp_sat.hh csp_sat.cc $(LIBCADICAL) csp.hh
	$(CCC) $(CCFLAGS) -c csp_sat.cc

$(LIBCADICAL): $(CADICAL)/src/*.hpp $(CADICAL)/src/*.cpp $(CADICAL)/src/
	cd $(CADICAL); pwd; ./configure && make

clean:
	rm -f  *.o mhs_wcsp

clean_cadical:
	cd $(CADICAL); make clean

################################

SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic
CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio201/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio201/concert
BOOSTDIR   = /usr/local

################

CCC = g++
LDFLAGS = #-g

ifdef nodebug
	DEBUGFLAG = -DNDEBUG
else
	DEBUGFLAG =
endif

#####

CCOPT = -std=c++11 -m64 -O3 -fPIC -fno-strict-aliasing -fexceptions  -DIL_STD

CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
BOOSTLIBDIR   = $(BOOSTDIR)/lib

CCLNFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -pthread -ldl

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
BOOSTINCDIR   = $(BOOSTDIR)/include

CCFLAGS = $(DEBUGFLAG) $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) -I$(BOOSTINCDIR)
