CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
RFLAGS   = -O2 -DNDEBUG

SRC_MAIN = main/main.cpp
SRC_LIB  = src/grammar.cpp \
            src/lr1.cpp     \
            src/parser.cpp  \
            src/server.cpp

SRC_ALL  = $(SRC_MAIN) $(SRC_LIB)
TARGET   = build/lr1_parser.exe

all: $(TARGET)

$(TARGET): $(SRC_ALL) | build
	$(CXX) $(CXXFLAGS) $(SRC_ALL) -o $(TARGET) -lws2_32

release: $(SRC_ALL) | build
	$(CXX) $(CXXFLAGS) $(RFLAGS) $(SRC_ALL) -o $(TARGET) -lws2_32

build:
	if not exist build mkdir build

# Generate visualization/js/data.js from a parse run (default string)
generate: $(TARGET)
	.\$(TARGET) --write-js "id = * id"

# Generate with a custom string: mingw32-make generate-str STR="* * id"
generate-str: $(TARGET)
	.\$(TARGET) --write-js "$(STR)"

# Start HTTP server (serves visualizer + handles /parse requests live)
serve: $(TARGET)
	.\$(TARGET) --serve

serve-port: $(TARGET)
	.\$(TARGET) --serve 8080

run: $(TARGET)
	.\$(TARGET) "id = * id"

test: $(TARGET)
	@echo --- id = * id   [expect ACCEPTED] ---
	@.\$(TARGET) -q "id = * id"     && echo PASS || echo FAIL
	@echo --- id          [expect ACCEPTED] ---
	@.\$(TARGET) -q "id"            && echo PASS || echo FAIL
	@echo --- * id        [expect ACCEPTED] ---
	@.\$(TARGET) -q "* id"          && echo PASS || echo FAIL
	@echo --- id = id     [expect ACCEPTED] ---
	@.\$(TARGET) -q "id = id"       && echo PASS || echo FAIL
	@echo --- * * id      [expect ACCEPTED] ---
	@.\$(TARGET) -q "* * id"        && echo PASS || echo FAIL
	@echo --- id = * * id [expect ACCEPTED] ---
	@.\$(TARGET) -q "id = * * id"   && echo PASS || echo FAIL
	@echo --- id = x      [expect REJECTED] ---
	@.\$(TARGET) -q "id = x" 2>NUL && echo FAIL || echo PASS

report: $(TARGET)
	.\$(TARGET) --report "id = * id"

clean:
	if exist build rmdir /s /q build
	if exist lr1_data.json del lr1_data.json

.PHONY: all release generate generate-str serve serve-port run test report clean