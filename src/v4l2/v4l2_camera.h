/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * v4l2_camera.h - V4L2 compatibility camera
 */

#ifndef __V4L2_CAMERA_H__
#define __V4L2_CAMERA_H__

#include <deque>
#include <mutex>
#include <utility>

#include <libcamera/base/semaphore.h>

#include <libcamera/camera.h>
#include <libcamera/file_descriptor.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>

class V4L2Camera
{
public:
	struct Buffer {
		Buffer(unsigned int index, const libcamera::FrameMetadata &data)
			: index_(index), data_(data)
		{
		}

		unsigned int index_;
		libcamera::FrameMetadata data_;
	};

	V4L2Camera(std::shared_ptr<libcamera::Camera> camera);
	~V4L2Camera();

	int open(libcamera::StreamConfiguration *streamConfig);
	void close();
	void bind(int efd);
	void unbind();

	std::vector<Buffer> completedBuffers();

	int configure(libcamera::StreamConfiguration *streamConfigOut,
		      const libcamera::Size &size,
		      const libcamera::PixelFormat &pixelformat,
		      unsigned int bufferCount);
	int validateConfiguration(const libcamera::PixelFormat &pixelformat,
				  const libcamera::Size &size,
				  libcamera::StreamConfiguration *streamConfigOut);

	int allocBuffers(unsigned int count);
	void freeBuffers();
	libcamera::FileDescriptor getBufferFd(unsigned int index);

	int streamOn();
	int streamOff();

	int qbuf(unsigned int index);

	void waitForBufferAvailable();
	bool isBufferAvailable();

	bool isRunning();

private:
	void requestComplete(libcamera::Request *request);

	std::shared_ptr<libcamera::Camera> camera_;
	std::unique_ptr<libcamera::CameraConfiguration> config_;

	bool isRunning_;

	std::mutex bufferLock_;
	libcamera::FrameBufferAllocator *bufferAllocator_;

	std::vector<std::unique_ptr<libcamera::Request>> requestPool_;

	std::deque<libcamera::Request *> pendingRequests_;
	std::deque<std::unique_ptr<Buffer>> completedBuffers_;

	int efd_;

	libcamera::Mutex bufferMutex_;
	std::condition_variable bufferCV_;
	unsigned int bufferAvailableCount_;
};

#endif /* __V4L2_CAMERA_H__ */
