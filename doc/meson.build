if get_option('gtk_doc')


ent_conf = configuration_data({
  'PACKAGE': 'Aperture',
  'PACKAGE_BUGREPORT': 'https://gitlab.gnome.org/jwestman/libaperture/issues',
  'PACKAGE_NAME': 'Aperture',
  'PACKAGE_STRING': 'libaperture',
  'PACKAGE_TARNAME': 'libaperture-' + meson.project_version(),
  'PACKAGE_URL': 'https://gitlab.gnome.org/jwestman/libaperture',
  'PACKAGE_VERSION': meson.project_version(),
  'PACKAGE_API_VERSION': aperture_api_version,
  'PACKAGE_API_NAME': aperture_library_name,
})


configure_file(
  input: 'gtkdocentities.ent.in',
  output: 'gtkdocentities.ent',
  configuration: ent_conf
)


gnome.gtkdoc('aperture',
  main_xml: 'aperture-docs.xml',
  src_dir: [meson.source_root() / 'src', meson.build_root() / 'src'],
  dependencies: libaperture_dep,

  scan_args: [
    '--rebuild-types',
    '--rebuild-sections',
    '--ignore-headers=pipeline private'
  ],

  install_dir: aperture_library_name,
  install: true,
)


endif

