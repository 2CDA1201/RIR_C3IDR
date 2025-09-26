CXX = g++
CXXFLAGS = -O2 -I include
SRC = main.cpp src/ultra_buffered_reader.cpp src/ip_utils.cpp
OUT = build/Run/main.exe

all: $(OUT)

$(OUT): $(SRC)
	-@cmd /c if not exist "build\Run" mkdir "build\Run"
	-@cmd /c if not exist "cidr" mkdir "cidr"
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	-@cmd /c if exist "$(OUT)" del /F /Q "$(OUT)"
	-@cmd /c if exist "cidr" rd /S /Q "cidr"

run: $(OUT)
	.\$(OUT)
