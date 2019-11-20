/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * camera_controls.h - Camera controls
 */
#ifndef __LIBCAMERA_CAMERA_CONTROLS_H__
#define __LIBCAMERA_CAMERA_CONTROLS_H__

#include "control_validator.h"

namespace libcamera {

class Camera;

class CameraControlValidator final : public ControlValidator
{
public:
	CameraControlValidator(Camera *camera);

	const std::string &name() const override;
	bool validate(unsigned int id) const override;

private:
	Camera *camera_;
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_CAMERA_CONTROLS_H__ */
