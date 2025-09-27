CXX = g++
CXXFLAGS = -O2 -I include
DEBUGFLAGS = -g -Og -I include

SRC = main.cpp src/buffered_reader.cpp src/ip_utils.cpp
OUT = build\Run\main.exe
DEBUGOUT = build\Debug\main.exe

all: $(OUT)

run: $(OUT)
	.\$(OUT)

$(OUT): $(SRC)
	-@cmd /c if not exist "build\Run" mkdir "build\Run"
	-@cmd /c if not exist "cidr" mkdir "cidr"
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

debug: $(DEBUGOUT)
	.\$(DEBUGOUT)

$(DEBUGOUT): $(SRC)
	-@cmd /c if not exist "build\Debug" mkdir "build\Debug"
	-@cmd /c if not exist "cidr" mkdir "cidr"
	$(CXX) $(DEBUGFLAGS) $(SRC) -o $(DEBUGOUT)

clean:
	-@cmd /c if exist "$(OUT)" del /F /Q "$(OUT)"
	-@cmd /c if exist "$(DEBUGOUT)" del /F /Q "$(DEBUGOUT)"
	-@cmd /c if exist "cidr" rd /S /Q "cidr"
