
ifeq ($(ENABLE_LSO),1)
OBJS += passes/oracle/mig.o
ifneq ($(LSOEXTERNAL),)
passes/oracle/mig.o: CXXFLAGS += -DABCEXTERNAL='"$(ABCEXTERNAL)"'
endif
endif