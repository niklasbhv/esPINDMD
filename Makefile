# Makefile to run clang-format recursively
FORMAT_EXTENSIONS = c cpp h hpp

# Find all relevant source files
SRC_FILES := $(shell find . -type f \( \
  $(foreach ext,$(FORMAT_EXTENSIONS), -name '*.$(ext)' -o) -false \))

.PHONY: format

format:
	@echo "Formatting source files..."
	@clang-format -i -style=file $(SRC_FILES)
	@echo "Done."
clean:
	rm -rf ./.pio
documentation:
	doxygen Doxyfile
