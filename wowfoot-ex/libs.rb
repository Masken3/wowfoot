require File.expand_path '../rules/cDll.rb'
require File.expand_path '../rules/cLib.rb'
require './config.rb'

commonFlags = ' -Wno-all -Wno-extra -Wno-c++-compat -Wno-missing-prototypes -Wno-missing-declarations -Wno-shadow'

LIBMPQ = DllWork.new do
	@SOURCES = ['src/libs/libmpq/libmpq']
	@EXTRA_INCLUDES = ['src/libs/libmpq']
	@EXTRA_CFLAGS = commonFlags
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@LIBRARIES = ['bz2', 'z']
	@LIBRARIES << 'mingwex' if(HOST == :win32)
	@NAME = 'libmpq'
end

CRBLIB = LibWork.new do
	@SOURCES = ['src/libs/blp/crblib']
	@IGNORED_FILES = ['list.c', 'myassert.c', 'floatutil.c',
		'crbeqlib.c', 'chshutil.c', 'spawnmutil.c']
	@EXTRA_INCLUDES = ['src/libs/blp']
	@EXTRA_CFLAGS = commonFlags + ' -Wno-error'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@SPECIFIC_CFLAGS = {
		'MATRIX.C' => ' -Wno-uninitialized',
	}
	@LIBRARIES = []
	@NAME = 'crblib'
end

PALBMP = LibWork.new do
	@SOURCES = ['src/libs/blp/palbmp']
	@IGNORED_FILES = []
	@EXTRA_INCLUDES = ['src/libs/blp']
	@EXTRA_CFLAGS = commonFlags
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@SPECIFIC_CFLAGS = {
		'palettize.c' => ' -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast',
	}
	@LIBRARIES = []
	@NAME = 'palbmp'
end

SQUISH = LibWork.new do
	@SOURCES = ['src/libs/blp/squish']
	@EXTRA_INCLUDES = @SOURCES
	@EXTRA_CFLAGS = commonFlags
	@NAME = 'squish'
end

BLP = LibWork.new do
	@SOURCES = ['src/libs/blp']
	@EXTRA_INCLUDES = ['src/libs/blp'] + CONFIG_BLP_INCLUDES
	@EXTRA_CFLAGS = commonFlags
	@SPECIFIC_CFLAGS = {
		'MemImage.cpp' => ' -Wno-clobbered',
	}
	@EXTRA_CPPFLAGS = ' -DXMD_H -DHAVE_BOOLEAN' if(HOST == :win32)
	#@LIBRARIES = ['png']	# ignored by *LibWork
	@NAME = 'libblp'
end
