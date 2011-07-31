class String
	if(!respond_to?(:getbyte))
		def getbyte(index)
			return self[index]
		end
	end
end
