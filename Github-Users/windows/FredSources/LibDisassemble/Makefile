#################################################
# Selection du mode de compilation
#################################################
#MODE=Debug
#MODE=Release
#ARCH=x86
#ARCH=x86_64

#################################################
# Directory listing
#################################################
# includes directories
INC_DIR = ./Include ./Ressource
# sources directories
SRC_DIR = ./Source
# output directory
OUT_DIR= ../

#################################################
# Compilation
#################################################
# Compiler definition
CC=gcc
CPP=g++
ASM=nasm
# Archive .a creator
AR=ar
# Additionnal OPTION for compiler
OPTION_CPP=-UWIN32 -UWIN64
OPTION_C=-UWIN32 -UWIN64
OPTION_ASM=-UWIN32 -UWIN64

#################################################
# Outils (depend de l'installation de la station)
#################################################
FIND=$(shell which find)
BASENAME=$(shell which basename)
PWD=$(shell which pwd)
SED=$(shell which sed)
SORT=$(shell which sort)
DIRNAME=$(shell which dirname)
XARGS=$(shell which xargs)
RM=$(shell which rm)
MV=$(shell which mv)
MKDIR=$(shell which mkdir)
ECHO=$(shell which echo)

#################################################
# Compilation mode
#################################################
MODE_DEBUG=Debug
MODE_RELEASE=Release
ARCH_x86=x86
ARCH_x86_64=x86_64

#################################################
# Extention config
#################################################
LIB_DYN_EXT=so
LIB_STC_EXT=a
OBJ_EXT=o
EXE_EXT=x
CPP_EXT=cpp
C_EXT=c
ASM_EXT=asm
H_EXT=h

#################################################
# LIBRARY name
#################################################
LIBRARY_DIR = $(shell $(BASENAME) "`$(PWD)`" | $(SED) 's/\(.*\)/\1/g' )
LIBRARY_BASENAME = $(LIBRARY_DIR)
EXE_BASENAME = $(LIBRARY_DIR)

#################################################
# Compilation OPTION
#################################################
# Option de compilation Release / DEBUG 
ACTIVATION_TRACE=-D_DEBUG
MODE_UTILISE=$(shell [ "$(MODE)" = "$(MODE_DEBUG)" ] && $(ECHO) "$(MODE_DEBUG)" || $(ECHO) "$(MODE_RELEASE)" )
OPTION_DEBUG=$(shell [ "$(MODE)" = "$(MODE_DEBUG)" ] && $(ECHO) "-g $(ACTIVATION_TRACE)" )
OPTION_DEBUG_ASM=$(shell [ "$(MODE)" = "$(MODE_DEBUG)" ] && $(ECHO) "-g $(ACTIVATION_TRACE)" )

# Mode de compilation
TRUE_ARCH = $(shell [ "$(ARCH)" = "" ] &&   $(ECHO) "`uname -m`" || $(ECHO) "$(ARCH)"  )
ARCHITECTURE_32=$(shell [ "`uname -m`" = "$(ARCH_x86_64)" ] && $(ECHO) "-m32" || $(ECHO) "")
ARCHITECTURE_64=-m64
ARCHITECTURE=$(shell [ "$(TRUE_ARCH)" = "$(ARCH_x86_64)" ] && $(ECHO) "$(ARCHITECTURE_64)" || $(ECHO) "$(ARCHITECTURE_32)"  )
DEFINEARCH= $(shell [ "$(TRUE_ARCH)" = "$(ARCH_x86_64)" ] && $(ECHO) "-DLIN64" || $(ECHO) "-DLIN32"  )
ARCHITECTURE_ASM=$(shell [ "$(TRUE_ARCH)" = "$(ARCH_x86_64)" ] && $(ECHO) "-f elf64" || $(ECHO) "-f elf32"  )

# Répertoire de sortie
OUTPUT_DIRECTORY=$(OUT_DIR)/$(MODE_UTILISE)/$(TRUE_ARCH)

