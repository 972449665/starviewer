REM Call vtk configuration script to make sure VtkDir is set
call "%ScriptsRoot%\config\vtk.cmd"

set SourceDir=%UserProfile%\Downloads\gdcm-2.4.3

if %BuildType% == debug (
    set CMakeBuildType=Debug
    set BuildDir=%SourceDir%-build-debug
    set InstallPrefix=%SystemDrive%/gdcm/2.4.3d
)
if %BuildType% == release (
    set CMakeBuildType=RelWithDebInfo
    set BuildDir=%SourceDir%-build-release
    set InstallPrefix=%SystemDrive%/gdcm/2.4.3
)

REM ============== Nothing should need to be changed below this line ==============

set CMakeOptions=-DCMAKE_BUILD_TYPE:STRING=%CMakeBuildType% ^
                 -DCMAKE_INSTALL_PREFIX:PATH="%InstallPrefix%" ^
                 -DGDCM_BUILD_SHARED_LIBS:BOOL=TRUE ^
                 -DGDCM_USE_VTK:BOOL=TRUE ^
                 -DVTK_DIR="%VtkDir%"

set GdcmDir=%InstallPrefix%/lib/gdcm-2.4
