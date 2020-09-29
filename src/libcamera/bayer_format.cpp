/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Limited
 *
 * bayer_format.cpp - Class to represent Bayer formats
 */

#include "libcamera/internal/bayer_format.h"

#include <map>

#include <libcamera/transform.h>

/**
 * \file bayer_format.h
 * \brief Class to represent Bayer formats and manipulate them
 */

namespace libcamera {

/**
 * \class BayerFormat
 * \brief Class to represent a raw image Bayer format
 *
 * This class encodes the different Bayer formats in such a way that they can
 * be easily manipulated. For example, the bit depth or Bayer order can be
 * easily altered - the Bayer order can even be "transformed" in the same
 * manner as happens in many sensors when their horizontal or vertical "flip"
 * controls are set.
 */

/**
 * \enum BayerFormat::Order
 * \brief The order of the colour channels in the Bayer pattern
 *
 * \var BayerFormat::BGGR
 * \brief B then G on the first row, G then R on the second row.
 * \var BayerFormat::GBRG
 * \brief G then B on the first row, R then G on the second row.
 * \var BayerFormat::GRBG
 * \brief G then R on the first row, B then G on the second row.
 * \var BayerFormat::RGGB
 * \brief R then G on the first row, G then B on the second row.
 */

/**
 * \enum BayerFormat::Packing
 * \brief Different types of packing that can be applied to a BayerFormat
 *
 * \var BayerFormat::None
 * \brief No packing
 * \var BayerFormat::CSI2Packed
 * \brief Format uses MIPI CSI-2 style packing
 * \var BayerFormat::IPU3Packed
 * \brief Format uses IPU3 style packing
 */

namespace {

const std::map<V4L2PixelFormat, BayerFormat> v4l2ToBayer{
	{ V4L2PixelFormat(V4L2_PIX_FMT_SBGGR8), { BayerFormat::BGGR, 8, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGBRG8), { BayerFormat::GBRG, 8, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGRBG8), { BayerFormat::GRBG, 8, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SRGGB8), { BayerFormat::RGGB, 8, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SBGGR10), { BayerFormat::BGGR, 10, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGBRG10), { BayerFormat::GBRG, 10, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGRBG10), { BayerFormat::GRBG, 10, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SRGGB10), { BayerFormat::RGGB, 10, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SBGGR10P), { BayerFormat::BGGR, 10, BayerFormat::CSI2Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGBRG10P), { BayerFormat::GBRG, 10, BayerFormat::CSI2Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGRBG10P), { BayerFormat::GRBG, 10, BayerFormat::CSI2Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SRGGB10P), { BayerFormat::RGGB, 10, BayerFormat::CSI2Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_IPU3_SBGGR10), { BayerFormat::BGGR, 10, BayerFormat::IPU3Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_IPU3_SGBRG10), { BayerFormat::GBRG, 10, BayerFormat::IPU3Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_IPU3_SGRBG10), { BayerFormat::GRBG, 10, BayerFormat::IPU3Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_IPU3_SRGGB10), { BayerFormat::RGGB, 10, BayerFormat::IPU3Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SBGGR12), { BayerFormat::BGGR, 12, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGBRG12), { BayerFormat::GBRG, 12, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGRBG12), { BayerFormat::GRBG, 12, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SRGGB12), { BayerFormat::RGGB, 12, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SBGGR12P), { BayerFormat::BGGR, 12, BayerFormat::CSI2Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGBRG12P), { BayerFormat::GBRG, 12, BayerFormat::CSI2Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGRBG12P), { BayerFormat::GRBG, 12, BayerFormat::CSI2Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SRGGB12P), { BayerFormat::RGGB, 12, BayerFormat::CSI2Packed } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SBGGR16), { BayerFormat::BGGR, 16, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGBRG16), { BayerFormat::GBRG, 16, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SGRBG16), { BayerFormat::GRBG, 16, BayerFormat::None } },
	{ V4L2PixelFormat(V4L2_PIX_FMT_SRGGB16), { BayerFormat::RGGB, 16, BayerFormat::None } },
};

/* Define a slightly arbitrary ordering so that we can use a std::map. */
struct BayerFormatComparator {
	constexpr bool operator()(const BayerFormat &lhs, const BayerFormat &rhs) const
	{
		if (lhs.bitDepth < rhs.bitDepth)
			return true;
		else if (lhs.bitDepth > rhs.bitDepth)
			return false;

		if (lhs.order < rhs.order)
			return true;
		else if (lhs.order > rhs.order)
			return false;

		if (lhs.packing < rhs.packing)
			return true;
		else
			return false;
	}
};

const std::map<BayerFormat, V4L2PixelFormat, BayerFormatComparator> bayerToV4l2{
	{ { BayerFormat::BGGR, 8, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SBGGR8) },
	{ { BayerFormat::GBRG, 8, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SGBRG8) },
	{ { BayerFormat::GRBG, 8, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SGRBG8) },
	{ { BayerFormat::RGGB, 8, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SRGGB8) },
	{ { BayerFormat::BGGR, 10, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SBGGR10) },
	{ { BayerFormat::GBRG, 10, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SGBRG10) },
	{ { BayerFormat::GRBG, 10, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SGRBG10) },
	{ { BayerFormat::RGGB, 10, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SRGGB10) },
	{ { BayerFormat::BGGR, 10, BayerFormat::CSI2Packed }, V4L2PixelFormat(V4L2_PIX_FMT_SBGGR10P) },
	{ { BayerFormat::GBRG, 10, BayerFormat::CSI2Packed }, V4L2PixelFormat(V4L2_PIX_FMT_SGBRG10P) },
	{ { BayerFormat::GRBG, 10, BayerFormat::CSI2Packed }, V4L2PixelFormat(V4L2_PIX_FMT_SGRBG10P) },
	{ { BayerFormat::RGGB, 10, BayerFormat::CSI2Packed }, V4L2PixelFormat(V4L2_PIX_FMT_SRGGB10P) },
	{ { BayerFormat::BGGR, 10, BayerFormat::IPU3Packed }, V4L2PixelFormat(V4L2_PIX_FMT_IPU3_SBGGR10) },
	{ { BayerFormat::GBRG, 10, BayerFormat::IPU3Packed }, V4L2PixelFormat(V4L2_PIX_FMT_IPU3_SGBRG10) },
	{ { BayerFormat::GRBG, 10, BayerFormat::IPU3Packed }, V4L2PixelFormat(V4L2_PIX_FMT_IPU3_SGRBG10) },
	{ { BayerFormat::RGGB, 10, BayerFormat::IPU3Packed }, V4L2PixelFormat(V4L2_PIX_FMT_IPU3_SRGGB10) },
	{ { BayerFormat::BGGR, 12, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SBGGR12) },
	{ { BayerFormat::GBRG, 12, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SGBRG12) },
	{ { BayerFormat::GRBG, 12, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SGRBG12) },
	{ { BayerFormat::RGGB, 12, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SRGGB12) },
	{ { BayerFormat::BGGR, 12, BayerFormat::CSI2Packed }, V4L2PixelFormat(V4L2_PIX_FMT_SBGGR12P) },
	{ { BayerFormat::GBRG, 12, BayerFormat::CSI2Packed }, V4L2PixelFormat(V4L2_PIX_FMT_SGBRG12P) },
	{ { BayerFormat::GRBG, 12, BayerFormat::CSI2Packed }, V4L2PixelFormat(V4L2_PIX_FMT_SGRBG12P) },
	{ { BayerFormat::RGGB, 12, BayerFormat::CSI2Packed }, V4L2PixelFormat(V4L2_PIX_FMT_SRGGB12P) },
	{ { BayerFormat::BGGR, 16, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SBGGR16) },
	{ { BayerFormat::GBRG, 16, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SGBRG16) },
	{ { BayerFormat::GRBG, 16, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SGRBG16) },
	{ { BayerFormat::RGGB, 16, BayerFormat::None }, V4L2PixelFormat(V4L2_PIX_FMT_SRGGB16) },
};

} /* namespace */

