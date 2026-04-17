
file(GLOB_RECURSE additional_source
	./include/**.h)
	
list (APPEND application_source ${additional_source})