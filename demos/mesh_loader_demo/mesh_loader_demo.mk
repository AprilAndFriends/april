##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=mesh_loader_demo
ConfigurationName      :=Debug
IntermediateDirectory  :=../../bin/$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "C:\projects\april\april"
ProjectPath            := "C:\projects\april\april\demos\mesh_loader_demo"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=domagoj
Date                   :=08/19/10
CodeLitePath           :="C:\Program Files\CodeLite"
LinkerName             :=g++
ArchiveTool            :=ar rcus
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
CompilerName           :=g++
C_CompilerName         :=gcc
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
MakeDirCommand         :=makedir
CmpOptions             := -g $(Preprocessors)
LinkOptions            :=  
IncludePath            := "$(IncludeSwitch)C:/dev/UnitTest++-1.3/src" "$(IncludeSwitch)C:/dev/MinGW-4.4.1/include/GL/"  "$(IncludeSwitch)../../include" "$(IncludeSwitch)../../../hltypes/include" "$(IncludeSwitch)../../../gtypes/include" "$(IncludeSwitch)../../aprilutil/include" "$(IncludeSwitch)." 
RcIncludePath          :=
Libs                   :=$(LibrarySwitch)april_d $(LibrarySwitch)hltypes_d $(LibrarySwitch)gtypes_d $(LibrarySwitch)aprilutil_d 
LibPath                :="$(LibraryPathSwitch)C:/dev/UnitTest++-1.3/Debug"  "$(LibraryPathSwitch)../../lib" "$(LibraryPathSwitch)../../bin" "$(LibraryPathSwitch)../../../hltypes/bin/Debug" "$(LibraryPathSwitch)../../../gtypes/bin/Debug" "$(LibraryPathSwitch)../../libaprilutilbin/Debug" 


##
## User defined environment variables
##
UNIT_TEST_PP_SRC_DIR:=C:\UnitTest++-1.3
WXWIN:=C:\dev\wxWidgets-2.8.10
WXCFG:=gcc_dll\mswu
CodeLiteDir:=C:\Program Files\CodeLite
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) 

##
## Main Build Targets 
##
all: $(OutputFile)

$(OutputFile): makeDirStep $(Objects)
	@$(MakeDirCommand) $(@D)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)
	@echo Executing Post Build commands ...
	copy ..\..\bin\*.dll ..\..\bin\Debug
	copy ..\..\..\hltypes\bin\Debug\*.dll ..\..\bin\Debug
	copy ..\..\..\gtypes\bin\Debug\*.dll ..\..\bin\Debug
	@echo Done

makeDirStep:
	@$(MakeDirCommand) "../../bin/$(ConfigurationName)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "C:/projects/april/april/demos/mesh_loader_demo/main.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "C:/projects/april/april/demos/mesh_loader_demo/main.cpp"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "C:/projects/april/april/demos/mesh_loader_demo/main.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe


