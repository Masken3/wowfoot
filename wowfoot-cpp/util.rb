
def nonDotEntries(dir)
	return Dir.entries(dir).reject do |file| file[0,1] == '.' end
end

def dirsAreEqual?(a, b)
	return false if(!File.exist?(a))
	return false if(!File.exist?(b))
	af = nonDotEntries(a)
	bf = nonDotEntries(b)
	return false if(af != bf)	# compare filenames
	af.each_with_index do |file, i|
		return false if(File.mtime(a+'/'+file) != File.mtime(b+'/'+bf[i]))
	end
	# We could check the contents of the files, but I think that's overkill.
	return true
end

#if(!dirsAreEqual?('C:/MinGW', 'C:/MinGW.4.6.1'))
#	system 'rmdir "C:/MinGW"'
#	sh 'junction C:/MinGW C:/MinGW.4.6.1'
#end
