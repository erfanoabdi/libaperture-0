aperture_build_dir = meson.current_build_dir()


# List headers and sources
libaperture_headers = [
  'aperture.h',
  'aperture-device-manager.h',
  'aperture-utils.h',
  'aperture-viewfinder.h'
]

libaperture_sources = files(
  'pipeline/aperture-pipeline-tee.c',

  'aperture-device-manager.c',
  'aperture-utils.c',
  'aperture-viewfinder.c',
)


libaperture_c_flags = [
  '-DG_LOG_DOMAIN="Aperture"',
  '-D_LIBAPERTURE_COMPILATION',
  '-Wimplicit-fallthrough',
  '-Wdeclaration-after-statement',
]

libaperture_header_install_dir = get_option('includedir') / aperture_library_name
install_headers(libaperture_headers, install_dir: libaperture_header_install_dir)

libaperture_enum_headers = files(
  'aperture-utils.h',
  'aperture-viewfinder.h',
)

libaperture_enums = gnome.mkenums_simple('aperture-enums',
  sources: libaperture_enum_headers,
  header_prefix: '''
#if !defined(_LIBAPERTURE_INSIDE) && !defined(_LIBAPERTURE_COMPILATION)
#error "Only <aperture.h> can be included directly."
#endif''',
  install_header: true,
  install_dir: libaperture_header_install_dir,
)
libaperture_sources += libaperture_enums[0]
libaperture_headers += libaperture_enums[1]


# Create the library
libaperture_lib = shared_library(aperture_library_name,
  libaperture_sources, libaperture_headers,
  dependencies: [ libaperture_deps, libm ],
  c_args: libaperture_c_flags,
  version: aperture_api_version,
  install: true,
  install_dir: true,
)

# Create a dependency to use later
libaperture_dep = declare_dependency(
  link_with: libaperture_lib,
  include_directories: include_directories('.', '..'),
  dependencies: libaperture_deps,
)


# GIR/introspection stuff
libaperture_gir = gnome.generate_gir(libaperture_lib,
  sources: libaperture_headers + libaperture_sources,
  nsversion: aperture_api_version,
  namespace: 'Aperture',
  header: 'aperture.h',
  symbol_prefix: 'aperture',
  identifier_prefix: 'Aperture',
  link_with: libaperture_lib,
  includes: ['Gst-1.0', 'Gtk-3.0'],
  install: true,
  extra_args: [
    '-D_LIBAPERTURE_COMPILATION',
  ]
)


# Pkg-config file
pkgconfig.generate(libaperture_lib,
  name: 'Aperture',
  description: 'The Aperture camera widget library for GTK',
  filebase: aperture_library_name,
  requires: libaperture_deps,
  libraries: libm
)
