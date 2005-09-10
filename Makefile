
# Make command to use for dependencies
MAKECMD=make

CFG=arm

#
# Configuration: arm 
#
ifeq "$(CFG)" "arm"
OUTDIR=bin
OUTFILE=cmucam3.elf
CFG_INC=
//CFG_LIB=-L "/cygdrive/c/Program Files/gnuarm/arm-elf/lib/" -lm -lc -lgcc 
//CFG_LIB=-L "/cygdrive/c/Program Files/gnuarm/lib/gcc/arm-elf/3.4.3/" -L "/cygdrive/c/Program Files/gnuarm/arm-elf/lib/" -lm -lc -lgcc 
CFG_LIB=-L "/cygdrive/c/Program Files/GNUARM/lib/gcc/arm-elf/4.0.0/" -L "/cygdrive/c/Program Files/gnuarm/arm-elf/lib/" -lm -lc -lgcc 
CFG_OBJ=
COMMON_OBJ=$(OUTDIR)/startup.o  $(OUTDIR)/main.o $(OUTDIR)/cc3.o  $(OUTDIR)/serial.o $(OUTDIR)/libc.o $(OUTDIR)/LPC2100.o $(OUTDIR)/interrupt.o
OBJ=$(COMMON_OBJ) $(CFG_OBJ)

OBJCPY=arm-elf-objcopy -g -v -O ihex "$(OUTDIR)/cmucam3.elf" "$(OUTDIR)/cmucam3.hex"
COMPILE_SX=arm-elf-gcc -O3 -dAP -save-temps -c -g -o "$(OUTDIR)/$(*F).o" $(CFG_INC) $<
COMPILE=arm-elf-gcc -c -o "$(OUTDIR)/$(*F).obj" $(CFG_INC) "$<"
LINK=arm-elf-ld  -g -o "$(OUTDIR)/cmucam3.elf" $(OBJ) $(CFG_LIB) -T ./arm.ln 

# Pattern rules
$(OUTDIR)/%.o : src/%.sx
	$(COMPILE_SX)

$(OUTDIR)/%.o : src/%.c
	$(COMPILE_SX)

$(OUTDIR)/%.o : %.i
	$(COMPILE_SX)

$(OUTDIR)/%.o : %.ii
	$(COMPILE_SX)

$(OUTDIR)/%.o : %.m
	$(COMPILE_SX)

$(OUTDIR)/%.o : %.cc
	$(COMPILE_SX)

$(OUTDIR)/%.o : %.cxx
	$(COMPILE_SX)

$(OUTDIR)/%.o : src/%.s
	$(COMPILE_SX)

endif

# Build rules
all: $(OUTDIR)/$(OUTFILE)
$(OUTDIR)/$(OUTFILE): $(OUTDIR)  $(OBJ)
	$(LINK) 
	$(OBJCPY)

$(OUTDIR):
	mkdir -p "$(OUTDIR)"

# Clean this project
clean:
	rm -f $(OUTDIR)/$(OUTFILE)
	rm -f $(OBJ)
	rm -f $(OUTDIR)/*.hex
	rm -f *.s
	rm -f *.i

# Clean this project and all dependencies
cleanall: clean
