all: build

CPLEXDIR      = solvers/cplex/cplex
CPLEXLIBDIR   = solvers/cplex/cplex/lib/x86-64_linux/static_pic
CONCERTDIR    = solvers/cplex/concert
CONCERTLIBDIR = solvers/cplex/concert/lib/x86-64_linux/static_pic

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
CCOPT         = -fno-strict-aliasing -fexceptions -DIL_STD
MIP_LNDIRS    = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)
MIP_LNFLAGS   = -lconcert -lilocplex -lcplex -lm -lpthread

build: $(MIPDEP) $(patsubst source/%.cpp, objects/%.o, $(wildcard source/*.cpp))
	g++ $(wildcard objects/*.o) \
		$(LMHS_CPPFLAGS) $(OBJFILES) $(LIB_OBJFILES) \
		$(MIP_LNDIRS) $(MIP_LNFLAGS) $(MIP_FLAGS) -O3 -o bcause

objects/%.o: source/%.cpp
	mkdir -p objects; g++ -I include -c -o $@ $< $(MIP_FLAGS) $(CCOPT) \
	    -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) -DMIP_CPLEX -std=c++0x -O3\
	    -Wfatal-errors -pedantic -Wall -Wextra

clean:
	rm -f objects/*.o
