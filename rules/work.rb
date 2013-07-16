require "#{File.dirname(__FILE__)}/base.rb"
require "#{File.dirname(__FILE__)}/util.rb"
require 'fileutils'

# A Task representing a file.
# @NAME is the name of the file.
class FileTask < Task
	def initialize(n)
		#puts "FileTask(#{n})"
		setName(n)
		super()
	end

	def setNeeded
		return if(@needed)
		if(!File.exist?(@NAME))
			@needed = "Because file does not exist:"
		else
			@needed = false
			super
			if(@prerequisites && !@needed)
				d = newDate
				@prerequisites.each do |n|
					if(n.respond_to?(:newDate) && n.newDate > d)
						@needed = "Because prerequisite '#{n}'(#{n.class}) is newer:"
						break
					end
				end
			end
		end
	end

	def newDate
		return File.mtime(@NAME)
	end

	def to_str
		@NAME
	end

	def to_s
		@NAME
	end

	# used by Works.add
	def name
		@NAME
	end

	def execute
		begin
			fileExecute
		rescue => e
			# In case the output file is created in error
			FileUtils.rm_f(@NAME)
			raise
		end
	end

private

	def setName(n)
		@NAME = n.to_s
		# names may not contain '~', the unix home directory hack, because Ruby doesn't parse it.
		if(@NAME.start_with?('~'))
			raise "Bad filename: #{@NAME}"
		end
	end
end

# A Task for creating a directory, recursively.
# For example, if you want to create 'foo/bar', you need not create two DirTasks. One will suffice.
class DirTask < FileTask
	def fileExecute
		FileUtils::Verbose.mkdir_p @NAME
	end

	def setNeeded
		return if(@needed)
		if(!File.exist?(@NAME))
			@needed = "Because directory does not exist:"
		elsif(!File.directory?(@NAME))
			@needed = "Because file is not a directory:"
		else
			@needed = false
		end
		# call Task.setNeeded, not FileTask.setNeeded.
		Task.instance_method(:setNeeded).bind(self).call
	end

	# Hack to make FileTask's date comparison ignore directories.
	# EARLY is less than every other date.
	def newDate
		return EARLY
	end
end

# A Task for copying a file.
class CopyFileTask < FileTask
	# name is a String, the destination filename.
	# src is a FileTask, the source file.
	# preq is an Array of Tasks, extra prerequisites. Alternatively, :force,
	# which makes sure the file is copied if the destination size or date is different.
	def initialize(name, src, preq = [])
		if(preq == :force)
			@force = true
			if(File.exist?(@src.to_s))
				if(File.mtime(@src) != File.mtime(@NAME))
					@needed = "Because source '#{@src}' has different date:"
				elsif(File.size(@src) != File.size?(@NAME))
					@needed = "Because source '#{@src}' has different size:"
				else
					@needed = false
				end
			end
		else
			@force = false
			@prerequisites = [src] + preq
		end
		@src = src
		super(name)
	end

	def fileExecute
		puts "Copy #{@src} #{@NAME}"
		FileUtils.copy_file(@src, @NAME, true)
		if(@force)
			now = Time.now
			# Update time to make sure downstream tasks are properly rebuilt.
			File.utime(now, now, @NAME)
			# Update source time to prevent file from being re-copied.
			File.utime(now, now, @src)
			return
		end
		# Work around a bug in Ruby's utime, which is called by copy_file.
		# Bug appears during Daylight Savings Time, when copying files with dates outside DST.
		mtime = File.mtime(@src)
		if(!mtime.isdst && Time.now.isdst)
			mtime += Time.now.utc_offset - mtime.utc_offset
			File.utime(mtime, mtime, @NAME)
		end
	end
end

# generate file in memory, then compare it to the one on disk
# to decide if it should be overwritten.
# subclasses must set member variable @buf before calling 'super' in 'initialize'.
class MemoryGeneratedFileTask < FileTask
	def initialize(name, &block)
		setName(name)
		instance_eval(&block) if(block)
		super(name)
	end
	def setNeeded
		if(File.exist?(@NAME))
			@ec = open(@NAME).read
			if(@buf != @ec)
				@needed = "Because generated file has changed:"
			else
				@needed = false
			end
		end
		super
	end
	def fileExecute
		file = open(@NAME, 'w')
		file.write(@buf)
		file.close
		@ec = @buf
	end
end

# Copies a directory, its contents and subdirectories.
class CopyDirTask < Task
	def initialize(dstRoot, name, srcName = name, copySubdirs = true)
		@NAME = name
		@dstRoot = dstRoot
		@srcName = srcName
		@copySubdirs = copySubdirs
		@prerequisites = []
		glob("#{@dstRoot}/#{@NAME}", @srcName)
		super()
	end
private
	def glob(dst, src)
		@prerequisites << DirTask.new(dst)
		sources = Dir.glob("#{src}/*", File::FNM_DOTMATCH) - ["#{src}/.", "#{src}/.."]
		sources.each do |s|
			if(File.directory?(s))
				glob("#{dst}/#{File.basename(s)}", s) if(@copySubdirs)
			else
				@prerequisites << CopyFileTask.new("#{dst}/#{File.basename(s)}", FileTask.new(s))
			end
		end
	end
end

# A Task representing multiple files.
# If any of the files are out-of-date, the Task will be executed.
# The first file is designated primary, and acts as the single file in the parent class, FileTask.
class MultiFileTask < FileTask
	# name -> string
	# files -> array of strings
	def initialize(name, files)
		@files = files.collect do |f|
			fn = f.to_s
			# names may not contain '~', the unix home directory hack, because Ruby doesn't parse it.
			if(fn.include?('~'))
				raise "Bad filename: #{fn}"
			end
			if(!File.exist?(fn))
				@needed = "Because secondary file '#{fn}' does not exist:"
			elsif(@prerequisites)
				d = File.mtime(fn)
				@prerequisites.each do |n|
					if(n.respond_to?(:newDate) && n.newDate > d)
						@needed = "Because prerequisite '#{n}'(#{n.class}) is newer:"
						break
					end
				end
			end
			fn
		end
		super(name)
	end

	# Returns the date of the newest file.
	def newDate
		d = super
		@files.each do |file|
			fd = File.mtime(file)
			d = fd if(fd > d)
		end
		return d
	end
end
