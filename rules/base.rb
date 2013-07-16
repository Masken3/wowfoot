#require "#{File.dirname(__FILE__)}/config.rb"
require "#{File.dirname(__FILE__)}/util.rb"
require "#{File.dirname(__FILE__)}/error.rb"
require 'thread'

# puts is not atomic; its newline can be written after another thread has written more stuff.
# write seems safer.
# update: not even write() was thread-safe. time for a mutex.
class IO
	@@mutex = Mutex.new
	def puts(str)
		@@mutex.synchronize do
			$stdout.write("#{Works.threadName}#{str}\n")
		end
	end
end

# We want exceptions to have better backtrace...
class Exception
	def message
		# If there's no backtrace, we're inside the default exception handler?
		s = Thread.current.backtrace.size
		if(s == 1)
			$stderr.write(': ')
			Works.prettyPrintException(self, 1)
			exit!(1)
		else
			return to_s
		end
	end
end

# @needed is false, or a "Because"-type string.
# It is often useful to overload 'setNeeded'.
# @prerequisites is an Array of Tasks.
# If any prerequisite is needed, this task is also needed.
# All prerequisites must be executed before this task can be executed.
# @requirements is an Array of Tasks.
# It is like @prerequisites, except they don't cause this task to become needed.
class Task
	# In subclasses, call 'super' at the end of 'initialize'.
	# Checks prerequisites, if any, and adds self to Works, if needed.
	def initialize
		setNeeded
		raise "@needed not set!" if(@needed == nil)
		@needed.freeze
		Works.add(self) if(@needed)
	end

	def setNeeded
		if(!@needed && @prerequisites)
			@prerequisites.each do |n|
				if(n.needed)
					# The prerequisite hasn't been rebuilt yet, but by the time this line is printed,
					# it will have been. Isn't tenses fun? :)
					@needed = "Because prerequisite '#{n}'(#{n.class}) was rebuilt:"
					break
				end
			end
		end
	end

	def execute
		raise "'execute' not implemented for class #{self.class.name}"
	end

	def needed
		return @needed
	end

	def prerequisites
		return @prerequisites if(@prerequisites)
		return []
	end

	def requirements
		return @requirements if(@requirements)
		return []
	end

	def eql?(o)
		compareWithLogging(o, false)
	end

	def compareWithLogging(o, log = true)
		if(self.class != o.class)
			p self.class, o.class if(log)
			return false
		end
		a = self.instance_variables - uncomparedVariables
		b = o.instance_variables - uncomparedVariables
		if(a != b)
			p a, b if(log)
			return false
		end
		a.each do |iv|
			av = self.instance_variable_get(iv)
			bv = o.instance_variable_get(iv)
			if(av != bv)
				p iv if(log)
				p av, bv if(log)
				return false
			end
		end
		return true
	end

	def uncomparedVariables
		return [:@needies, :@neededCount]
	end

	# These must not be used by any class other than Works.
	attr_accessor(:needies, :neededCount)
end

# algo:
# Have a queue of tasks that are ready for execution (RfE).
# When adding tasks, add them to RfE iff they have no needed prerequisites.
# Otherwise, save the count of needed prereqs, and add, in all needed prereqs, a link to the needy task.
# When a task has finished execution, for each link to needy tasks, decrement the count of needed prereqs.
# If the count is zero, add the needy task to RfE.

# We could keep the set of needy tasks, to make sure we didn't miss anything at the end,
# but that seems redundant.

