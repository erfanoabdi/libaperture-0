project('aperture', ['c'],
          version: '0.1.0',
    meson_version: '>= 0.50.0',
          license: 'LGPL3+',
)


aperture_api_version = '0'
aperture_library_name = 'aperture-@0@'.format(aperture_api_version)


add_project_arguments([
  '-DAPERTURE_COMPILATION',
], language: 'c')

gnome = import('gnome')
i18n = import('i18n')
pkgconfig = import('pkgconfig')

cc = meson.get_compiler('c')
g_ir_compiler = find_program('g-ir-compiler')

gio_dep       = dependency('gio-2.0')
gtk_dep       = dependency('gtk+-3.0')
gst_dep       = dependency('gstreamer-1.0')
gst_video_dep = dependency('gstreamer-video-1.0')
gst_app_dep   = dependency('gstreamer-app-1.0')
libm          = cc.find_library('m', required : true)

libaperture_deps = [
  gio_dep,
  gtk_dep,
  gst_dep,
  gst_video_dep,
  gst_app_dep,
]

subdir('src')
subdir('doc')

if get_option('demo')
  subdir('demo')
endif

subdir('tests')

