/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * uvcvideo.cpp - Pipeline handler for uvcvideo devices
 */

#include <algorithm>
#include <iomanip>
#include <tuple>

#include <libcamera/camera.h>
#include <libcamera/controls.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include "device_enumerator.h"
#include "log.h"
#include "media_device.h"
#include "pipeline_handler.h"
#include "utils.h"
#include "v4l2_controls.h"
#include "v4l2_videodevice.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(UVC)

class UVCCameraData : public CameraData
{
public:
	UVCCameraData(PipelineHandler *pipe)
		: CameraData(pipe), video_(nullptr)
	{
	}

	~UVCCameraData()
	{
		delete video_;
	}

	int init(MediaEntity *entity);
	void bufferReady(Buffer *buffer);

	V4L2VideoDevice *video_;
	Stream stream_;
};

class UVCCameraConfiguration : public CameraConfiguration
{
public:
	UVCCameraConfiguration();

	Status validate() override;
};

class PipelineHandlerUVC : public PipelineHandler
{
public:
	PipelineHandlerUVC(CameraManager *manager);

	CameraConfiguration *generateConfiguration(Camera *camera,
		const StreamRoles &roles) override;
	int configure(Camera *camera, CameraConfiguration *config) override;

	int allocateBuffers(Camera *camera,
			    const std::set<Stream *> &streams) override;
	int freeBuffers(Camera *camera,
			const std::set<Stream *> &streams) override;

	int start(Camera *camera) override;
	void stop(Camera *camera) override;

	int queueRequest(Camera *camera, Request *request) override;

	bool match(DeviceEnumerator *enumerator) override;

private:
	int processControls(UVCCameraData *data, Request *request);

	UVCCameraData *cameraData(const Camera *camera)
	{
		return static_cast<UVCCameraData *>(
			PipelineHandler::cameraData(camera));
	}
};

UVCCameraConfiguration::UVCCameraConfiguration()
	: CameraConfiguration()
{
}

CameraConfiguration::Status UVCCameraConfiguration::validate()
{
	Status status = Valid;

	if (config_.empty())
		return Invalid;

	/* Cap the number of entries to the available streams. */
	if (config_.size() > 1) {
		config_.resize(1);
		status = Adjusted;
	}

	StreamConfiguration &cfg = config_[0];
	const StreamFormats &formats = cfg.formats();
	const unsigned int pixelFormat = cfg.pixelFormat;
	const Size size = cfg.size;

	const std::vector<unsigned int> pixelFormats = formats.pixelformats();
	auto iter = std::find(pixelFormats.begin(), pixelFormats.end(), pixelFormat);
	if (iter == pixelFormats.end()) {
		cfg.pixelFormat = pixelFormats.front();
		LOG(UVC, Debug)
			<< "Adjusting pixel format from " << pixelFormat
			<< " to " << cfg.pixelFormat;
		status = Adjusted;
	}

	const std::vector<Size> &formatSizes = formats.sizes(cfg.pixelFormat);
	cfg.size = formatSizes.front();
	for (const Size &formatsSize : formatSizes) {
		if (formatsSize > size)
			break;

		cfg.size = formatsSize;
	}

	if (cfg.size != size) {
		LOG(UVC, Debug)
			<< "Adjusting size from " << size.toString()
			<< " to " << cfg.size.toString();
		status = Adjusted;
	}

	cfg.bufferCount = 4;

	return status;
}

PipelineHandlerUVC::PipelineHandlerUVC(CameraManager *manager)
	: PipelineHandler(manager)
{
}

CameraConfiguration *PipelineHandlerUVC::generateConfiguration(Camera *camera,
	const StreamRoles &roles)
{
	UVCCameraData *data = cameraData(camera);
	CameraConfiguration *config = new UVCCameraConfiguration();

	if (roles.empty())
		return config;

	ImageFormats v4l2formats = data->video_->formats();
	StreamFormats formats(v4l2formats.data());
	StreamConfiguration cfg(formats);

	cfg.pixelFormat = formats.pixelformats().front();
	cfg.size = formats.sizes(cfg.pixelFormat).back();
	cfg.bufferCount = 4;

	config->addConfiguration(cfg);

	config->validate();

	return config;
}

int PipelineHandlerUVC::configure(Camera *camera, CameraConfiguration *config)
{
	UVCCameraData *data = cameraData(camera);
	StreamConfiguration &cfg = config->at(0);
	int ret;

	V4L2DeviceFormat format = {};
	format.fourcc = cfg.pixelFormat;
	format.size = cfg.size;

	ret = data->video_->setFormat(&format);
	if (ret)
		return ret;

	if (format.size != cfg.size ||
	    format.fourcc != cfg.pixelFormat)
		return -EINVAL;

	cfg.setStream(&data->stream_);

	return 0;
}

int PipelineHandlerUVC::allocateBuffers(Camera *camera,
					const std::set<Stream *> &streams)
{
	UVCCameraData *data = cameraData(camera);
	Stream *stream = *streams.begin();
	const StreamConfiguration &cfg = stream->configuration();

	LOG(UVC, Debug) << "Requesting " << cfg.bufferCount << " buffers";

	if (stream->memoryType() == InternalMemory)
		return data->video_->exportBuffers(&stream->bufferPool());
	else
		return data->video_->importBuffers(&stream->bufferPool());
}

