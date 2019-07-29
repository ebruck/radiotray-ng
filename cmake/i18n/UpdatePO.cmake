if(UPDATE_PO)

	# main targets
	add_custom_target(pot-update
			COMMENT "pot-update: Done."
			)
	add_custom_target(po-update
			COMMENT "po-update: Done."
			)

	foreach(i18n_dir ${i18n_po})
		string (REPLACE "/" "_" SAFE_TARGET_NAME ${i18n_dir})
		string (CONCAT i18n_dir_full ${PROJECT_SOURCE_DIR} "/" ${i18n_dir})

		add_custom_command(
			OUTPUT ${i18n_dir_full}/i18n.pot

			COMMAND touch
				${i18n_dir_full}/i18n.pot

			COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
				--add-comments=TRANSLATORS
				--files-from="${i18n_dir_full}/src.txt"
				--copyright-holder=\"radiotray-ng team\"
				--msgid-bugs-address=\"https://github.com/ebruck/radiotray-ng/issues\"
				--from-code=UTF-8
				--keyword=_
				--keyword=N_
				--keyword=F_
				--keyword=F_:2 --flag=F_:2:pass-c-format
				--keyword=FN_
				--output=${i18n_dir_full}/i18n.pot
				--sort-output
				--add-comments=i18n
			# replace the chartype
			COMMAND sed -i
				s/charset=CHARSET/charset=UTF-8/
				${i18n_dir_full}/i18n.pot
			DEPENDS "${i18n_dir_full}/src.txt"
			WORKING_DIRECTORY ${i18n_dir_full}
			COMMENT "pot-update: Generated source pot file in ${i18n_dir_full}."
		)

		add_custom_target(pot-update-${SAFE_TARGET_NAME}
				COMMENT "pot-update: Done in ${i18n_dir_full}."
				DEPENDS  ${i18n_dir_full}/i18n.pot
				)

		add_dependencies("pot-update" pot-update-${SAFE_TARGET_NAME})

		# Generate/Update the po files for all languages
		foreach(LANG ${LANGUAGES})
			### Generate/update po file.
			# For some reason CMake is rather happy to delete the po file in
			# some cases. Too avoid that problem only generate the init rule
			# if the po file doesn't exist. The case where a po file used to
			# exist and no longer exists should never occur
			if(NOT EXISTS ${i18n_dir_full}/${LANG}.po)
				add_custom_command(
					OUTPUT ${i18n_dir_full}/${LANG}.po.dummy
					COMMAND ${GETTEXT_MSGINIT_EXECUTABLE}
							--no-translator
							--input=i18n.pot
							--output-file=${LANG}.po
							--locale=${LANG}
					WORKING_DIRECTORY ${i18n_dir_full}
					DEPENDS
						${i18n_dir_full}/i18n.pot
					COMMENT
						"po-update ${LANG}: Initialized po file in ${i18n_dir_full}."
				)

			else(NOT EXISTS ${i18n_dir_full}/${LANG}.po)
				add_custom_command(
						OUTPUT ${i18n_dir_full}/${LANG}.po.dummy
						# After the po file is updated it might look not entirely as
						# wanted, for example poedit reorders the file. Use msgattrib
						# to reformat the file, use a helper file to do so.
						COMMAND ${GETTEXT_MSGATTRIB_EXECUTABLE}
								--output ${LANG}.po.tmp
								${LANG}.po
						COMMAND ${CMAKE_COMMAND} -E copy
								${LANG}.po.tmp
								${LANG}.po
						COMMAND ${CMAKE_COMMAND} -E remove
								${LANG}.po.tmp
						# Now merge with the pot file.
						COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
								--backup=none
								--update
								${LANG}.po
								i18n.pot
						WORKING_DIRECTORY ${i18n_dir_full}
						DEPENDS
							${i18n_dir_full}/i18n.pot
						COMMENT
							"po-update ${LANG}: Update po file in ${i18n_dir_full}."
				)
			endif(NOT EXISTS ${i18n_dir_full}/${LANG}.po)

			SET(po-update-${SAFE_TARGET_NAME}-SRC
					${po-update-${SAFE_TARGET_NAME}-SRC}
					${i18n_dir_full}/${LANG}.po.dummy
					)
		endforeach(LANG ${LANGUAGES})


		add_custom_target(po-update-${SAFE_TARGET_NAME}
				COMMENT "po-update: Done in ${i18n_dir_full}."
				DEPENDS ${po-update-${SAFE_TARGET_NAME}-SRC}
				)

		add_dependencies("po-update" po-update-${SAFE_TARGET_NAME})
	endforeach(i18n_dir in ${i18n_po})

endif(UPDATE_PO)
