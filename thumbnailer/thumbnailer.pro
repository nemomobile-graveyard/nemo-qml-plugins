TEMPLATE = subdirs

SUBDIRS = plugin.pro

packagesExist(gstreamer-0.10 gstreamer-app-0.10): SUBDIRS += gstvideothumbnailer
