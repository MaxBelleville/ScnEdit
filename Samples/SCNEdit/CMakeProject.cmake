
file(GLOB_RECURSE additional_source
	./Header/**.h)
	
list (APPEND application_source ${additional_source})