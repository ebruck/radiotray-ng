if(CREATE_MO)

	# main targets
	add_custom_target(mo-update ALL
			COMMENT "mo-update: Done."
			)

	if(UPDATE_PO)
		# Only set the verbose flag for maintainers.
		set(GETTEXT_MSGFMT_PARAMETER -v)
	endif(UPDATE_PO)

	foreach(i18n_mo_item ${i18n_mo})
		string(REPLACE " => " ";" i18n_mo_item_s ${i18n_mo_item})

		list(GET i18n_mo_item_s 0 mo_filename)
		list(REMOVE_AT i18n_mo_item_s 0)
		string(REPLACE " + " ";" i18n_mo_item_s ${i18n_mo_item_s})

		string (REPLACE "/" "_" SAFE_TARGET_NAME ${mo_filename})

		foreach(LANG ${LANGUAGES})
			foreach(poDir ${i18n_mo_item_s})
				list(APPEND ${SAFE_TARGET_NAME}_${LANG}_src "${poDir}/${LANG}.po")
			endforeach(poDir in ${i18n_mo_item_s})

			message (STATUS "mo ${mo_filename} will include [${${SAFE_TARGET_NAME}_${LANG}_src}] files for  ${LANG}")

			add_custom_command(
					OUTPUT ${PROJECT_BINARY_DIR}/${LANG}/LC_MESSAGES/${mo_filename}

					COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/${LANG}/LC_MESSAGES

					COMMAND ${GETTEXT_MSGCAT_EXECUTABLE}
						-o ${PROJECT_BINARY_DIR}/${LANG}/LC_MESSAGES/${mo_filename}.po
						${${SAFE_TARGET_NAME}_${LANG}_src}
					WORKING_DIRECTORY
						${PROJECT_SOURCE_DIR}
					COMMENT "Creating merged po file ${PROJECT_BINARY_DIR}/${LANG}/LC_MESSAGES/${mo_filename}.po"

					COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
							${GETTEXT_MSGFMT_PARAMETER}
							-o ${PROJECT_BINARY_DIR}/${LANG}/LC_MESSAGES/${mo_filename}
							${PROJECT_BINARY_DIR}/${LANG}/LC_MESSAGES/${mo_filename}.po
					WORKING_DIRECTORY
						${PROJECT_SOURCE_DIR}
					COMMENT "Creating mo file ${PROJECT_BINARY_DIR}/${LANG}/LC_MESSAGES/${mo_filename}"
			)

			list(APPEND mo-update-${SAFE_TARGET_NAME}-SRC
					${PROJECT_BINARY_DIR}/${LANG}/LC_MESSAGES/${mo_filename}
					)
		endforeach(LANG ${LANGUAGES})

		add_custom_target(mo-update-${SAFE_TARGET_NAME}
				COMMENT "mo-update-${SAFE_TARGET_NAME}: Done for ${mo_filename}."
				DEPENDS ${mo-update-${SAFE_TARGET_NAME}-SRC}
				)

		add_dependencies("mo-update" mo-update-${SAFE_TARGET_NAME})
	endforeach(i18n_mo_item ${i18n_mo})

	if(UPDATE_PO)
		add_dependencies("mo-update" po-update)
	endif(UPDATE_PO)

endif(CREATE_MO)
