# Makefile for applying clang-format to C++ files with directory exclusion support

# List of directories to exclude (space-separated)
EXCLUDE_DIRS := .github .pio .vscode doc

# File extensions to target
EXTENSIONS := *.cpp *.cc *.h *.hpp *.cxx *.hh *.c

# Name of the formatter
CLANG_FORMAT := clang-format

# Find all source files excluding specified directories
CPP_FILES := $(shell find . -type f \( $(foreach ext,$(EXTENSIONS), -name "$(ext)" -o) -false \) $(foreach dir,$(EXCLUDE_DIRS), -not -path "./$(dir)/*"))

# Format target
.PHONY: format

build:
	pio run

flash:
	pio run --target upload

clean:
	pio run --target clean

format:
	@echo "Applying clang-format to source files..."
	@$(foreach file, $(CPP_FILES), \
		echo "Formatting $(file)" && $(CLANG_FORMAT) -i $(file);)

documentation:
	doxygen Doxyfile
