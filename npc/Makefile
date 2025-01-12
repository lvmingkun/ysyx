
VERILATOR = verilator
VERILATOR_CFLAGS = --build --cc --exe -j 4 -Wall --trace  --top-module top

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
	$(VERILATOR) $(VERILATOR_CFLAGS) $(SIM_CPP) $(TOP_V) 

wave:
	gtkwave wave.fst

clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)
	rm *.fst

include ../Makefile
