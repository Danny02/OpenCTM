function(do_pdflatex target)
	add_custom_target(${target} ALL
		pdflatex -output-directory ${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/${target}.tex COMMAND
		pdflatex -output-directory ${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/${target}.tex
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${target}.tex
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
endfunction()

do_pdflatex(DevelopersManual)
do_pdflatex(FormatSpecification)

# TODO: It should be out-of-tree build
add_custom_target(Doxygen ALL
	doxygen ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../lib/openctm.h
	${CMAKE_CURRENT_SOURCE_DIR}/../lib/openctmpp.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if(${BUILD_TOOLS})
	add_custom_target(Manpages_HTML ALL
		groff -t -mandoc -Thtml ${CMAKE_CURRENT_SOURCE_DIR}/ctmconv.1 > ctmconv.html COMMAND
		groff -t -mandoc -Thtml ${CMAKE_CURRENT_SOURCE_DIR}/ctmviewer.1 > ctmviewer.html
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ctmconv.1
		${CMAKE_CURRENT_SOURCE_DIR}/ctmviewer.1
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)

	set(MANPAGE_PATH ${CMAKE_INSTALL_PREFIX}/share/man/man1 CACHE PATH)

	install(FILES ctmconv.1 ctmviewer.1
		DESTINATION ${MANPAGE_PATH}
	)
endif()
