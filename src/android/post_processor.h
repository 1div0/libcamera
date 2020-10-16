/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * post_processor.h - CameraStream Post Processing Interface
 */
#ifndef __ANDROID_POST_PROCESSOR_H__
#define __ANDROID_POST_PROCESSOR_H__

#include <libcamera/buffer.h>
#include <libcamera/span.h>
#include <libcamera/stream.h>

class CameraMetadata;

class PostProcessor
{
public:
	virtual ~PostProcessor() {}

	virtual int configure(const libcamera::StreamConfiguration &inCfg,
			      const libcamera::StreamConfiguration &outCfg) = 0;
	virtual int process(const libcamera::FrameBuffer *source,
			    const libcamera::Span<uint8_t> &destination,
			    CameraMetadata *metadata) = 0;
};

#endif /* __ANDROID_POST_PROCESSOR_H__ */
