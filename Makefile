CC=cl
CCFLAGS=/EHsc /W4

#nmake macros:
#	$@	- Current target's full name
 
all:protect encrypt decrypt lsaSecretRead lsaSecretStore

protect: $@.cpp
	@echo ---------Start building $@:
	$(CC) $(CCFLAGS) $@.cpp
	@echo ---------End building $@:

encrypt: $@.cpp
	@echo ---------Start building $@:
	$(CC) $(CCFLAGS) $@.cpp
	@echo ---------End building $@:

decrypt: $@.cpp
	@echo ---------Start building $@:
	$(CC) $(CCFLAGS) $@.cpp
	@echo ---------End building $@:

lsa_util: $@.cpp
	@echo ---------Start building $@:
	$(CC) $(CCFLAGS) /c $@.cpp
	@echo ---------End building $@:

lsaSecretRead: lsa_util $@.cpp
	@echo ---------Start building $@:
	$(CC) $(CCFLAGS) $@.cpp lsa_util.obj
	@echo ---------End building $@:

lsaSecretStore: lsa_util $@.cpp
	@echo ---------Start building $@:
	$(CC) $(CCFLAGS) $@.cpp lsa_util.obj
	@echo ---------End building $@:
		
clean:
	del *.obj
	 
cleanall:
	del *.exe *.obj