require 'dbi'

module DBI
	def self.setupExtras(connection)
		puts "setupExtras"
		p connection
		connection.instance_eval do
			extend DBIConnectionTimer
		end
	end
end

module MyTimer
	def timer_start(title)
		@timer_title = title
		@timer_start = Time.now
	end
	def timer_end
		te = Time.now
		elapsed = (te - @timer_start) * 1000
		if(elapsed > 50)
			puts "Time: #{elapsed} ms for #{@timer_title}."
		end
	end
end

module DBIConnectionTimer
	include MyTimer
	def self.extended(base)
		puts "Conn Aliased!"
		base.instance_eval do
			alias orig_prepare prepare
			alias prepare my_prepare
		end
	end

	def my_prepare(sql)
		timer_start("prepare(#{sql})")
		stm = orig_prepare(sql)
		timer_end
		stm.instance_eval do
			@timer_sql = sql
			extend DBIStatementTimer
		end
	end
end

module DBIStatementTimer
	include MyTimer
	def self.extended(base)
		#puts "Stm Aliased!"
		base.instance_eval do
			alias orig_execute execute
			alias execute my_execute
			alias orig_fetch_all fetch_all
			alias fetch_all my_fetch_all
			#alias orig_fetch fetch
			#alias fetch my_fetch
		end
	end
	def timer_sstart(t)
		timer_start("#{t}(#{@timer_sql})")
	end

	def my_execute(*params)
		timer_sstart('execute')
		orig_execute(*params)
		timer_end
	end

	def my_fetch_all
		timer_sstart('fetch_all')
		res = orig_fetch_all
		timer_end
		return res
	end

	def my_fetch
		timer_sstart('fetch')
		res = orig_fetch
		timer_end
		return res
	end
end
