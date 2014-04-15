# this makefile is to be used with gmake

# Global variables
include $(XSISDK_ROOT)/mkfiles/vars.gnu

INCLUDES := \
	-I./include \
	-I../Alembic/include/OpenEXR \
	-I../Alembic/include \
	$(END_OF_LIST)

DEFINES += -DABCFRAMEWORK_IMPL -D_SGI_MP_SOURCE -fpermissive

ABCLIBDIR := ../Alembic/linux/optimized/

SYSLIBS += \
	-lAlembicAbcMaterial \
	-lAlembicAbcCollection \
	-lAlembicAbcGeom \
	-lAlembicAbc \
	-lAlembicAbcCoreFactory \
	-lAlembicAbcCoreHDF5 \
	-lAlembicAbcCoreAbstract \
	-lAlembicAbcCoreOgawa \
	-lAlembicOgawa \
	-lAlembicUtil \
	-lhdf5_hl \
	-lhdf5 \
	-lImath \
	-lIlmThread \
	-lIex \
	-lHalf \
	-lboost_system-gcc41-mt-1_53 \
	-lboost_program_options-gcc41-mt-1_53 \
	-lboost_thread-gcc41-mt-1_53 \
	-lboost_filesystem-gcc41-mt-1_53 \
	-lrt \
	-lm \
	-lpthread \
	-lz \
	$(END_OF_LIST)

LDOPTS := -L$(ABCLIBDIR) -Wall

SOURCES := \
	src/AbcFrameworkDll.cpp \
	src/AbcFrameworkMem.cpp \
	src/CAbcDataTypeHelper.cpp \
	src/CAbcFramework.cpp \
	src/CAbcIArchive.cpp \
	src/CAbcICamera.cpp \
	src/CAbcIFaceSet.cpp \
	src/CAbcInput.cpp \
	src/CAbcIPoints.cpp \
	src/CAbcIPolyMesh.cpp \
	src/CAbcIPropertyAccessor.cpp \
	src/CAbcISampleSelector.cpp \
	src/CAbcIXform.cpp \
	src/CAbcOArchive.cpp \
	src/CAbcOCamera.cpp \
	src/CAbcOFaceSet.cpp \
	src/CAbcOObjectFactory.cpp \
	src/CAbcOPoints.cpp \
	src/CAbcOPolyMesh.cpp \
	src/CAbcOProperty.cpp \
	src/CAbcOPropertyFactory.cpp \
	src/CAbcOXform.cpp \
	src/CAbcTimeSampling.cpp \
	src/CAbcUtils.cpp \
	$(END_OF_LIST)
			
# Target
SOTARGETS  := ../Application/bin/libAbcFramework.so

# Global rules
include $(XSISDK_ROOT)/mkfiles/rules.gnu

# Specific dependencies
$(SOTARGETS): $(ALLOBJECTS)
$(CXXOBJECTS): $(OBJDIR)/%.o: %.cpp
$(COBJECTS): $(OBJDIR)/%.o: %.c

