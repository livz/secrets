CC=cl
CCFLAGS=/EHsc /W4

#nmake macros:
#	$@	- Current target's full name
 
all:protect encrypt decrypt lsaSecretRead lsaSecretStore

protect: $@.cpp
	@echo ---------Building $@: 
	$(CC) $(CCFLAGS) $@.cpp

encrypt: $@.cpp
	@echo ---------Building $@:
	$(CC) $(CCFLAGS) $@.cpp

decrypt: $@.cpp
	@echo ---------Building $@:
	$(CC) $(CCFLAGS) $@.cpp

lsaSecretRead: $@.cpp
	@echo ---------Building $@:
	$(CC) $(CCFLAGS) $@.cpp lsa_util.cpp

lsaSecretStore: $@.cpp
	@echo ---------Building $@:
	$(CC) $(CCFLAGS) $@.cpp lsa_util.cpp
		
clean:
	del *.obj
	 
cleanall:
	del *.exe *.obj