# Responsible for scheduling and running tasks.
class Works
	def self.prettyPrintException(e, offset, c = nil)
		msg = "Unhandled exception: #{e.to_s} (#{e.class})"
		if(c == nil || e.class != c)
			msg << "\n#{e.backtrace[offset..-1].join("\n")}"
		end
		puts msg
	end

	# Run all scheduled tasks. Don't stop until they're all done, or one fails.
	# If one fails, let the other running ones complete before returning.
	def self.run
		return if(@@tasks.empty?)

		puts "starting multi-processing..."

		threads = []
		threadCount = number_of_processors
		#if(@@tasks.size < threadCount)
		#	threadCount = @@tasks.size
		#end
		for i in (1..threadCount) do
			threads << Thread.new(i) do |i|
				#puts "Start thread #{i}"
				begin
					runThread(i)
				rescue Object => e
					# stop the other threads from starting new tasks, but let them finish their current ones.
					@@mutex.synchronize do
						prettyPrintException(e, 0, WorkError)
						@@abort = true
						# @@threadNames.size needs to be up-to-date.
						@@threadNames.delete(Thread.current.object_id)
					end
				end
				#puts "End thread #{i}"
			end
		end

		threads.each do |t|
			t.join
		end

		puts "multi-processing complete."
		reset
	end

	def self.add(task)
		#puts "Add #{task} with #{task.prerequisites} prereqs"
		# make sure task is valid
		checkTaskMethod(task, :needed)
		checkTaskMethod(task, :needies)
		checkTaskMethod(task, :needies=)
		checkTaskMethod(task, :neededCount)
		checkTaskMethod(task, :neededCount=)
		if(task.is_a?(FileTask) && task.method(:execute).owner != FileTask)
			raise "You may not override FileTask.execute"
		end
		raise "Task is not needed!" if(!task.needed)

		# detect duplicates
		if(task.respond_to?(:name))
			key = task.name
		else
			key = task
		end
		t = @@taskSet[key]
		if(t == nil)
			@@taskSet[key] = task
		else
			e = t.compareWithLogging(task)
			if(!e)
				#p task.respond_to?(:name), key, t.name, task.name
				raise "Duplicate variant task detected!"
			end
			# duplicate task detected.
			# replace prerequisites of needies of the new task with references to the old one,
			# then discard the new one.
			# except we can't do that yet, because, as a prerequisite, this task is added before any needies.
			# mark this task as supplanted by t.
			task.needies = t
			return
		end

		# if task has needed prereqs, add it to them.
		count = handleRequirements(task.prerequisites, task)
		count += handleRequirements(task.requirements, task)

		#puts "#{task} has #{count} of #{task.prerequisites.size} prerequisites needed."
		if(count == 0)
			@@tasks << task
		else
			task.neededCount = count
		end
	end

	def self.parseArgs(args)
		#puts args.inspect
		raise hell if(@@args_handled)
		args.each do |a| handle_arg(a) end
		@@args_handled = true
	end

	def self.threadName
		return @@threadNames[Thread.current.object_id]
	end

	def self.addTarget(a)
		# todo: fix
	end

	private

	# returns count
	def self.handleRequirements(array, task)
		count = 0
		array.each_with_index do |p, i|
			if(p.needed)
				count += 1
				# needies is normally an array.
				# if it's a task, then p is a supplanted task.
				# replace it.
				if(p.needies.is_a?(Task))
					array[i] = p = p.needies
				end
				p.needies ||= []
				p.needies << task
			end
		end
		return count
	end

	def self.checkTaskMethod(task, name)
		if(task.method(name).owner != Task)
			raise "You may not override Task.#{name}"
		end
	end

	@@goals = []
	@@mutex = Mutex.new	# protects access to @@abort, @@waitingThreads, @@tasks and @@nextTask.
	@@cond = ConditionVariable.new	# signaled when @@tasks or @@abort changes.

	def self.reset
		# used to deduplicate identical tasks,
		# and detect tasks with the same name but different settings.
		@@taskSet = {}

		@@tasks = []
		@@nextTask = 0
		@@waitingThreads = 0
		@@abort = false
		@@threadNames = {}
	end

	reset

	def self.runThread(i)
		@@threadNames[Thread.current.object_id] = "#{i} "

		loop do
			task = nil
			@@mutex.synchronize do
				return if(@@abort)
				while(@@tasks.size == @@nextTask)
					@@waitingThreads += 1
					@@cond.wait(@@mutex)
					@@waitingThreads -= 1
					return if(@@abort)
				end
				task = @@tasks[@@nextTask]
				@@nextTask += 1
			end
			puts "#{task.needed} #{task}"
			task.execute
			#puts "done"
			@@mutex.synchronize do
				if(task.needies)
					task.needies.each do |n|
						n.neededCount -= 1
						@@tasks << n if(n.neededCount == 0)
						raise hell if(n.neededCount < 0)
						@@cond.signal
					end
				end
				# if there are no more tasks, and all other threads are waiting, we're done.
				if(@@tasks.size == @@nextTask && @@waitingThreads == (@@threadNames.size - 1))
					@@abort = true
					@@cond.broadcast
				end
			end
		end
	end

	def self.handle_arg(a)
		i = a.index('=')
		if(i) then
			if(@@handlers[name])
				puts "Handler #{name}"
				@@handlers[name].each do |block|
					block.call(value)
				end
			else
				raise "Unhandled argument #{a}"
			end
		else
			@@goals << a.to_sym
		end
	end

	def self.startTask(task)
		puts "#{thread.id}: In #{FileUtils.pwd}" if(PRINT_WORKING_DIRECTORY)
		puts "#{thread.id}: #{task.needed}"
	end
end


def target(args, &block)
	Works.addTarget(args, &block)
end
