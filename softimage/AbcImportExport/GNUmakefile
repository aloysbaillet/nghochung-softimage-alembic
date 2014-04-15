# this makefile is to be used with gmake

# Global variables
include $(XSISDK_ROOT)/mkfiles/vars.gnu

INCLUDES += \
	-I../Alembic/include/OpenEXR \
	-I../AbcFramework/include \
    $(END_OF_LIST)

SOURCES := \
	AbcImportExport.cpp \
	AbcXsiExporter.cpp \
	AbcXsiIO.cpp \
	AbcXsiImporter.cpp \
	AbcXsiUtils.cpp \
	ExportHelper.cpp \
	$(END_OF_LIST)

# Target
SOTARGETS  := ../Application/Plugins/libAbcImportExport.so

# Localization
LOCNAME := ABCIMPORTEXPORT
DICTPATH := ../Application/Dictionary

LOCHEADER := ./sidictres.h
LOCDICT := $(DICTPATH)/en/$(LOCNAME).dict

CUSTOM_TARGETS = $(LOCDICT)

# Global rules
include $(XSISDK_ROOT)/mkfiles/rules.gnu

# Note: this rule has to be defined after including rules.gnu
$(LOCHEADER) $(LOCDICT): $(LOCNAME).dictxml
	mkdir -p ../Application/Dictionary/en
	perl $(XSI_BINDIR)/xsiDictParser.pl -dictdir $(DICTPATH) -inputfile $(LOCNAME).dictxml -headerdir .

# Specific dependencies
$(SOTARGETS): $(ALLOBJECTS) $(XSISDKLIBS)
$(CXXOBJECTS): $(OBJDIR)/%.o: %.cpp
$(COBJECTS): $(OBJDIR)/%.o: %.c

