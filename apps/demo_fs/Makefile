# https://makefiletutorial.com/


TARGET_EXEC := main
BUILD_DIR := ./build
SRC_DIRS := ./

#INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS := . ../../src/ ../../vendor/
ifeq ($(OS),Windows_NT)
	INC_FLAGS := $(addprefix -I,$(INC_DIRS))
	CFLAGS := $(INC_FLAGS) -g -MMD -MP -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
	LDFLAGS := -lws2_32
else
	INC_FLAGS := $(addprefix -I,$(INC_DIRS))
	CFLAGS := $(INC_FLAGS) -g -MMD -MP
	LDFLAGS := 
endif


SRCS := main.c
SRCS += ../../src/eg_dirwatch_win32.c
SRCS += ../../src/EgQuantities.c
SRCS += ../../src/EgFs.c
SRCS += ../../vendor/flecs.c


OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)


#$(BUILD_DIR)/../../vendor/flecs.c.o:

$(BUILD_DIR)/../../vendor/flecs.c.o: CFLAGS += -Wno-error=unknown-pragmas -Wno-error=incompatible-pointer-types -Wno-error=cast-function-type -Wno-error=sign-compare

# The final build step.
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