examples_sources = files(
  'aperture-example-main.c'
)

examples = executable('aperture-demo', examples_sources,
  dependencies: libaperture_dep,
  c_args: '-DHANDY_USE_UNSTABLE_API',
  install: true,
)
