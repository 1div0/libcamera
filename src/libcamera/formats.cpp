/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * formats.cpp - libcamera image formats
 */

#include "formats.h"

#include <errno.h>

#include "log.h"

/**
 * \file formats.h
 * \brief Types and helper methods to handle libcamera image formats
 */

namespace libcamera {

LOG_DEFINE_CATEGORY(Formats)

/**
 * \class ImageFormats
 * \brief Describe V4L2Device and V4L2SubDevice image formats
 *
 * This class stores a list of image formats, each associated with a
 * corresponding set of image sizes. It is used to describe the formats and
 * sizes supported by a V4L2Device or V4L2Subdevice.
 *
 * Formats are stored as an integer. When used for a V4L2Device, the image
 * formats are fourcc pixel formats. When used for a V4L2Subdevice they are
 * media bus codes. Both are defined by the V4L2 specification.
 *
 * Sizes are stored as a list of SizeRange.
 */

/**
 * \brief Add a format and corresponding sizes to the description
 * \param[in] format Pixel format or media bus code to describe
 * \param[in] sizes List of supported size ranges for the format
 *
 * \return 0 on success or a negative error code otherwise
 * \retval -EEXIST The format is already described
 */
int ImageFormats::addFormat(unsigned int format, const std::vector<SizeRange> &sizes)
{
	if (data_.find(format) != data_.end())
		return -EEXIST;

	data_[format] = sizes;

	return 0;
}

/**
 * \brief Check if the list of devices supported formats is empty
 * \return True if the list of supported formats is empty
 */
bool ImageFormats::isEmpty() const
{
	return data_.empty();
}

/**
 * \brief Retrieve a list of all supported image formats
 * \return List of pixel formats or media bus codes
 */
std::vector<unsigned int> ImageFormats::formats() const
{
	std::vector<unsigned int> formats;
	formats.reserve(data_.size());

	/* \todo: Should this be cached instead of computed each time? */
	for (auto const &it : data_)
		formats.push_back(it.first);

	return formats;
}

/**
 * \brief Retrieve all sizes for a specific format
 * \param[in] format The pixel format or mbus code
 *
 * Retrieve all size ranges for a specific format. For V4L2Device \a format is a
 * pixel format while for a V4L2Subdevice \a format is a media bus code.
 *
 * \return The list of image sizes supported for \a format, or an empty list if
 * the format is not supported
 */
const std::vector<SizeRange> &ImageFormats::sizes(unsigned int format) const
{
	static const std::vector<SizeRange> empty;

	auto const &it = data_.find(format);
	if (it == data_.end())
		return empty;

	return it->second;
}

/**
 * \brief Retrieve the map that associates formats to image sizes
 * \return The map that associates formats to image sizes
 */
const std::map<unsigned int, std::vector<SizeRange>> &ImageFormats::data() const
{
	return data_;
}

/**
 * \class PixelFormatInfo
 * \brief Information about pixel formats
 *
 * The PixelFormatInfo class groups together information describing a pixel
 * format. It facilitates handling of pixel formats by providing data commonly
 * used in pipeline handlers.
 *
 * \var PixelFormatInfo::format
 * \brief The PixelFormat described by this instance
 *
 * \var PixelFormatInfo::v4l2Format
 * \brief The V4L2 pixel format corresponding to the PixelFormat
 *
 * \var PixelFormatInfo::bitsPerPixel
 * \brief The average number of bits per pixel
 *
 * The number per pixel averages the total number of bits for all colour
 * components over the whole image, excluding any padding bits or padding
 * pixels.
 *
 * For formats that store pixels with bit padding within words, only the
 * effective bits are taken into account. For instance, 12-bit Bayer data
 * stored in two bytes per pixel report 12, not 16, in this field.
 *
 * Formats that don't have a fixed number of bits per pixel, such as compressed
 * formats, report 0 in this field.
 *
 * \var PixelFormatInfo::colourEncoding
 * \brief The colour encoding type
 *
 * \var PixelFormatInfo::packed
 * \brief Tell if multiple pixels are packed in the same bytes
 *
 * Packed formats are defined as storing data from multiple pixels in the same
 * bytes. For instance, 12-bit Bayer data with two pixels stored in three bytes
 * is packed, while the same data stored with 4 bits of padding in two bytes
 * per pixel is not packed.
 */

/**
 * \enum PixelFormatInfo::ColourEncoding
 * \brief The colour encoding type
 *
 * \var PixelFormatInfo::ColourEncodingRGB
 * \brief RGB colour encoding
 *
 * \var PixelFormatInfo::ColourEncodingYUV
 * \brief YUV colour encoding
 *
 * \var PixelFormatInfo::ColourEncodingRAW
 * \brief RAW colour encoding
 */

namespace {

const std::map<PixelFormat, PixelFormatInfo> pixelFormatInfo{
	/* RGB formats. */
	{ PixelFormat(DRM_FORMAT_BGR888), {
		.format = PixelFormat(DRM_FORMAT_BGR888),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_RGB24),
		.bitsPerPixel = 24,
		.colourEncoding = PixelFormatInfo::ColourEncodingRGB,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_RGB888), {
		.format = PixelFormat(DRM_FORMAT_RGB888),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_BGR24),
		.bitsPerPixel = 24,
		.colourEncoding = PixelFormatInfo::ColourEncodingRGB,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_ABGR8888), {
		.format = PixelFormat(DRM_FORMAT_ABGR8888),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_RGBA32),
		.bitsPerPixel = 32,
		.colourEncoding = PixelFormatInfo::ColourEncodingRGB,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_ARGB8888), {
		.format = PixelFormat(DRM_FORMAT_ARGB8888),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_ABGR32),
		.bitsPerPixel = 32,
		.colourEncoding = PixelFormatInfo::ColourEncodingRGB,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_BGRA8888), {
		.format = PixelFormat(DRM_FORMAT_BGRA8888),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_ARGB32),
		.bitsPerPixel = 32,
		.colourEncoding = PixelFormatInfo::ColourEncodingRGB,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_RGBA8888), {
		.format = PixelFormat(DRM_FORMAT_RGBA8888),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_BGRA32),
		.bitsPerPixel = 32,
		.colourEncoding = PixelFormatInfo::ColourEncodingRGB,
		.packed = false,
	} },

	/* YUV packed formats. */
	{ PixelFormat(DRM_FORMAT_YUYV), {
		.format = PixelFormat(DRM_FORMAT_YUYV),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_YUYV),
		.bitsPerPixel = 16,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_YVYU), {
		.format = PixelFormat(DRM_FORMAT_YVYU),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_YVYU),
		.bitsPerPixel = 16,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_UYVY), {
		.format = PixelFormat(DRM_FORMAT_UYVY),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_UYVY),
		.bitsPerPixel = 16,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_VYUY), {
		.format = PixelFormat(DRM_FORMAT_VYUY),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_VYUY),
		.bitsPerPixel = 16,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },

	/* YUV planar formats. */
	{ PixelFormat(DRM_FORMAT_NV16), {
		.format = PixelFormat(DRM_FORMAT_NV16),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_NV16),
		.bitsPerPixel = 16,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_NV61), {
		.format = PixelFormat(DRM_FORMAT_NV61),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_NV61),
		.bitsPerPixel = 16,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_NV12), {
		.format = PixelFormat(DRM_FORMAT_NV12),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_NV12),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_NV21), {
		.format = PixelFormat(DRM_FORMAT_NV21),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_NV21),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },

	/* Greyscale formats. */
	{ PixelFormat(DRM_FORMAT_R8), {
		.format = PixelFormat(DRM_FORMAT_R8),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_GREY),
		.bitsPerPixel = 8,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },

	/* Bayer formats. */
	{ PixelFormat(DRM_FORMAT_SBGGR8), {
		.format = PixelFormat(DRM_FORMAT_SBGGR8),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SBGGR8),
		.bitsPerPixel = 8,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SGBRG8), {
		.format = PixelFormat(DRM_FORMAT_SGBRG8),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGBRG8),
		.bitsPerPixel = 8,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SGRBG8), {
		.format = PixelFormat(DRM_FORMAT_SGRBG8),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGRBG8),
		.bitsPerPixel = 8,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SRGGB8), {
		.format = PixelFormat(DRM_FORMAT_SRGGB8),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SRGGB8),
		.bitsPerPixel = 8,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SBGGR10), {
		.format = PixelFormat(DRM_FORMAT_SBGGR10),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SBGGR10),
		.bitsPerPixel = 10,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SGBRG10), {
		.format = PixelFormat(DRM_FORMAT_SGBRG10),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGBRG10),
		.bitsPerPixel = 10,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SGRBG10), {
		.format = PixelFormat(DRM_FORMAT_SGRBG10),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGRBG10),
		.bitsPerPixel = 10,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SRGGB10), {
		.format = PixelFormat(DRM_FORMAT_SRGGB10),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SRGGB10),
		.bitsPerPixel = 10,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SBGGR10, MIPI_FORMAT_MOD_CSI2_PACKED), {
		.format = PixelFormat(DRM_FORMAT_SBGGR10, MIPI_FORMAT_MOD_CSI2_PACKED),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SBGGR10P),
		.bitsPerPixel = 10,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = true,
	} },
	{ PixelFormat(DRM_FORMAT_SGBRG10, MIPI_FORMAT_MOD_CSI2_PACKED), {
		.format = PixelFormat(DRM_FORMAT_SGBRG10, MIPI_FORMAT_MOD_CSI2_PACKED),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGBRG10P),
		.bitsPerPixel = 10,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = true,
	} },
	{ PixelFormat(DRM_FORMAT_SGRBG10, MIPI_FORMAT_MOD_CSI2_PACKED), {
		.format = PixelFormat(DRM_FORMAT_SGRBG10, MIPI_FORMAT_MOD_CSI2_PACKED),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGRBG10P),
		.bitsPerPixel = 10,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = true,
	} },
	{ PixelFormat(DRM_FORMAT_SRGGB10, MIPI_FORMAT_MOD_CSI2_PACKED), {
		.format = PixelFormat(DRM_FORMAT_SRGGB10, MIPI_FORMAT_MOD_CSI2_PACKED),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SRGGB10P),
		.bitsPerPixel = 10,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = true,
	} },
	{ PixelFormat(DRM_FORMAT_SBGGR12), {
		.format = PixelFormat(DRM_FORMAT_SBGGR12),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SBGGR12),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SGBRG12), {
		.format = PixelFormat(DRM_FORMAT_SGBRG12),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGBRG12),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SGRBG12), {
		.format = PixelFormat(DRM_FORMAT_SGRBG12),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGRBG12),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SRGGB12), {
		.format = PixelFormat(DRM_FORMAT_SRGGB12),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SRGGB12),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = false,
	} },
	{ PixelFormat(DRM_FORMAT_SBGGR12, MIPI_FORMAT_MOD_CSI2_PACKED), {
		.format = PixelFormat(DRM_FORMAT_SBGGR12, MIPI_FORMAT_MOD_CSI2_PACKED),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SBGGR12P),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = true,
	} },
	{ PixelFormat(DRM_FORMAT_SGBRG12, MIPI_FORMAT_MOD_CSI2_PACKED), {
		.format = PixelFormat(DRM_FORMAT_SGBRG12, MIPI_FORMAT_MOD_CSI2_PACKED),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGBRG12P),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = true,
	} },
	{ PixelFormat(DRM_FORMAT_SGRBG12, MIPI_FORMAT_MOD_CSI2_PACKED), {
		.format = PixelFormat(DRM_FORMAT_SGRBG12, MIPI_FORMAT_MOD_CSI2_PACKED),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SGRBG12P),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = true,
	} },
	{ PixelFormat(DRM_FORMAT_SRGGB12, MIPI_FORMAT_MOD_CSI2_PACKED), {
		.format = PixelFormat(DRM_FORMAT_SRGGB12, MIPI_FORMAT_MOD_CSI2_PACKED),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_SRGGB12P),
		.bitsPerPixel = 12,
		.colourEncoding = PixelFormatInfo::ColourEncodingRAW,
		.packed = true,
	} },

	/* Compressed formats. */
	{ PixelFormat(DRM_FORMAT_MJPEG), {
		.format = PixelFormat(DRM_FORMAT_MJPEG),
		.v4l2Format = V4L2PixelFormat(V4L2_PIX_FMT_MJPEG),
		.bitsPerPixel = 0,
		.colourEncoding = PixelFormatInfo::ColourEncodingYUV,
		.packed = false,
	} },
};

} /* namespace */

/**
 * \fn bool PixelFormatInfo::isValid() const
 * \brief Check if the pixel format info is valid
 * \return True if the pixel format info is valid, false otherwise
 */

/**
 * \brief Retrieve information about a pixel format
 * \param[in] format The pixel format
 * \return The PixelFormatInfo describing the \a format if known, or an invalid
 * PixelFormatInfo otherwise
 */
const PixelFormatInfo &PixelFormatInfo::info(const PixelFormat &format)
{
	static const PixelFormatInfo invalid{};

	const auto iter = pixelFormatInfo.find(format);
	if (iter == pixelFormatInfo.end()) {
		LOG(Formats, Warning)
			<< "Unsupported pixel format "
			<< format.toString();
		return invalid;
	}

	return iter->second;
}

} /* namespace libcamera */