int PipelineHandlerUVC::freeBuffers(Camera *camera,
				    const std::set<Stream *> &streams)
{
	UVCCameraData *data = cameraData(camera);
	return data->video_->releaseBuffers();
}

int PipelineHandlerUVC::start(Camera *camera)
{
	UVCCameraData *data = cameraData(camera);
	return data->video_->streamOn();
}

void PipelineHandlerUVC::stop(Camera *camera)
{
	UVCCameraData *data = cameraData(camera);
	data->video_->streamOff();
}

int PipelineHandlerUVC::processControls(UVCCameraData *data, Request *request)
{
	V4L2ControlList controls;

	for (auto it : request->controls()) {
		const ControlInfo *ci = it.first;
		ControlValue &value = it.second;

		switch (ci->id()) {
		case Brightness:
			controls.add(V4L2_CID_BRIGHTNESS, value.getInt());
			break;

		case Contrast:
			controls.add(V4L2_CID_CONTRAST, value.getInt());
			break;

		case Saturation:
			controls.add(V4L2_CID_SATURATION, value.getInt());
			break;

		case ManualExposure:
			controls.add(V4L2_CID_EXPOSURE_AUTO, 1);
			controls.add(V4L2_CID_EXPOSURE_ABSOLUTE, value.getInt());
			break;

		case ManualGain:
			controls.add(V4L2_CID_GAIN, value.getInt());
			break;

		default:
			break;
		}
	}

	for (const V4L2Control &ctrl : controls)
		LOG(UVC, Debug)
			<< "Setting control 0x"
			<< std::hex << std::setw(8) << ctrl.id() << std::dec
			<< " to " << ctrl.value();

	int ret = data->video_->setControls(&controls);
	if (ret) {
		LOG(UVC, Error) << "Failed to set controls: " << ret;
		return ret < 0 ? ret : -EINVAL;
	}

	return ret;
}

int PipelineHandlerUVC::queueRequest(Camera *camera, Request *request)
{
	UVCCameraData *data = cameraData(camera);
	Buffer *buffer = request->findBuffer(&data->stream_);
	if (!buffer) {
		LOG(UVC, Error)
			<< "Attempt to queue request with invalid stream";

		return -ENOENT;
	}

	int ret = processControls(data, request);
	if (ret < 0)
		return ret;

	ret = data->video_->queueBuffer(buffer);
	if (ret < 0)
		return ret;

	PipelineHandler::queueRequest(camera, request);

	return 0;
}

bool PipelineHandlerUVC::match(DeviceEnumerator *enumerator)
{
	MediaDevice *media;
	DeviceMatch dm("uvcvideo");

	media = acquireMediaDevice(enumerator, dm);
	if (!media)
		return false;

	std::unique_ptr<UVCCameraData> data = utils::make_unique<UVCCameraData>(this);

	/* Locate and initialise the camera data with the default video node. */
	for (MediaEntity *entity : media->entities()) {
		if (entity->flags() & MEDIA_ENT_FL_DEFAULT) {
			if (data->init(entity))
				return false;
			break;
		}
	}

	if (!data) {
		LOG(UVC, Error) << "Could not find a default video device";
		return false;
	}

	/* Create and register the camera. */
	std::set<Stream *> streams{ &data->stream_ };
	std::shared_ptr<Camera> camera = Camera::create(this, media->model(), streams);
	registerCamera(std::move(camera), std::move(data));

	/* Enable hot-unplug notifications. */
	hotplugMediaDevice(media);

	return true;
}

int UVCCameraData::init(MediaEntity *entity)
{
	int ret;

	/* Create and open the video device. */
	video_ = new V4L2VideoDevice(entity);
	ret = video_->open();
	if (ret)
		return ret;

	video_->bufferReady.connect(this, &UVCCameraData::bufferReady);

	/* Initialise the supported controls. */
	const V4L2ControlInfoMap &controls = video_->controls();
	for (const auto &ctrl : controls) {
		unsigned int v4l2Id = ctrl.first;
		const V4L2ControlInfo &info = ctrl.second;
		ControlId id;

		switch (v4l2Id) {
		case V4L2_CID_BRIGHTNESS:
			id = Brightness;
			break;
		case V4L2_CID_CONTRAST:
			id = Contrast;
			break;
		case V4L2_CID_SATURATION:
			id = Saturation;
			break;
		case V4L2_CID_EXPOSURE_ABSOLUTE:
			id = ManualExposure;
			break;
		case V4L2_CID_GAIN:
			id = ManualGain;
			break;
		default:
			continue;
		}

		controlInfo_.emplace(std::piecewise_construct,
				     std::forward_as_tuple(id),
				     std::forward_as_tuple(id, info.min(), info.max()));
	}

	return 0;
}

void UVCCameraData::bufferReady(Buffer *buffer)
{
	Request *request = buffer->request();

	pipe_->completeBuffer(camera_, request, buffer);
	pipe_->completeRequest(camera_, request);
}

REGISTER_PIPELINE_HANDLER(PipelineHandlerUVC);

} /* namespace libcamera */
