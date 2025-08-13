CC= gcc
RM= rm

MAIN= novamain
PIPE= novapipe
INPUT= novain
OUTPUT= novaout
PARSE= novacmp
EXEC= novaexe
OPS= novaops
NAMES= $(PIPE) $(INPUT) $(OUTPUT) $(EXEC) $(OPS) $(PARSE) $(MAIN)

SRCDIR= sources/
OBJDIR= objects/
HEADDIR= headers/
OBJS= $(addsuffix .o,$(addprefix $(OBJDIR),$(NAMES)))

PROGNAME= nova

CCFLAGS= -Wall -I $(HEADDIR)
LNKFLAGS= -lpthread

all: $(OBJS)
	$(CC) $(OBJS) -o $(PROGNAME) $(LNKFLAGS)
$(OBJDIR)$(PIPE).o: $(SRCDIR)$(PIPE).c $(HEADDIR)$(PIPE).h
	$(CC) -c $(SRCDIR)$(PIPE).c -o $(OBJDIR)$(PIPE).o $(CCFLAGS)
$(OBJDIR)$(INPUT).o: $(SRCDIR)$(INPUT).c $(HEADDIR)$(INPUT).h $(HEADDIR)$(PIPE).h
	$(CC) -c $(SRCDIR)$(INPUT).c -o $(OBJDIR)$(INPUT).o $(CCFLAGS)
$(OBJDIR)$(OUTPUT).o: $(SRCDIR)$(OUTPUT).c $(HEADDIR)$(OUTPUT).h $(HEADDIR)$(PIPE).h
	$(CC) -c $(SRCDIR)$(OUTPUT).c -o $(OBJDIR)$(OUTPUT).o $(CCFLAGS)
$(OBJDIR)$(EXEC).o: $(SRCDIR)$(EXEC).c $(HEADDIR)$(EXEC).h $(HEADDIR)$(PIPE).h
	$(CC) -c $(SRCDIR)$(EXEC).c -o $(OBJDIR)$(EXEC).o $(CCFLAGS)
$(OBJDIR)$(OPS).o: $(SRCDIR)$(OPS).c $(HEADDIR)$(OPS).h $(HEADDIR)$(EXEC).h
	$(CC) -c $(SRCDIR)$(OPS).c -o $(OBJDIR)$(OPS).o $(CCFLAGS)
$(OBJDIR)$(PARSE).o: $(SRCDIR)$(PARSE).c $(HEADDIR)$(PARSE).h $(HEADDIR)$(PIPE).h $(HEADDIR)$(EXEC).h $(HEADDIR)$(OPS).h
	$(CC) -c $(SRCDIR)$(PARSE).c -o $(OBJDIR)$(PARSE).o $(CCFLAGS)
$(OBJDIR)$(MAIN).o: $(SRCDIR)$(MAIN).c $(HEADDIR)$(PIPE).h $(HEADDIR)$(INPUT).h $(HEADDIR)$(OUTPUT).h $(HEADDIR)$(PARSE).h $(HEADDIR)$(EXEC).h
	$(CC) -c $(SRCDIR)$(MAIN).c -o $(OBJDIR)$(MAIN).o $(CCFLAGS)
	


.PHONY: clean
clean:
	$(RM) $(PROGNAME) $(OBJS)
