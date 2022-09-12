GCC = gcc
GCCFLAGS = -g -Wall -std='c99' -lpthread -lrt

EXECUTABLES := app slave view

OUTPUT = results.csv

EXEC = App Slave View

APPLICATION_C = app.c

SLAVE_C = slave.c

VIEW_C = view.c

SHMADT_C = shmADT.c

LIB_C = lib.c

all: $(EXEC)

Slave:
	@$(GCC) $(GCCFLAGS) $(SHMADT_C) $(SLAVE_C) $(LIB_C) -o slave
	@echo "Slave successfully compiled"

App:
	@$(GCC) $(GCCFLAGS) $(SHMADT_C) $(LIB_C) $(APPLICATION_C) -o app
	@echo "Solve successfully compiled"

View:
	@$(GCC) $(GCCFLAGS) $(SHMADT_C) $(LIB_C) $(VIEW_C) -o view
	@echo "View successfully compiled"

clean:
	@rm -rf $(EXECUTABLES)
	@rm -rf $(OUTPUT)
	@rm -rf check

check:
	@mkdir check

	@pvs-studio-analyzer trace -- make
	@pvs-studio-analyzer analyze
	@plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log

	@mv strace_out check
	@rm PVS-Studio.log

.PHONY: all clean check