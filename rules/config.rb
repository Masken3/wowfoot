# load local_config.rb, if it exists.
lc = "#{File.dirname(__FILE__)}/local_config.rb"
require lc if(File.exists?(lc))

require "#{File.dirname(__FILE__)}/util.rb"

# These are default values. Users should not modify them.
# Instead, users should create local_config.rb and put their settings there.

default_const(:PRINT_FLAG_CHANGES, true)
default_const(:USE_COMPILER_VERSION_IN_BUILDDIR_NAME, true)
default_const(:EXIT_ON_ERROR, true)
default_const(:PRINT_WORKING_DIRECTORY, false)

require "#{File.dirname(__FILE__)}/gccModule.rb"
default_const(:DefaultCCompilerModule, GccCompilerModule)
