project('lsml', 'cpp')

ncurses_dep = dependency('ncurses')
curl_dep = dependency('libcurl')
libarchive_dep = dependency('libarchive')

executable('lsml',
	['lsml.cpp', 'archives.cpp'],
	dependencies: [ncurses_dep, curl_dep, libarchive_dep],
	install: true)
