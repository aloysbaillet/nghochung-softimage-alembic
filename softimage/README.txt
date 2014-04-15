============================================================================================
Copyright 2013 Autodesk, Inc.
============================================================================================

BUILD INSTRUCTION FOR SOFTIMAGE ALEMBIC PLUGIN

Requirements:
	Softimage 2015 Beta2
	Alembic 1.5.1 libraries (refer to BuildingAlembic.txt for notes)
	Environment: 
		Windows: Windows 8, MS Visual Studio 2012 
		Linux: Fedora 14, gcc 4.1.2, make 3.82

1. Start a shell with Softimage environment
	Windows:
		Start Softimage Command Prompt
	Linux:
		$ source path/to/Softimage_2015/.xsi_2015

2. Setup build environment:
	Windows: 
		> call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x64
	
	Linux:
		$ setenv CXX path/to/gcc-4.1.2
		$ setenv CC path/to/gcc-4.1.2

3. Unzip the source into your destination folder

4. Change to the destination folder

5. Create symbolic link to Alembic 1.5.1 folder
	Windows:
		> mklink /D Alembic path\to\Alembic1.5.1
	
	Linux:
		$ ln -s path/to/Alembic1.5.1 Alembic
	
	Alternatively, you can copy Alembic folder over

6. (Optional) Run the script to backup the original libraries in Softimage 2015 installation
	Windows: 
		> backup.bat
		(The libraries are saved in folder ./backup)
	
	Linux: 
		$ make backup
		(The libraries are saved in ./backup.tar)

7. Run the build script.
	Windows: 
		> build.bat
	Linux: 
		$ make
	 
	The build result is saved in folder ./Application
		Application
			bin
				AbcFramework.dll / libAbcFramework.so
			Plugins
				AbcImportExport.dll / libAbcImportExport.so
			Dictionary
				en
					ABCIMPORTEXPORT.dict
				jp
					ABCIMPORTEXPORT.dict [only on Windows]
			
8. Run the script to deploy the built libraries to Softimage 2015 installation folder
	Windows: 
		> deploy.bat
	Linux: 
		$ make deploy

