
ifeq ($(ENABLE_LSO),1)
OBJS += passes/oracle/oracle.o
ifneq ($(LSOEXTERNAL),)
passes/oracle/oracle.o: CXXFLAGS += -DLSOEXTERNAL='"$(LSOEXTERNAL)"'
endif
endif