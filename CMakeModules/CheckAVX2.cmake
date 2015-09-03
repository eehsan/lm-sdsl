
set(BUILTIN_INTERLEAVE 0)
set(BUILTIN_POPCNT 0)
# Check if we are on a Linux system
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# Use /proc/cpuinfo to get the information
	file(STRINGS "/proc/cpuinfo" _cpuinfo)
	if(_cpuinfo MATCHES "(avx2)|(bmi2)")
		set(BUILTIN_INTERLEAVE 1)
	endif()
	if(_cpuinfo MATCHES "(popcnt)|(sse4_2)")
		set(BUILTIN_POPCNT 1)
	endif()
endif()	
	
