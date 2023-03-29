# https://makefiletutorial.com/


TARGET_EXEC := main
BUILD_DIR := ./build
SRC_DIRS := ./

#INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS := . ../../src/ ../../vendor/
ifeq ($(OS),Windows_NT)
	INC_FLAGS := $(addprefix -I,$(INC_DIRS))
	CFLAGS := $(INC_FLAGS) -g -MMD -MP
	LDFLAGS := -lws2_32 -lgdi32 -lWinmm
else
	INC_FLAGS := $(addprefix -I,$(INC_DIRS))
	CFLAGS := $(INC_FLAGS) -g -MMD -MP
	LDFLAGS := 
endif


SRCS := main.c


OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)


$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

all: $(BUILD_DIR)/$(TARGET_EXEC)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)