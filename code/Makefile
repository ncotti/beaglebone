# Author: Nicolas Gabriel Cotti (ngcotti@gmail.com)

#------------------------------------------------------------------------------
# Makefile Initialization
#------------------------------------------------------------------------------
SHELL=/bin/bash
.ONESHELL:
.POSIX:
.EXPORT_ALL_VARIABLES:
.DELETE_ON_ERROR:
.SILENT:
.DEFAULT_GOAL := help

#------------------------------------------------------------------------------
# User modifiable variables
#------------------------------------------------------------------------------
# Which compiler, linker, assembler and binutils to use, e.g.:
# arm-none-eabi, arm-linux-gnueabihf, (left empty), etc
toolchain := arm-linux-gnueabihf

# User specific flags for C compiler (implicit flags: -c)
compiler_flags := -Wall -g -mcpu=cortex-a8 -mfpu=neon -march=armv7-a

# User specific flags for Assembler
assembler_flags := ${compiler_flags}

# Direction to the linker script (can be empty)
linker_script :=

# Auxiliary linker scripts (are included in the main one, but not called by the linker)
aux_linker_scripts :=

# User specific linker flags
linker_flags := -g

# List of header files' directories.
header_dirs := inc

# List of source files' directories.
source_dirs := src

# Name of the final executable (without extension)
executable_name := exe

# Name of the gdb script (can be empty)
#gdb_script := sup/debug_testing.gdb
gdb_script := sup/debug.gdb

# Name of the doxygen script (can be empty)
doxygen_file :=

#------------------------------------------------------------------------------
# Binutils
#------------------------------------------------------------------------------
cc 			:= ${toolchain}-gcc
as 			:= ${toolchain}-gcc
linker 		:= ${toolchain}-gcc
objdump 	:= ${toolchain}-objdump
objcopy 	:= ${toolchain}-objcopy

#------------------------------------------------------------------------------
# File extensions
#------------------------------------------------------------------------------
obj_ext 		:= .o
c_ext 			:= .c
h_ext 			:= .h
asm_ext 		:= .S
elf_ext 		:= .elf
bin_ext 		:= .bin
obj_header_ext 	:= .txt
dasm_ext		:= .lst

#------------------------------------------------------------------------------
# Miscelaneous constants
#------------------------------------------------------------------------------
print_checkmark 	:= printf "\\033[0;32m\\u2714\n\\033[0m"
print_cross 		:= printf "\\u274c\n"

#------------------------------------------------------------------------------
# File location
#------------------------------------------------------------------------------
build_dir	:= bin
info_dir 	:= ../doc
elf_file 	:= ${build_dir}/${executable_name}${elf_ext}
bin_file 	:= ${build_dir}/${executable_name}${bin_ext}
map_file	:= ${build_dir}/${info_dir}/memory.map

