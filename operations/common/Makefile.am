SUBDIRS = perlin

include $(top_srcdir)/operations/Makefile-common.am

EXTRA_DIST = $(wildcard $(srcdir)/*.h)

AM_CPPFLAGS += -I$(srcdir)
AM_CFLAGS +=  -DGEGL_OP_BUNDLE
LIBS = $(op_libs)


opdir = $(ext_dir)
op_LTLIBRARIES = \
	gegl-common.la

gegl_common_la_SOURCES =\
	module.c \
	absolute.c \
	alien-map.c \
	bilateral-filter.c \
	box-blur.c \
	brightness-contrast.c \
	buffer-sink.c \
	buffer-source.c \
	c2g.c \
	checkerboard.c \
	dither.c \
	color-enhance.c \
	color-overlay.c \
	color-rotate.c \
	color-temperature.c \
	color-warp.c \
	color.c \
	component-extract.c \
	contrast-curve.c \
	convolution-matrix.c \
	copy-buffer.c \
	difference-of-gaussians.c \
	display.c \
	dropshadow.c \
	edge-neon.c \
	edge-sobel.c \
	exp-combine.c \
	exposure.c \
	fattal02.c \
	gaussian-blur.c \
	gblur-1d.c \
	gegl-buffer-load-op.c \
	gegl-buffer-save-op.c \
	gegl.c \
	grey.c \
	grid.c \
	high-pass.c \
	hue-chroma.c \
	icc-save.c \
	image-compare.c \
	image-gradient.c \
	introspect.c \
	invert-common.h \
	invert-gamma.c \
	invert-linear.c \
	layer.c \
	levels.c \
	linear-gradient.c \
	little-planet.c \
	long-shadow.c \
	magick-load.c \
	mantiuk06.c \
	map-absolute.c \
	map-common.h \
	map-relative.c \
	matting-global.c \
	mblur.c \
	mean-curvature-blur.c \
	median-blur.c \
	mirrors.c \
	mix.c \
	mono-mixer.c \
	motion-blur-linear.c \
	newsprint.c \
	noise-cie-lch.c \
	noise-hsv.c \
	noise-hurl.c \
	noise-perlin.c \
	noise-pick.c \
	noise-reduction.c \
	noise-rgb.c \
	noise-simplex.c \
	noise-spread.c \
	noise-cell.c \
	opacity.c \
	open-buffer.c \
	over.c \
	panorama-projection.c \
	pixelize.c \
	posterize.c \
	radial-gradient.c \
	rectangle.c \
	recursive-transform.c \
	reinhard05.c \
	remap.c \
	rgb-clip.c \
	saturation.c \
	save.c \
	sepia.c \
	slic.c \
	snn-mean.c \
	spherize.c \
	stress.c \
	stretch-contrast-hsv.c \
	stretch-contrast.c \
	svg-huerotate.c \
	svg-luminancetoalpha.c \
	svg-matrix.c \
	svg-saturate.c \
	threshold.c \
	tile.c \
	tile-seamless.c \
	unsharp-mask.c \
	unpremultiply.c \
	value-invert.c \
	vignette.c \
	waterpixels.c \
	watershed-transform.c \
	wavelet-blur-1d.c \
	wavelet-blur.c \
	weighted-blend.c \
	write-buffer.c

module.c: Makefile $(gegl_common_la_SOURCES)
	$(AM_V_GEN) \
	$(top_srcdir)/tools/gen-loader.sh `echo $^ | cut -d' ' -f2-` > module.c

CLEANFILES = module.c
