/*
 * Copyright (C) 2012 Hannu Mallat <hmallat@gmail.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "nemoimagemetadata.h"

#include <QFile>
#include <QString>
#include <QtEndian>

#define EXIF_ORIENTATION_TOP_LEFT 1
#define EXIF_ORIENTATION_TOP_RIGHT 2
#define EXIF_ORIENTATION_BOTTOM_RIGHT 3
#define EXIF_ORIENTATION_BOTTOM_LEFT 4
#define EXIF_ORIENTATION_LEFT_TOP 5
#define EXIF_ORIENTATION_RIGHT_TOP 6
#define EXIF_ORIENTATION_RIGHT_BOTTOM 7
#define EXIF_ORIENTATION_LEFT_BOTTOM 8

#ifdef HAS_LIBEXIF

extern "C" {
#include <libexif/exif-data.h>
#include <libexif/exif-loader.h>
}

static void exifBrowseContent(ExifContent *c, void *p)
{
    if (c == 0 || p == 0)
        return;

    quint16 *o = static_cast<quint16 *>(p);
    ExifIfd ifd = exif_content_get_ifd(c);
    if (ifd == EXIF_IFD_0) {
        ExifEntry *entry = exif_content_get_entry(c, EXIF_TAG_ORIENTATION);
        if (entry != 0 &&
	    entry->data !=0 &&
            entry->tag == EXIF_TAG_ORIENTATION &&
            entry->format == EXIF_FORMAT_SHORT &&
            entry->components == 1 &&
            entry->size == 2) {
            *o = exif_data_get_byte_order(entry->parent->parent) ==
                    EXIF_BYTE_ORDER_MOTOROLA
                    ? qFromBigEndian<quint16>(entry->data)
                    : qFromLittleEndian<quint16>(entry->data);
        }
    }
}

static quint16 exifOrientation(const QString &filename)
{
    quint16 o = EXIF_ORIENTATION_TOP_LEFT;
    ExifLoader *loader = 0;
    ExifData *exif = 0;

    loader = exif_loader_new();
    if (loader == 0)
        goto exit;

    exif_loader_write_file(loader, QFile::encodeName(filename).constData());
    exif = exif_loader_get_data(loader);
    if (exif == 0)
        goto exit;

    exif_data_foreach_content(exif, exifBrowseContent, &o);
    if (o < EXIF_ORIENTATION_TOP_LEFT || o > EXIF_ORIENTATION_LEFT_BOTTOM)
        o = EXIF_ORIENTATION_TOP_LEFT;

exit:
    if (exif != 0)
        exif_data_unref(exif);
    if (loader != 0)
        exif_loader_unref(loader);

    return o;
}

#else // HAS_LIBEXIF

#define EXIF_TIFF_LSB_MAGIC "Exif\x00\x00II\x2a\x00"
#define EXIF_TIFF_MSB_MAGIC "Exif\x00\x00MM\x00\x2a"
#define EXIF_TIFF_MAGIC_LEN 10

#define TIFF_HEADER_LEN 8
#define TIFF_IFD_ENTRY_LEN 12

#define EXIF_IDENTIFIER_LEN 6

#define EXIF_TYPE_SHORT 3

#define EXIF_TAG_ORIENTATION 0x112

/* Standalone markers without length information */
#define JPEG_MARKER_TEM  0x01
#define JPEG_MARKER_RST0 0xd0
#define JPEG_MARKER_RST1 0xd1
#define JPEG_MARKER_RST2 0xd2
#define JPEG_MARKER_RST3 0xd3
#define JPEG_MARKER_RST4 0xd4
#define JPEG_MARKER_RST5 0xd5
#define JPEG_MARKER_RST6 0xd6
#define JPEG_MARKER_RST7 0xd7
#define JPEG_MARKER_SOI  0xd8
#define JPEG_MARKER_EOI  0xd9

#define JPEG_MARKER_APP1 0xe1

static uchar getMarker(QFile &f)
{
    /* CCITT T.81 Annex B: "All markers are assigned two-byte
       codes: an XFF byte followed by a byte which is not equal
       to 0 or XFF (see Table B.1). Any marker may optionally be
       preceded by any number of fill bytes, which are bytes
       assigned code XFF." */

    char c;

    if (f.getChar(&c) == false || c != -1)
        return 0;

    while (c == -1)
        if (f.getChar(&c) == false)
            return 0;

    if (c == 0) /* Not a marker */
        return 0;

    return static_cast<uchar>(c);
}

static quint16 getMarkerLength(QFile &f)
{
    char buf[2];

    if (f.read(buf, 2) != 2)
        return 0;
    return qFromBigEndian<quint16>(reinterpret_cast<uchar *>(buf));
}

