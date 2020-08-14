aperture_build_dir = meson.current_build_dir()


# List headers and sources
libaperture_headers = [
  'aperture.h',
  'aperture-device-manager.h',
  'aperture-utils.h',
  'aperture-viewfinder.h'
]
libaperture_generated_headers = []

libaperture_sources = files(
  'pipeline/aperture-pipeline-tee.c',

  'aperture-device-manager.c',
  'aperture-utils.c',
  'aperture-viewfinder.c',
)


configure_file(
  output: 'aperture-build-info.h',
  configuration: {
    'APERTURE_MAJOR_VERSION': aperture_major,
    'APERTURE_MINOR_VERSION': aperture_minor,
    'APERTURE_MICRO_VERSION': aperture_micro,
  }
)
libaperture_generated_headers += ['aperture-build-info.h']


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
libaperture_generated_headers += libaperture_enums[1]


# Create the library
libaperture_lib = shared_library(aperture_library_name,
  libaperture_sources, libaperture_headers,
  dependencies: [ libaperture_deps ],
  c_args: libaperture_c_flags,
  version: aperture_api_version,
  install: true,
  install_dir: true,
)

# Create a dependency to use later
libaperture_dep = declare_dependency(
  sources: libaperture_headers,
  link_with: libaperture_lib,
  include_directories: include_directories('.', '..'),
  dependencies: libaperture_deps,
)


if get_option('gir')
  # GIR/introspection stuff
  libaperture_gir = gnome.generate_gir(libaperture_lib,
    sources: libaperture_headers + libaperture_sources,
    nsversion: aperture_api_version,
    export_packages: aperture_library_name,
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

  if get_option('vapi')
    gnome.generate_vapi(aperture_library_name,
      sources: libaperture_gir[0],
      packages: [ 'gtk+-3.0', 'gio-2.0', 'gstreamer-1.0' ],
      install: true,
      metadata_dirs: [meson.current_source_dir()],
    )
  endif
endif

# Pkg-config file
pkgconfig.generate(libaperture_lib,
  name: 'Aperture',
  description: 'The Aperture camera widget library for GTK',
  subdirs: aperture_library_name,
  filebase: aperture_library_name,
  requires: libaperture_deps,
)
