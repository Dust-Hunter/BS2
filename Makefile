# =====================================================================
# Makefile - used with Make to build three programs.
#
# Author: Prof. Ronald Charles Moore
# Fachbereich Informatik - Dept. of Computer Science
# Hochschule Darmstadt   - University of Applied Sciences
#                          Darmstadt, Germany
#
# This File is part of the RMMIX Assembler/Simulator Package.
# Version 0.6.1, October 2013
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
# rufen sie "make -n" auf - das zeigt, was passieren würde, wenn Sie
# "make" aufrufen würden - ohne aber irgendetwas sonst zu tun!
# D.h. "make -n" ist harmlos  - aber informativ.

# ==== Macros ====

# Hier sind die Bibliotheke, die ganz am Ende gelinkt werden mussen
LIBS =

# Hier sind die Namen der Programmen, die wir bauen wollen
TARGETS = rmmixas rmmixsim unitTester
TARGETOBJS = rmmixas.o rmmixsim.o unitTester.o

# Alle Quellcode-Dateien - ausser die, wo "main" vorkommt...
CPPFILES  = RMMIXJobLang.cpp RMMIXinstruction.cpp \
            rmmixHardware.cpp rmminixos.cpp

# Fuer jede Quell-Datei soll es eine .d-Datei geben (die der Compiler erzeugen wird)
# Die .d-Dateien geben die Abhängigkeiten an (automatisch!)
OBJS = $(CPPFILES:.cpp=.o)
DEPS = $(OBJS:.o=.d) $(TARGETOBJS:.o=.d)

# Die Name des Compilers (g++, gcc, cc, oder so was)
# Uncomment one of the next two lines to choose your C++ compilert
# CC = g++
CC = clang++

#  Compilerflags
#  --------------
# -g  		- erzeuge Debugging info für den debugger
# -Wall 	- alle Warnungen, bitte
# -fmessage-length=0 - Warnungen bzw Fehlermeldungen nicht über mehrere Zeile teilen
#                      - für Eclipse
# -MMD          - erzeuge *.d Dateien, die später von Make importiert werden.
#                               vgl. http://mad-scientist.net/make/autodep.html
#             (die Version hier ist viel einfacher, und daher u.U. nur mit
#              gnu make und { g++ oder clag++ } kompatibel).
FLAGS = -g -std=c++11 -Wall -MMD -fmessage-length=0

# Tell make that the following "targets" are "phony"
# Cf. https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html#Phony-Targets
.PHONY : all clean test check

# ==== TARGETS und REGELN ====
# Es ist ganz WICHTIG, dass die Zeile, die Befehle beinhalten
# (z. B. gcc... oder g++... oder rm ... ), mit TABULATOR anfangen!

# Regel: "all" hängt von TARGETS ab.
#        D.h. "make all" == "make <was auch immer TARGETS ist>
# Das ist der erste Regel, also ist "make" == "make all"
# If you're not using the unitTester, uncomment the next line
# all: $(TARGETS) unitTester
# If you are using the unitTester, uncomment the next line
all: $(TARGETS)

####################  Abhängigkeiten
# Hier geben wir an, welche Objekt-Dateien von welcher Header-Dateien abhängen.
# Hinweis - Ganz und gar automatisch!
# Vgl. GNU Make Manual, Chapter 4.14
# bzw. http://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
# Siehe auch: http://www.makelinux.net/make3/make3-CHP-2-SECT-7.html

#      "Variables", die hier benutzt werden:
#      Vgl. http://www.makelinux.net/make3/make3-CHP-2-SECT-2.html
# $@ = The filename representing the target.
# $< = The filename of the first prerequisite.
# $(*F) = The stem of the filename of the target (i.e. without .o, .cpp...)
# $^ = The names of all the prerequisites, with spaces between them.

# Nun mussen wir make sagen, dass es die .d-Dateien lesen
# (und folgen) soll...
# Das Minus-Zeichen sagt, dass keine Meldung notwendig ist, falls die *.d
# Dateien noch nicht existieren
# (in dem Fall werden die generische Regeln verwendet, s.o.).
-include $(DEPS)

# --------- Andere Regeln
#special - "make clean" deletes all *.o files, the Targets, and test temporaries
clean:
	rm -fv *.o *~ *.d $(TARGETS) rmmix.log
	cd tests && $(MAKE) clean

# By the way, for more information about calling make from make, see
# https://www.gnu.org/software/make/manual/html_node/Recursion.html#Recursion

# For convenience, "make check" == "make test"
check: test

# make tests changes into the tests directory and calls make test there
test: $(TARGETS)
	@echo "************* Unit Tests **********"
	-./unitTester
	@echo "************* Integration Tests **********"
	cd tests && $(MAKE) test

# Algemeiner Regel - wie eine .o Datei von einer .cpp Datei
# erzeugt wird - auch, dass es eine Abhängigkeit gibt (!)
# Hinweis: Der "Recipe" ($(CC) -c $(FLAGS) ...), aber nicht die Abhänigkeit,
# wird verwendet wenn es keine *.d Dateien gibt.  Falls es sie gibt, werden
# die Regelen dort kombiniert mit dem Rezept hier.
%.o : %.cpp
	$(CC) -c $(FLAGS) -o $@ $<

# Nun haben wir alle der *.o Dateien erzeugt.
# Nun müssen wir die übrigen Targets linken.

# Eine allgemeine Regel reicht aus.
# (Nimmt an, dass sowohl der Simulator als auch der Assembler alle $(OBJ)
# brauchen - muss nicht stimmen, ist dennoch harmlos falls falsch).
rmmixas rmmixsim unitTester: %: %.o $(OBJS)
	$(CC) $< $(OBJS) $(LIBS) -o $@


# Fertig!
