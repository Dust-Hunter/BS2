# =====================================================================
# tests/Makefile - used with Make to run test cases
#
# Author: Prof. Ronald Charles Moore
# Fachbereich Informatik - Dept. of Computer Science
# Hochschule Darmstadt   - University of Applied Sciences
#                          Darmstadt, Germany
#
# This File is part of the RMMIX Assembler/Disassembler Package.
# Version 0.6.2, 29 October 2013
#
# This package has been developed to be used ONLY with the course
# named "Betriebssysteme" (Operating Systems), given by
# Prof. Moore.
#
# No warranty of any kind is given to anyone.
#
# Only students currently enrolled and actively involved in the
# "Betriebssysteme" course are granted permission to work with this
# package, but they are given unlimited permission to do as much or
# as little with this package as they choose, up to but not including
# distribution of the package or any of its contents.
#
# Please report bugs to <ronald.moore@h-da.de>
#
# =====================================================================

# Wichtig - Diese Datei muss "Makefile" gennant werden!

# HINWEIS: Wenn Sie nicht sicher sind, ob diese Datei noch richtig ist,
# rufen sie "make -n" auf - das zeit, was passieren würde, wenn Sie
# "make" aufrufen würden - ohne aber irgendetwas sonst zu tun!
# D.h. "make -n" ist harmlos  - aber informativ.

# In this makefile, we want to keep going even if we find errors
# (we expect to find errors!)
.IGNORE :

# Fuehre nur ein Test auf einmal durch; egal ob "make -j2" aufgerufen wurde
.NOTPARALLEL :

# See https://www.gnu.org/software/make/manual/html_node/Special-Targets.html

# ==== Macros ====

# Testing files - must be provided by developers
SIMPLETESTJOBS = test0.job test0a.job test0b.job test0c.job testErrors.job
BIGTESTJOBS   = test1.job test2a.job test2b.job test3.job test4.job
SIMTESTJOBS   = simtest1.job simtest3.job simtest3tricky.job
TESTJOBS  = $(SIMPLETESTJOBS) $(BIGTESTJOBS) $(SIMTESTJOBS)
TESTREFS  = $(TESTJOBS:.job=.ref)

# Obj files - will be created using the assembler
TESTOBJS  = $(TESTJOBS:.job=.obj)

# Simulator output Files - will be created by running the assembled files
SIMOUTS  = $(BIGTESTJOBS:.job=.simout) $(SIMTESTJOBS:.job=.simout)

# Reference simulator output files - what we expect to see.
SIMREFS = $(BIGTESTJOBS:.job=.simref) $(SIMTESTJOBS:.job=.simref)

# Programs - the assembler and the simulator (emulator)
PROGRAMS = ../rmmixas ../rmmixsim

# Following files should not be deleted, regardless of what errors occur
.PRECIOUS: $(TESTREFS) $(SIMREFS) bigtest.ref

# ==== TARGETS und REGELN ====
# Es ist ganz WICHTIG, dass die Zeile unten, die Befehle beinhalten
# (z. B. gcc... oder g++... oder rm ... ) mit TABULATOR anfangen!

# Tell make that the following "targets" are "phony"
# Cf. https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html#Phony-Targets
.PHONY : all check test updatetests clean testclean

# Regel: "make all" == "make tested"
# Das ist der erste Regel, also ist "make" == "make all"
all: test

# For convenience, "make check" == "make test"
check: test

# TESTING
# Calling "make test" should
# (1) make the programs (if necessary)
# (2) erase all the test outputs
# (3) create them again (see rules, below).
# (4) run the special case "bigtest" (see below)
test:
	$(MAKE) clean
	$(MAKE) updatetests

updatetests: $(PROGRAMS) $(TESTOBJS) bigtest.obj $(SIMOUTS)

# "make clean" or equivalently  "make testclean" deletes all files created by testing
clean: testclean

testclean:
	rm -fv *.obj *~ *.simout rmmix.log

# Die Programme werden hoffentlich schon da sein...
$(PROGRAMS):
	cd .. && $(MAKE) all

# By the way, for more information about calling make from make, see
# https://www.gnu.org/software/make/manual/html_node/Recursion.html#Recursion

# User-defined function (Cf. http://oreilly.com/catalog/make3/book/ch04.pdf)
# Takes two arguments - an output file and a reference output file
# (the two files should be the same). Function  prints "success" or "failure",
# depending on whether the two files are equal.
define testReferenceOutput
    @cmp -s $1 $2                                        \
    && /bin/echo -e "Test against $2 successful" \
    || /bin/echo -e "\n\tTest against $2 FAILED!!!\n"
endef

#### If your terminal supports colors, comment out the version above,
# and use this version instead - good tests have a green background,
# failed tests have a red background. See
#    <http://misc.flogisoft.com/bash/tip_colors_and_formatting>
# for information about output formatting (colors, bold, etc.)).
#
#define testReferenceOutput
#    @cmp -s $1 $2                                        \
#    && /bin/echo -e "\e[1;42mTest against $2 successful.\e[0m" \
#    || /bin/echo -e "\e[1;41mTest against $2 FAILED!!!  \e[0m"
#endef

#      "Variables", die hier benutzt werden:
#      Vgl. http://www.makelinux.net/make3/make3-CHP-2-SECT-2.html
# $@ = The filename representing the target.
# $< = The filename of the first prerequisite.
# $* = The stem of the target (i.e. without .o, .cpp...)
# $(*F) = The stem of the target (i.e. without .o, .cpp... AND without directory)
# $^ = The names of all the prerequisites, with spaces between them.

# Hier geben wir an, welche Objekt-Dateien von welcher Header-Dateien abhängen.
# Hinweis - Ganz und gar automatisch!
# Vgl. GNU Make Manual, Chapter 4.14
# bzw. http://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
# Siehe auch: http://www.makelinux.net/make3/make3-CHP-2-SECT-7.html

################ Assembler Tests ###################
# For some files x.job, we have stored the "correct" (expected) output in x.ref
# (where "ref" is short for "reference").
$(TESTOBJS): %.obj: %.job %.ref
	../rmmixas $< >$@
	$(call testReferenceOutput,$@, $*.ref)

# Now, test the assembler with several arguments
bigtest.obj: $(BIGTESTJOBS)
	../rmmixas $^ >$@
	$(call testReferenceOutput, bigtest.obj, bigtest.ref)

################ Simulator Tests ###################
# Run each obj file through the assembler and then the simulator
# Check the output against the reference output.
# By the way:
# test1.obj - test4.obj should return status 0.
# simtest*.obj should NOT return status 0!
$(SIMOUTS): %.simout: %.obj %.simref
	../rmmixsim $< > $@   2>&1
	$(call testReferenceOutput,$@, $*.simref)

# Fertig!
