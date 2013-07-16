# Copyright (C) 2009 Mobile Sorcery AB
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License, version 2, as published by
# the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to the Free
# Software Foundation, 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

require "#{File.dirname(__FILE__)}/config.rb"

# This module contains functions for saving and comparing compile or link flags.
# execFlags and flagsNeeded require the function "cFlags".
module FlagsChanged
	# Call from execute.
	def execFlags
		if(@OLDFLAGS != cFlags) then
			open(@FLAGSFILE, 'w') { |f| f.write(cFlags) }
			@OLDFLAGS = cFlags
		end
	end

	def setNeeded
		@FLAGSFILE = @NAME + ".flags"
		super
		if(File.exists?(@FLAGSFILE)) then
			@OLDFLAGS = open(@FLAGSFILE) { |f| f.read }
		else
			@needed = "Because the flags file is missing:"
			return
		end
		if(!@needed && @OLDFLAGS != cFlags)
			@needed = "Because the flags have changed:"
			@needed << "\nOld: #{@OLDFLAGS}" if(PRINT_FLAG_CHANGES)
			@needed << "\nNew: #{cFlags}\n" if(PRINT_FLAG_CHANGES)
			#p @OLDFLAGS, cFlags
			#p (@OLDFLAGS == cFlags)
			#raise hell
		end
	end
end