static bool getExifData(QFile &f, QByteArray &data)
{
    uchar marker;
    quint16 len;
    qint64 skip;

    f.seek(0);

    marker = getMarker(f);
    if (marker != JPEG_MARKER_SOI)
        return false;
    while (true) {
        marker = getMarker(f);
        if (marker == 0)
            return false;

        switch (marker) {

        case JPEG_MARKER_SOI: /* shouldn't see this anymore */
        case JPEG_MARKER_EOI: /* end of the line, no EXIF in sight */
            return false;

        case JPEG_MARKER_TEM:
        case JPEG_MARKER_RST0:
        case JPEG_MARKER_RST1:
        case JPEG_MARKER_RST2:
        case JPEG_MARKER_RST3:
        case JPEG_MARKER_RST4:
        case JPEG_MARKER_RST5:
        case JPEG_MARKER_RST6:
        case JPEG_MARKER_RST7:
            /* Standalones, just skip */
            break;

        case JPEG_MARKER_APP1:
            /* CCITT T.81 Annex B:
               The first parameter in a marker segment is
               the two-byte length parameter. This length
               parameter encodes the number of bytes in
               the marker segment, including the length
               parameter and excluding the two-byte
               marker. */
            len = getMarkerLength(f);
            if (len < 2)
                return false;
            data.resize(len - 2);
            if (f.read(data.data(), len - 2) != len - 2)
                return false;
            return true;

        default:
            /* Marker segment, just skip. */
            len = getMarkerLength(f);
            if (len < 2)
                return false;
            skip = f.pos() + static_cast<qint64>(len) - 2;
            f.seek(skip);
            if (f.pos() != skip)
                return false;
            break;
        }
    }
}

static quint16 exifOrientationFromJpeg(const QString &fname)
{
    QByteArray data;
    const uchar *ptr;
    quint32 len;
    quint32 pos;
    bool msbFirst;
    quint32 ifdOff;
    quint16 fieldCount;
    quint16 o = EXIF_ORIENTATION_TOP_LEFT;

    QFile f(fname);
    if (f.open(QIODevice::ReadOnly) == false || getExifData(f, data) == false)
        goto exit;

    ptr = reinterpret_cast<const uchar *>(data.constData());
    len = data.length();
    pos = 0;

    /* 6 bytes for Exif identifier, 8 bytes for TIFF header */
    if (len < EXIF_IDENTIFIER_LEN + TIFF_HEADER_LEN)
        goto exit;

    if (memcmp(ptr + pos, EXIF_TIFF_LSB_MAGIC, EXIF_TIFF_MAGIC_LEN) == 0)
        msbFirst = false;
    else if (memcmp(ptr + pos, EXIF_TIFF_MSB_MAGIC, EXIF_TIFF_MAGIC_LEN) == 0)
        msbFirst = true;
    else
        goto exit;

    ifdOff = msbFirst
        ? qFromBigEndian<quint32>(ptr + pos + EXIF_TIFF_MAGIC_LEN)
        : qFromLittleEndian<quint32>(ptr + pos + EXIF_TIFF_MAGIC_LEN);

    /* IFD offset is measured from TIFF header and can't go backwards */
    if (ifdOff < TIFF_HEADER_LEN)
           return false;

    pos = EXIF_IDENTIFIER_LEN + ifdOff;

    if (len < pos + 2)
        goto exit;
    fieldCount = msbFirst
        ? qFromBigEndian<quint16>(ptr + pos)
        : qFromLittleEndian<quint16>(ptr + pos);
    pos += 2;
    if (len < pos + TIFF_IFD_ENTRY_LEN*static_cast<quint32>(fieldCount))
        goto exit;

    for (quint16 f = 0; f < fieldCount; f++) {
        quint16 tag = msbFirst
            ? qFromBigEndian<quint16>(ptr + pos)
            : qFromLittleEndian<quint16>(ptr + pos);
        quint16 type = msbFirst
            ? qFromBigEndian<quint16>(ptr + pos + 2)
            : qFromLittleEndian<quint16>(ptr + pos + 2);
        quint32 num = msbFirst
            ? qFromBigEndian<quint32>(ptr + pos + 4)
            : qFromLittleEndian<quint32>(ptr + pos + 4);
        if (tag == EXIF_TAG_ORIENTATION &&
            type == EXIF_TYPE_SHORT &&
            num == 1) {
            o = msbFirst
                ? qFromBigEndian<quint16>(ptr + pos + 8)
                : qFromLittleEndian<quint16>(ptr + pos + 8);
            goto exit;
        } else {
            pos += TIFF_IFD_ENTRY_LEN;
        }
    }

    /* We're only interested in the 0th IFD, so quit when at its end */

exit:
    f.close();

    if (o < EXIF_ORIENTATION_TOP_LEFT || o > EXIF_ORIENTATION_LEFT_BOTTOM)
        o = EXIF_ORIENTATION_TOP_LEFT;

    return o;
}

static quint16 exifOrientation(const QString &filename)
{
    if (filename.endsWith(".jpg", Qt::CaseInsensitive) == true ||
        filename.endsWith(".jpeg", Qt::CaseInsensitive) == true) {
        return exifOrientationFromJpeg(filename);
    }

    return EXIF_ORIENTATION_TOP_LEFT;
}

#endif // HAS_LIBEXIF

NemoImageMetadata::NemoImageMetadata()
    : m_orientation(NemoImageMetadata::TopLeft)
{
}

NemoImageMetadata::NemoImageMetadata(const QString &filename)
    : m_orientation(NemoImageMetadata::TopLeft)
{
    m_orientation = static_cast<Orientation>(exifOrientation(filename));
}

NemoImageMetadata::NemoImageMetadata(const NemoImageMetadata &other)
    : m_orientation(other.m_orientation)
{
}

NemoImageMetadata &NemoImageMetadata::operator=(const NemoImageMetadata &other)
{
	if (&other == this)
		return *this;

	m_orientation = other.m_orientation;
	return *this;
}

NemoImageMetadata::~NemoImageMetadata()
{
}