/**
 * \fn BayerFormat::BayerFormat()
 * \brief Construct an empty (and invalid) BayerFormat
 */

/**
 * \fn BayerFormat::BayerFormat(Order o, uint8_t b, Packing p)
 * \brief Construct a BayerFormat from explicit values
 * \param[in] o The order of the Bayer pattern
 * \param[in] b The bit depth of the Bayer samples
 * \param[in] p The type of packing applied to the pixel values
 */

/**
 * \brief Construct a BayerFormat from a V4L2PixelFormat
 * \param[in] v4l2Format The raw format to convert into a BayerFormat
 */
BayerFormat::BayerFormat(V4L2PixelFormat v4l2Format)
	: order(BGGR), packing(None)
{
	const auto it = v4l2ToBayer.find(v4l2Format);
	if (it == v4l2ToBayer.end())
		bitDepth = 0;
	else
		*this = it->second;
}

/**
 * \fn BayerFormat::isValid()
 * \brief Return whether a BayerFormat is valid
 */

/**
 * \brief Assemble and return a readable string representation of the
 * BayerFormat
 * \return A string describing the BayerFormat
 */
std::string BayerFormat::toString() const
{
	std::string result;

	static const char *orderStrings[] = {
		"BGGR",
		"GBRG",
		"GRBG",
		"RGGB"
	};
	if (isValid() && order <= RGGB)
		result = orderStrings[order];
	else
		return "INVALID";

	result += "-" + std::to_string(bitDepth);

	if (packing == CSI2Packed)
		result += "-CSI2P";
	else if (packing == IPU3Packed)
		result += "-IPU3P";

	return result;
}

