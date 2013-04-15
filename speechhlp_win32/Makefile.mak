LOC =

SHAREDLIB = speechhlp.dll
IMPLIB    = speechhlp.lib

CXX = cl
LD = link
RC = rc
CXXFLAGS  = -nologo -MT -O2 -Oi -Gy -EHsc \
			-DWIN32 -DUNICODE -D_UNICODE -DNDEBUG \
			-D_WINDLL -DSPEECHHLP_DLL -D_WINDOWS -D_USING_V110_SDK71_
MANIFESTFLAGS = -MANIFEST -MANIFESTUAC:"level='asInvoker' uiAccess='false'" \
				-manifest:embed
LDFLAGS = -nologo -release -OPT:REF -OPT:ICF $(MANIFESTFLAGS) \
		  -SUBSYSTEM:WINDOWS -DYNAMICBASE -NXCOMPAT

OBJS = speechhlp.obj

all: $(SHAREDLIB) $(IMPLIB)

$(IMPLIB): $(SHAREDLIB)

$(SHAREDLIB): $(OBJS)
	$(LD) $(LDFLAGS) -dll -implib:$(IMPLIB) \
	  -out:$@ $(OBJS)

.cc.obj:
	$(CXX) -c $(CXXFLAGS) $<

clean:
	-del $(SHAREDLIB)
	-del *.obj
	-del *.exp
	-del *.dll
