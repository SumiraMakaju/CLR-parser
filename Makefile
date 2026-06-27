CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
RFLAGS   = -O2 -DNDEBUG

SRC_MAIN = main/main.cpp
SRC_LIB  = src/grammar.cpp \
            src/lr1.cpp     \
            src/parser.cpp

SRC_ALL  = $(SRC_MAIN) $(SRC_LIB)

TARGET   = build/lr1_parser.exe

all: $(TARGET)

$(TARGET): $(SRC_ALL) | build
	$(CXX) $(CXXFLAGS) $(SRC_ALL) -o $(TARGET)

release: $(SRC_ALL) | build
	$(CXX) $(CXXFLAGS) $(RFLAGS) $(SRC_ALL) -o $(TARGET)

build:
	if not exist build mkdir build

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

trace: $(TARGET)
	.\$(TARGET) --trace "id = * id"

json: $(TARGET)
	.\$(TARGET) --json --quiet "id = * id"

clean:
	if exist build rmdir /s /q build
	if exist lr1_data.json del lr1_data.json

.PHONY: all release run test report trace json clean