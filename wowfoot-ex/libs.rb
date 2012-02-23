require File.expand_path '../rules/dll.rb'
require File.expand_path '../rules/native_lib.rb'
require './config.rb'

LIBMPQ = DllWork.new
LIBMPQ.instance_eval do
	@SOURCES = ['src/libs/libmpq/libmpq']
	@EXTRA_INCLUDES = ['src/libs/libmpq']
	@EXTRA_CFLAGS = ' -Wno-all -Wno-error -Wno-extra -Wno-missing-declarations'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@LIBRARIES = ['bz2', 'z']
	@LIBRARIES << 'mingwex' if(HOST == :win32)
	@NAME = 'libmpq'
end

CRBLIB = NativeLibWork.new
CRBLIB.instance_eval do
	@SOURCES = ['src/libs/blp/crblib']
	@IGNORED_FILES = ['list.c', 'myassert.c', 'floatutil.c',
		'crbeqlib.c', 'chshutil.c', 'spawnmutil.c']
	@EXTRA_INCLUDES = ['src/libs/blp']
	@EXTRA_CFLAGS = ' -Wno-all -Wno-error -Wno-extra'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@LIBRARIES = []
	@NAME = 'crblib'
end

PALBMP = NativeLibWork.new
PALBMP.instance_eval do
	@SOURCES = ['src/libs/blp/palbmp']
	@IGNORED_FILES = []
	@EXTRA_INCLUDES = ['src/libs/blp']
	@EXTRA_CFLAGS = ' -Wno-all -Wno-error -Wno-extra -Wno-c++-compat'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@LIBRARIES = []
	@NAME = 'palbmp'
end

SQUISH = NativeLibWork.new
SQUISH.instance_eval do
	@SOURCES = ['src/libs/blp/squish']
	@EXTRA_INCLUDES = @SOURCES
	@EXTRA_CFLAGS = ' -Wno-all -Wno-error -Wno-extra'
	@NAME = 'squish'
end

BLP = NativeLibWork.new
BLP.instance_eval do
	@SOURCES = ['src/libs/blp']
	@EXTRA_INCLUDES = ['src/libs/blp'] + CONFIG_BLP_INCLUDES
	@EXTRA_CFLAGS = ' -Wno-all -Wno-error -Wno-extra'
	@EXTRA_CPPFLAGS = ' -DXMD_H -DHAVE_BOOLEAN' if(HOST == :win32)
	#@LIBRARIES = ['png']	# ignored by *LibWork
	@NAME = 'libblp'
end