/**
 * \brief Convert a BayerFormat into the corresponding V4L2PixelFormat
 * \return The V4L2PixelFormat corresponding to this BayerFormat
 */
V4L2PixelFormat BayerFormat::toV4L2PixelFormat() const
{
	const auto it = bayerToV4l2.find(*this);
	if (it != bayerToV4l2.end())
		return it->second;

	return V4L2PixelFormat();
}

/**
 * \brief Apply a transform to this BayerFormat
 * \param[in] t The transform to apply
 *
 * Appplying a transform to an image stored in a Bayer format affects the Bayer
 * order. For example, performing a horizontal flip on the Bayer pattern
 * RGGB causes the RG rows of pixels to become GR, and the GB rows to become BG.
 * The transformed image would have a GRBG order. The bit depth and modifiers
 * are not affected.
 *
 * Note that transpositions are ignored as the order of a transpose with
 * respect to the flips would have to be defined, and sensors are not expected
 * to support transposition.
 *
 * \return The transformed Bayer format
 */
BayerFormat BayerFormat::transform(Transform t) const
{
	BayerFormat result = *this;

	/*
	 * Observe that flipping bit 0 of the Order enum performs a horizontal
	 * mirror on the Bayer pattern (e.g. RGGB goes to GRBG). Similarly,
	 * flipping bit 1 performs a vertical mirror operation on it. Hence:
	 */
	if (!!(t & Transform::HFlip))
		result.order = static_cast<Order>(result.order ^ 1);
	if (!!(t & Transform::VFlip))
		result.order = static_cast<Order>(result.order ^ 2);

	return result;
}

/**
 * \var BayerFormat::order
 * \brief The order of the colour channels in the Bayer pattern
 */

/**
 * \var BayerFormat::bitDepth
 * \brief The bit depth of the samples in the Bayer pattern
 */

/**
 * \var BayerFormat::packing
 * \brief Any packing scheme applied to this BayerFormat
 */

} /* namespace libcamera */