# Option de compilation
OPTION_LINK=$(OPTION_DEBUG) $(ARCHITECTURE)
OPTION_FPIC = $(shell [ "`uname`" = "SunOS" -o "`uname`" = "Linux" ] && $(ECHO) "-fPIC" )
OPTION_CPP+=$(OPTION_DEBUG) -Wall $(DEFINEARCH) $(ARCHITECTURE) $(OPTION_FPIC) $(DEFINE)
OPTION_C+=$(OPTION_DEBUG) -Wall $(DEFINEARCH) $(ARCHITECTURE)
OPTION_ASM= -Wall -g $(DEFINEARCH) $(ARCHITECTURE_ASM)

# Find Include DIR
INCLUDE=$(shell $(FIND) $(INC_DIR) -type f -name "*.$(H_EXT)" -exec $(DIRNAME) {} \; | $(SORT) -u | $(SED) 's/\(.*\)/-I\1/g' | $(XARGS) )

# Find C/C++ files for program
CPP_FILES=$(shell $(FIND) $(SRC_DIR) -type f -name "*.$(CPP_EXT)" | $(XARGS) )
ASM_FILES=$(shell $(FIND) $(SRC_DIR) -type f -name "*.$(ASM_EXT)" | $(XARGS) )
C_FILES=$(shell $(FIND) $(SRC_DIR) -type f -name "*.$(C_EXT)" | $(XARGS) )

# Compute OBJ files for program
OBJ_FILES_ASM=$(patsubst %.$(ASM_EXT),%.$(OBJ_EXT),$(ASM_FILES)) 
OBJ_FILES_C=$(patsubst  %.$(C_EXT),%.$(OBJ_EXT),$(C_FILES)) 
OBJ_FILES_CPP=$(patsubst %.$(CPP_EXT),%.$(OBJ_EXT),$(CPP_FILES)) 

# Move OBJ files to $(MODE_UTILISE)
OBJ_FILES=$(patsubst %.$(OBJ_EXT),$(OUTPUT_DIRECTORY)/%.$(OBJ_EXT),$(OBJ_FILES_ASM) $(OBJ_FILES_C) $(OBJ_FILES_CPP)) 

#################################################
# Generic rules
#################################################
# Que la lib static
all : MAKE_DEST_DIR $(OUTPUT_DIRECTORY)/$(LIBRARY_BASENAME).$(LIB_STC_EXT) 

#Que la lib dynamique
#all : MAKE_DEST_DIR $(OUTPUT_DIRECTORY)/$(LIBRARY_BASENAME).$(LIB_DYN_EXT) 

# Les deux lib
#all : MAKE_DEST_DIR $(OUTPUT_DIRECTORY)/$(LIBRARY_BASENAME).$(LIB_DYN_EXT) $(OUTPUT_DIRECTORY)/$(LIBRARY_BASENAME).$(LIB_STC_EXT) 

#Que l'executable
#all : MAKE_DEST_DIR  $(OUTPUT_DIRECTORY)/$(EXE_BASENAME).$(EXE_EXT)

#################################################
# Library compilation
#################################################

$(OUTPUT_DIRECTORY)/$(LIBRARY_BASENAME).$(LIB_DYN_EXT) :  $(OBJ_FILES)
	@$(ECHO) "CREATE DYNAMIC LIBRARY $@ en $(MODE_UTILISE)"
	@$(CC) -shared -o $@ $(OBJ_FILES) $(OPTION_LINK)

$(OUTPUT_DIRECTORY)/$(LIBRARY_BASENAME).$(LIB_STC_EXT) :  $(OBJ_FILES)
	@$(ECHO) "CREATE STATIC LIBRARY $@ en $(MODE_UTILISE) "
	@$(AR) cr $@ $(OBJ_FILES) 

#################################################
# Execute compilation
#################################################

$(OUTPUT_DIRECTORY)/$(EXE_BASENAME).$(EXE_EXT) :  $(OBJ_FILES)
	@$(ECHO) "CREATE APPLICATION $@ en $(MODE_UTILISE) "
	@$(CC) -o $@ $(OBJ_FILES) $(OPTION_LINK)

