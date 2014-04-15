build:: projects

GNUmkfiles := \
	AbcFramework/GNUmakefile \
	AbcImportExport/GNUmakefile \
	$(END_OF_LINE)

projects:
	exitcode=0;\
	for file in $(GNUmkfiles); do \
		make -f `basename $$file` -C `dirname $$file` || exitcode=1;\
	done;\
	exit $$exitcode
	
clean::
	for file in $(GNUmkfiles); do \
		make -f `basename $$file` -C `dirname $$file` clean;\
	done;\

clean::
	-rm -rf ./Application

backup::
	-tar -cf ./backup.tar \
		$(SI_HOME)/Application/bin/libAbcFramework.so \
		$(SI_HOME)/Application/Plugins/libAbcImportExport.so \
		#$(SI_HOME)/Application/Dictionary/en/ABCIMPORTEXPORT.dict \
		$(END_OF_LINE)

deploy::
	-cp -r -v ./Application/ $(SI_HOME)
	
rebuild::
	make clean
	make
	
