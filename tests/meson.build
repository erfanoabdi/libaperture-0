test_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
]

test_sources = files(
  'dummy-device.c',
  'dummy-device-provider.c',

  'main.c',

  'test-barcodes.c',
  'test-device-manager.c',
  'test-viewfinder.c',

  'utils.c',
)

subdir('data')
test_sources += test_gresources

test_c_args = []
if get_option('barcode_tests_skippable') == 'true'
  test_c_args += '-DBARCODE_TESTS_SKIPPABLE'
endif

test_executable = executable('aperture-tests',
  test_sources,
  dependencies: libaperture_dep,
  c_args: test_c_args,
  install: true,
)

test('aperture-tests',
  test_executable,
  env: test_env,
)


# We could use the built-in, default coverage reports. However, doing it
# manually here allows us to filter the files, so that only files under src/
# are included.
# Otherwise, the tests themselves, the demo project, and some generated source
# files would skew the results.

# This could also be done using a gcovr.cfg file, but meson will use lcov if
# it's installed, which does not obey that file.
if get_option('b_coverage')
  test_coverage = run_target('test-coverage', command: ['coverage.sh'])
endif
