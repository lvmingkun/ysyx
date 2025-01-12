CFLAGS = --cc --exe --build -j 4 -Wall

SIM_CPP = ./csrc/*.cpp
TOP_V = ./vsrc/*.v
BUILD_DIR = ./obj_dir
EXECUTABLE = $(BUILD_DIR)/Vtop

all: sim
	@echo "Write this Makefile by your self."

sim: $(EXECUTABLE)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@echo "Running simulation..."
	$(EXECUTABLE)

$(EXECUTABLE): $(SIM_CPP) $(TOP_V)
	@echo "Building simulation exexcutable..."
	@$(VERILATOP) $(CFLAGS) $(SIM_CPP) $(TOP_V) 

clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

include ../Makefile
