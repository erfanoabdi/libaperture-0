demo_sources = files(
  'aperture-demo.c',
  'aperture-demo-window.c',
)

demo_resources = gnome.compile_resources(
  'aperture-demo-resources',
  'demo.gresources.xml',
)

demo = executable('aperture-demo', demo_sources, demo_resources,
  dependencies: libaperture_dep,
  install: true,
)
