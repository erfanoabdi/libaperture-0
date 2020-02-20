aperture_build_dir = meson.current_build_dir()

# Core library
libaperture_gir_name = 'Aperture-@0@'.format(aperture_api_version)

libaperture_sources = files(
  'aperture.c',
  'aperture.vala',
  'aperture-audio-profile.vala',
  'aperture-camera.vala',
  'aperture-camera-switcher-button.vala',
  'aperture-device.vala',
  'aperture-device-manager.vala',
  'aperture-disk-space.vala',
  'aperture-microphone.vala',
  'aperture-video-profile.vala',
  'aperture-widget.vala',
)

libaperture_vala_flags = [
  '--includedir', meson.project_name(),
]

libaperture_c_flags = [
  '-DG_LOG_DOMAIN="aperture"',
]

libaperture_header_install_dir = get_option('includedir') / 'aperture'

libaperture_lib = shared_library('aperture', libaperture_sources,
  dependencies: [ libaperture_deps ],
  vala_args: libaperture_vala_flags,
  c_args: libaperture_c_flags,
  version: aperture_api_version,
  vala_header: 'aperture/aperture.h',
  vala_vapi: libaperture_gir_name + '.vapi',
  vala_gir: libaperture_gir_name + '.gir',
  install: true,
  install_dir: [true, libaperture_header_install_dir, true, true],
)


custom_target(libaperture_gir_name + '.typelib',
  command: [ g_ir_compiler,
    '--output', '@OUTPUT@',
    meson.current_build_dir() / (libaperture_gir_name + '.gir')
  ],
  output: libaperture_gir_name + '.typelib',
  depends: libaperture_lib,
  install: true,
  install_dir: get_option('libdir') / 'girepository-1.0',
)
libaperture_gir_include_dir = meson.current_build_dir()

# Pkg-config file
pkgconfig.generate(libaperture_lib,
  name: 'aperture',
  description: 'The Aperture camera widget library for GTK',
  filebase: 'aperture',
  requires: libaperture_deps,
)

libaperture_dep = declare_dependency(
  link_with: libaperture_lib,
  include_directories: include_directories('.', '..'),
  dependencies: libaperture_deps,
)

