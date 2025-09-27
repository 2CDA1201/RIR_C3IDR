CXX = g++
CXXFLAGS = -O2 -I include
DEBUGFLAGS = -g -Og -I include

SRC = main.cpp src/buffered_reader.cpp src/ip_utils.cpp

OUTDIR = build/Run
DEBUGDIR = build/Debug
OUT = $(OUTDIR)/main
DEBUGOUT = $(DEBUGDIR)/main

all: $(OUT)

run: $(OUT)
	./$(OUT)

$(OUT): $(SRC)
	mkdir -p $(OUTDIR)
	mkdir -p cidr
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

debug: $(DEBUGOUT)
	./$(DEBUGOUT)

$(DEBUGOUT): $(SRC)
	mkdir -p $(DEBUGDIR)
	mkdir -p cidr
	$(CXX) $(DEBUGFLAGS) $(SRC) -o $(DEBUGOUT)

clean:
	-rm -f $(OUT) 2> NUL || true
	-rm -f $(DEBUGOUT) 2> NUL || true
	-rm -rf cidr 2> NUL || true
