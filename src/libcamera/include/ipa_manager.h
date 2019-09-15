/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * ipa_manager.h - Image Processing Algorithm module manager
 */
#ifndef __LIBCAMERA_IPA_MANAGER_H__
#define __LIBCAMERA_IPA_MANAGER_H__

#include <vector>

#include <ipa/ipa_interface.h>
#include <ipa/ipa_module_info.h>

#include "ipa_module.h"
#include "pipeline_handler.h"

namespace libcamera {

class IPAManager
{
public:
	static IPAManager *instance();

	std::unique_ptr<IPAInterface> createIPA(PipelineHandler *pipe,
						uint32_t maxVersion,
						uint32_t minVersion);

private:
	std::vector<IPAModule *> modules_;

	IPAManager();
	~IPAManager();

	int addDir(const char *libDir);
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_IPA_MANAGER_H__ */
