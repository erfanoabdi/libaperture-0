project('aperture', ['c', 'vala'],
          version: '0.1.0',
    meson_version: '>= 0.50.0',
          license: 'LGPL3+',
)


aperture_api_version = '0.1'


gnome = import('gnome')
i18n = import('i18n')
pkgconfig = import('pkgconfig')

cc = meson.get_compiler('c')
g_ir_compiler = find_program('g-ir-compiler')

gio_dep       = dependency('gio-2.0')
glib_dep      = dependency('glib-2.0')
gee_dep       = dependency('gee-0.8')
gdk_dep       = dependency('gdk-3.0')
gdk_dep       = dependency('gdk-x11-3.0')
gtk_dep       = dependency('gtk+-3.0')
hdy_dep       = dependency('libhandy-0.0', version: '>= 0.0.13')
gst_dep       = dependency('gstreamer-1.0')
gst_video_dep = dependency('gstreamer-video-1.0')
gst_app_dep   = dependency('gstreamer-app-1.0')
clutter_gtk_dep = dependency('clutter-gtk-1.0', version: '>= 1.8.1')
clutter_gst_dep = dependency('clutter-gst-3.0', version: '>= 2.99.2')
x11_dep       = dependency('x11')
libm          = cc.find_library('m', required : true)

libaperture_deps = [
  glib_dep,
  gio_dep,
  gee_dep,
  gdk_dep,
  gtk_dep,
  hdy_dep,
  gst_dep,
  gst_video_dep,
  gst_app_dep,
  clutter_gtk_dep,
  clutter_gst_dep,
]

subdir('src')

if get_option('examples')
  subdir('examples')
endif

