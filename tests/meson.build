test_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  'G_DEBUG=fatal-warnings',
  'MALLOC_CHECK_=2',
]

test_sources = files(
  'dummy-device.c',
  'dummy-device-provider.c',

  'main.c',

  'test-barcodes.c',
  'test-device-manager.c',

  'utils.c',
)

test_executable = executable('aperture-tests',
  test_sources,
  dependencies: libaperture_dep,
  install: true,
)

test('aperture-tests',
  test_executable,
  env: test_env,
)