#################################################
# Compilation tools
#################################################
MAKE_DEST_DIR :
	@[ ! -f $(MODE_UTILISE) ] || $(RM) -fr $(OUTPUT_DIRECTORY) ;
	@[ -d $(MODE_UTILISE) ] || $(ECHO) "CREATE OUTPUT DIRECTORY : $(MODE_UTILISE)" ; $(MKDIR) -p $(OUTPUT_DIRECTORY)
	
#################################################
# Module compilation
#################################################

$(OUTPUT_DIRECTORY)/%.$(OBJ_EXT): %.$(CPP_EXT)
	@$(ECHO) "COMPILATION $< en $(MODE_UTILISE) "
	@[ -d `$(DIRNAME) $@` ] || $(MKDIR) -p `$(DIRNAME) $@`
	@$(CPP) $(OPTION_CPP) -c -o $@ $< $(INCLUDE)

$(OUTPUT_DIRECTORY)/%.$(OBJ_EXT): %.$(C_EXT)
	@$(ECHO) "COMPILATION $< en $(MODE_UTILISE)"
	@[ -d `$(DIRNAME) $@` ] || $(MKDIR) -p `$(DIRNAME) $@`
	@$(CC) $(OPTION_C) -c -o $@ $< $(INCLUDE)

$(OUTPUT_DIRECTORY)/%.$(OBJ_EXT): %.$(ASM_EXT)
	@$(ECHO) "COMPILATION $< en $(MODE_UTILISE)"
	@[ -d `$(DIRNAME) $@` ] || $(MKDIR) -p `$(DIRNAME) $@`
	@$(ASM) -X gnu -O0 $(OPTION_ASM) -o $@ $< $(INCLUDE)
	
#################################################
# Clean rules
#################################################
clean:
	@$(RM) -f $(OUTPUT_DIRECTORY)/$(LIBRARY_BASENAME).$(LIB_DYN_EXT)
	@$(RM) -f $(OUTPUT_DIRECTORY)/$(LIBRARY_BASENAME).$(LIB_STC_EXT)
	@$(FIND) $(OUTPUT_DIRECTORY) -type f -name "*.$(OBJ_EXT)" | $(XARGS) $(RM) -f 

#################################################
# TEST du makefile
#################################################
DEBUG_MAKEFILE :
	@$(ECHO) "#################################################"
	@$(ECHO) "LIBRARY_BASENAME = $(LIBRARY_BASENAME)"
	@$(ECHO) "EXE_BASENAME     = $(EXE_BASENAME)"
	@$(ECHO) "LIBRARY_DIR      = $(LIBRARY_DIR)"
	@$(ECHO) "OUTPUT_DIRECTORY = $(OUTPUT_DIRECTORY)"
	@$(ECHO) #################################################
	@$(ECHO) "ARCHITECTURE     = $(ARCHITECTURE)"
	@$(ECHO) "MODE_UTILISE     = $(MODE_UTILISE)"
	@$(ECHO) #################################################
	@$(ECHO) "OPTION_LINK      = $(OPTION_LINK)"
	@$(ECHO) "OPTION_CPP       = $(OPTION_CPP)"
	@$(ECHO) "OPTION_C         = $(OPTION_C)"
	@$(ECHO) #################################################
	@$(ECHO) "SRC_DIR          = $(SRC_DIR)"
	@$(ECHO) "INCLUDE          = $(INCLUDE)"
	@$(ECHO) #################################################
	@$(ECHO) "ASM_FILES        = $(ASM_FILES)"
	@$(ECHO) "C_FILES          = $(C_FILES)"
	@$(ECHO) "CPP_FILES        = $(CPP_FILES)"
	@$(ECHO) #################################################
	@$(ECHO) "OBJ_FILES_ASM    = $(OBJ_FILES_ASM)"
	@$(ECHO) "OBJ_FILES_C      = $(OBJ_FILES_C)"
	@$(ECHO) "OBJ_FILES_CPP    = $(OBJ_FILES_CPP)"
	@$(ECHO) #################################################
	@$(ECHO) "OBJ_FILES        = $(OBJ_FILES)"
	@$(ECHO) #################################################