# List all C source files as "source_dir/source_file"
define c_source_files !=
	for dir in ${source_dirs}; do
		if ls $${dir}/*${c_ext} &> /dev/null; then
			ls $${dir}/*${c_ext} 2> /dev/null
		fi
	done
endef

# List all assembly source files as "source_dir/source_file"
define asm_source_files !=
	for dir in ${source_dirs}; do
		if ls $${dir}/*${asm_ext} &> /dev/null; then
			ls $${dir}/*${asm_ext} 2> /dev/null
		fi
	done
endef

# List all header files as "header_dir/header_file"
define header_files !=
	for dir in ${header_dirs}; do
		if ls $${dir}/*${h_ext} &> /dev/null; then
			ls $${dir}/*${h_ext} 2> /dev/null
		fi
		if ls $${dir}/*${asm_ext} &> /dev/null; then
			ls $${dir}/*${asm_ext} 2> /dev/null
		fi
	done
endef

# List all object files as "build_dir/source_dir/object_file"
define object_files !=
	# Replace source extension for object extension
	for file in ${c_source_files} ${asm_source_files}; do
		file=$${file//${c_ext}/${obj_ext}}
		file=$${file//${asm_ext}/${obj_ext}}
		echo "${build_dir}/$${file}"
	done
endef

# List all object files' headers as "build_dir/info_dir/object_header_file"
define object_header_files !=
	for file in ${object_files} ${elf_file}; do
		# Add info dir after build dir.
		dst=$${file//"${build_dir}"/"${build_dir}/${info_dir}"}
		# Change extension to object header.
		dst=$${dst//"${obj_ext}"/"${obj_header_ext}"}
		dst=$${dst//"${elf_ext}"/"${obj_header_ext}"}
		echo "$${dst}"
	done
endef

# List all disassemblies as "build_dir/info_dir/dasm_file"
define dasm_files !=
	# Change extension to dasm_ext
	for file in ${object_header_files}; do
		file=$${file//${obj_header_ext}/${dasm_ext}}
		echo "$${file}"
	done
endef

#------------------------------------------------------------------------------
# User targets
#------------------------------------------------------------------------------
.PHONY: compile
compile: ${elf_file} ## Compile all source code, generate ELF file.

.PHONY: help
help: ## Display this message.
	grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-10s\033[0m %s\n", $$1, $$2}'

.PHONY: binary
binary: ${bin_file} ## Generate binary file, without ELF headers.

.PHONY: docs
docs: ${elf_file}	## Generate doxygen documentation as HTML
	if [ -f ${doxygen_file} ]; then
		if ! man doxygen &>/dev/null; then
			echo -n "Failed to create docs"
			${print_cross}
			echo "  Please, install \"doxygen\" to generate the documentation with:"
			echo "    sudo apt install doxygen"
		elif ! man dot &>/dev/null; then
			echo -n "Failed to create docs"
			${print_cross}
			echo "  Please, install \"dot\" to generate the documentation with:"
			echo "    sudo apt install graphviz"
		else
			doxygen ${doxygen_file}
			echo -n "Documentation generated correctly."
			${print_checkmark}
			open doc/doxygen/html/index.html 1>/dev/null
		fi
	else
		echo -n "Failed to create docs. Couldn't find ${doxygen_file}."
		${print_cross}
	fi

.PHONY: headers
headers: ${object_header_files} ## Generate symbol table and section headers for all object files.

.PHONY: dasm
dasm: ${dasm_files} ## Generate disassemble for all object files and elf file.

.PHONY: clean
clean: ## Erase contents of build directory.
	if [ -d $${build_dir} ]; then
		if [ -d $${build_dir}/$${info_dir} ]; then
			rm -R $${build_dir}/$${info_dir}
		fi
		rm -R $${build_dir}
		echo -n "All files successfully erased "
		${print_checkmark}
	else
		echo -n "Nothing to erase "
		${print_cross}
	fi

.PHONY: clear
clear: clean ## Same as clean.

.PHONY: run
run: ${bin_file} ## Execute compiled program (using QEMU)
	# echo -n "Initiating Qemu... "
	# coproc qemu-system-arm \
	# 	-M realview-pb-a8 \
	# 	-m 512M \
	# 	-no-reboot -nographic \
	# 	-monitor telnet:127.0.0.1:1234,server,nowait \
	# 	-S -gdb tcp::2159 \
	# 	-kernel ${bin_file} &>/dev/null
	# ${print_checkmark}


.PHONY: debug
debug: run ## Debug the program (no need to "make run" first, compile with "-g")
	if [ -n "${gdb_script}" ]; then
		arg_gdb_script="-x ${gdb_script}"
	fi
	sshpass -p temppwd scp ${elf_file} debian@192.168.7.2:/home/debian/exe.elf
	gdb-multiarch -q $${arg_gdb_script} "${elf_file}"

#------------------------------------------------------------------------------
# Compilation targets
#------------------------------------------------------------------------------
# Main executable linking
${elf_file}: ${object_files}
	echo -n "Linking everything together... "
	if [ -n "${linker_script}" ]; then
		script="-T ${linker_script}"
	fi
	mkdir -p ${build_dir}/${info_dir}
	${linker} ${linker_flags} $${script} -o $@ $^
	${print_checkmark}
	echo "Executable file \"$@\" successfully created."

# Compiling individual object files
${build_dir}/%${obj_ext}: %.* ${header_files} Makefile ${linker_script} ${aux_linker_scripts}
	# Create compilation folders if they don't exist
	for dir in ${source_dirs}; do
		mkdir -p ${build_dir}/$${dir}
	done

	# Add "-I" flag in between header direrctories
	include_headers=""
	for dir in ${header_dirs}; do
		include_headers="$${include_headers} -I $${dir}"
	done

	# Actual compiling
	if echo $< | grep "\${c_ext}" &>/dev/null; then
		echo -n "Compiling $< --> $@... "
		${cc} ${compiler_flags} -o $@ -c $${include_headers} $<
	elif echo $< | grep "\${asm_ext}" &>/dev/null; then
		echo -n "Assembling $< --> $@... "
		${as} ${assembler_flags} -o $@ -c $${include_headers} $<
	else
		${print_cross}
		echo "Unrecognized file extension."
		exit 1
	fi
	${print_checkmark}

# Print object files' headers
${build_dir}/${info_dir}/%${obj_header_ext}: ${build_dir}/%.o
	for dir in ${source_dirs}; do
		mkdir -p "${build_dir}/${info_dir}/$${dir}"
	done
	echo -n "Printing $< -> $@... "
	${objdump} -x $< > $@
	${print_checkmark}

# Print elf file's header
${build_dir}/${info_dir}/%${obj_header_ext}: ${build_dir}/%.elf
	echo -n "Printing $< -> $@... "
	${objdump} -x $< > $@
	${print_checkmark}

# Print object files' disassembly
${build_dir}/${info_dir}/%${dasm_ext}: ${build_dir}/%.o
	for dir in ${source_dirs}; do
		mkdir -p "${build_dir}/${info_dir}/$${dir}"
	done
	echo -n "Disassembling $< -> $@... "
	${objdump} -d $< > $@
	${print_checkmark}

# Print elf file disassembly
${build_dir}/${info_dir}/%${dasm_ext}: ${build_dir}/%.elf
	echo -n "Disassembling $< -> $@... "
	${objdump} -d $< > $@
	${print_checkmark}

# Copy ELF file into BIN file
${bin_file}: ${elf_file}
	echo -n "Creating binary file $@... "
	${objcopy} -O binary ${elf_file} ${bin_file}
	${print_checkmark}
